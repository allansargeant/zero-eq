#pragma once

#include <JuceHeader.h>
#include "../PluginParameters.h"

namespace ZeroEQ
{

// Feed-forward, stereo-linked compressor. Classic "branched" gain-reduction smoother
// (Giannoulis/Massberg/Reiss style): the static soft-knee curve is evaluated per-sample
// and the resulting gain reduction is smoothed directly with separate attack/release
// coefficients. No lookahead, no block buffering - zero added latency.
class Compressor
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    struct Params
    {
        float thresholdDb = -18.0f;
        float ratio = 2.0f;
        float attackMs = 10.0f;
        float releaseMs = 100.0f;
        float kneeDb = 6.0f;
        float makeupDb = 0.0f;
        bool autoMakeup = true;
        DetectorType detector = DetectorType::RMS;
    };

    void process(juce::AudioBuffer<float>& buffer, const Params& params);

    float getCurrentGainReductionDb() const { return currentGainReductionDb.load(); }

private:
    double sampleRate = 44100.0;
    float smoothedGainReductionDb = 0.0f;
    float rmsStateSquared = 0.0f;
    std::atomic<float> currentGainReductionDb { 0.0f };

    static float computeAutoMakeup(const Params& params);
};

} // namespace ZeroEQ
