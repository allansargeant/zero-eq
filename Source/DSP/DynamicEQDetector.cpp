#include "DynamicEQDetector.h"

namespace ZeroEQ
{

void DynamicEQDetector::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    auto monoSpec = spec;
    monoSpec.numChannels = 1;
    analysisFilter.prepare(monoSpec);
    reset();
}

void DynamicEQDetector::reset()
{
    analysisFilter.reset();
    rmsStateSquared = 0.0f;
    smoothedGainDeltaDb = 0.0f;
    currentGainDeltaDb.store(0.0f);
}

DynamicEQDetector::Coeffs::Ptr DynamicEQDetector::designAnalysisFilter(FilterType bandType, float freqHz, float q, double sr)
{
    freqHz = juce::jlimit(20.0f, (float) (sr * 0.49), freqHz);
    q = juce::jmax(0.1f, q);

    switch (bandType)
    {
        case FilterType::LowShelf:  return Coeffs::makeLowPass(sr, freqHz, q);
        case FilterType::HighShelf: return Coeffs::makeHighPass(sr, freqHz, q);
        case FilterType::Bell:
        case FilterType::TiltShelf:
        default:                    return Coeffs::makeBandPass(sr, freqHz, q);
    }
}

float DynamicEQDetector::process(const juce::AudioBuffer<float>& buffer, FilterType bandType,
                                  float freqHz, float q, const Params& params)
{
    auto designed = designAnalysisFilter(bandType, freqHz, q, sampleRate);
    if (analysisFilter.coefficients == nullptr)
        analysisFilter.coefficients = designed;
    else
        *analysisFilter.coefficients = *designed;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0 || numChannels == 0)
        return smoothedGainDeltaDb;

    const float attackCoeff  = std::exp(-1.0f / (float) (sampleRate * (params.attackMs * 0.001f)));
    const float releaseCoeff = std::exp(-1.0f / (float) (sampleRate * (params.releaseMs * 0.001f)));
    const float rmsCoeff     = std::exp(-1.0f / (float) (sampleRate * 0.005));

    auto* const* channelData = buffer.getArrayOfReadPointers();

    for (int n = 0; n < numSamples; ++n)
    {
        float monoSample = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            monoSample += channelData[ch][n];
        monoSample /= (float) numChannels;

        const float filtered = analysisFilter.processSample(monoSample);

        rmsStateSquared = rmsCoeff * rmsStateSquared + (1.0f - rmsCoeff) * (filtered * filtered);
        const float envelope = std::sqrt(juce::jmax(0.0f, rmsStateSquared));
        const float levelDb = juce::Decibels::gainToDecibels(envelope, -100.0f);

        float targetDeltaDb = 0.0f;
        if (params.direction == DynamicDirection::Downward)
        {
            const float over = levelDb - params.thresholdDb;
            if (over > 0.0f)
                targetDeltaDb = -juce::jmin(params.rangeDb, over * (1.0f - 1.0f / params.ratio));
        }
        else
        {
            const float under = params.thresholdDb - levelDb;
            if (under > 0.0f)
                targetDeltaDb = juce::jmin(params.rangeDb, under * (1.0f - 1.0f / params.ratio));
        }

        // Branched attack/release: attack while the magnitude of the delta is growing,
        // release while it's shrinking back toward zero. Same technique as Compressor.
        if (std::abs(targetDeltaDb) > std::abs(smoothedGainDeltaDb))
            smoothedGainDeltaDb = attackCoeff * smoothedGainDeltaDb + (1.0f - attackCoeff) * targetDeltaDb;
        else
            smoothedGainDeltaDb = releaseCoeff * smoothedGainDeltaDb + (1.0f - releaseCoeff) * targetDeltaDb;
    }

    currentGainDeltaDb.store(smoothedGainDeltaDb);
    return smoothedGainDeltaDb;
}

} // namespace ZeroEQ
