#include "Compressor.h"

namespace ZeroEQ
{

void Compressor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
}

void Compressor::reset()
{
    smoothedGainReductionDb = 0.0f;
    rmsStateSquared = 0.0f;
    currentGainReductionDb.store(0.0f);
}

float Compressor::computeAutoMakeup(const Params& p)
{
    // Rough heuristic: roughly compensate for the gain reduction a 0 dBFS peak would
    // incur, halved so it reads as "gentle" rather than fully restoring peak level.
    const float reductionAtFullScale = (p.thresholdDb < 0.0f)
        ? -p.thresholdDb * (1.0f - 1.0f / p.ratio)
        : 0.0f;
    return juce::jlimit(0.0f, 24.0f, reductionAtFullScale * 0.5f);
}

void Compressor::process(juce::AudioBuffer<float>& buffer, const Params& params)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0)
        return;

    const float attackCoeff  = std::exp(-1.0f / (float) (sampleRate * (params.attackMs * 0.001f)));
    const float releaseCoeff = std::exp(-1.0f / (float) (sampleRate * (params.releaseMs * 0.001f)));
    const float knee = juce::jmax(0.0f, params.kneeDb);
    const float makeupDb = params.autoMakeup ? computeAutoMakeup(params) : params.makeupDb;

    // RMS detector smoothing time constant (~5 ms), independent of attack/release.
    const float rmsCoeff = std::exp(-1.0f / (float) (sampleRate * 0.005));

    auto* const* channelData = buffer.getArrayOfWritePointers();

    for (int n = 0; n < numSamples; ++n)
    {
        // Stereo-linked detector: peak (or RMS) of the loudest channel this sample.
        float linkedInput = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            linkedInput = juce::jmax(linkedInput, std::abs(channelData[ch][n]));

        float detectorLevel;
        if (params.detector == DetectorType::RMS)
        {
            rmsStateSquared = rmsCoeff * rmsStateSquared + (1.0f - rmsCoeff) * (linkedInput * linkedInput);
            detectorLevel = std::sqrt(juce::jmax(0.0f, rmsStateSquared));
        }
        else
        {
            detectorLevel = linkedInput;
        }

        const float inputDb = juce::Decibels::gainToDecibels(detectorLevel, -100.0f);

        // Static soft-knee curve (Giannoulis/Reiss tutorial form).
        float outputDb;
        const float delta = inputDb - params.thresholdDb;
        if (delta < -knee * 0.5f)
        {
            outputDb = inputDb;
        }
        else if (delta > knee * 0.5f)
        {
            outputDb = params.thresholdDb + delta / params.ratio;
        }
        else
        {
            const float t = delta + knee * 0.5f;
            outputDb = inputDb + ((1.0f / params.ratio - 1.0f) * t * t) / (2.0f * juce::jmax(knee, 0.0001f));
        }

        const float targetGainReductionDb = outputDb - inputDb; // <= 0

        // Branched attack/release: attack when reduction is deepening, release when easing.
        if (targetGainReductionDb < smoothedGainReductionDb)
            smoothedGainReductionDb = attackCoeff * smoothedGainReductionDb + (1.0f - attackCoeff) * targetGainReductionDb;
        else
            smoothedGainReductionDb = releaseCoeff * smoothedGainReductionDb + (1.0f - releaseCoeff) * targetGainReductionDb;

        const float totalGainDb = smoothedGainReductionDb + makeupDb;
        const float totalGainLinear = juce::Decibels::decibelsToGain(totalGainDb);

        for (int ch = 0; ch < numChannels; ++ch)
            channelData[ch][n] *= totalGainLinear;
    }

    currentGainReductionDb.store(smoothedGainReductionDb);
}

} // namespace ZeroEQ
