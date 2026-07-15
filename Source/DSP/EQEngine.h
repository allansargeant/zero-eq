#pragma once

#include <JuceHeader.h>
#include "EQBand.h"
#include "DynamicEQDetector.h"
#include "../PluginParameters.h"

namespace ZeroEQ
{

// Owns all EQ bands, pulls live parameter values from the APVTS each block, and
// runs the series chain. All processing is minimum-phase IIR - zero added latency.
class EQEngine
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void updateAndProcess(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts);

    // For GUI curve drawing: composite magnitude across all active bands at a frequency,
    // using a caller-supplied snapshot of parameter values (thread-safe, stateless).
    struct BandSnapshot
    {
        FilterType type;
        float freqHz;
        float gainDb;
        float q;
        FilterCharacter character;
        FilterSlope slope;
        bool active;
        bool solo;

        bool dynActive;
        DynamicDirection dynDirection;
        float dynThresholdDb;
        float dynRatio;
        float dynAttackMs;
        float dynReleaseMs;
        float dynRangeDb;
    };

    static float getCompositeMagnitude(const std::array<BandSnapshot, numBands>& bands, double frequencyHz, double sampleRate);

    // Message-thread-safe: reads current parameter values (atomics) into a snapshot struct.
    static BandSnapshot readSnapshot(juce::AudioProcessorValueTreeState& apvts, int bandIndex);
    static std::array<BandSnapshot, numBands> readAllSnapshots(juce::AudioProcessorValueTreeState& apvts);

    // Message-thread-safe: current live dynamic gain delta (dB) for GUI metering/curve display.
    float getBandDynamicGainDeltaDb(int bandIndex) const;

private:
    std::array<EQBand, numBands> bands;
    std::array<DynamicEQDetector, numBands> dynamicDetectors;

    struct SmoothedBandParams
    {
        juce::SmoothedValue<float> freq { 1000.0f };
        juce::SmoothedValue<float> gain { 0.0f };
        juce::SmoothedValue<float> q { 0.707f };
    };
    std::array<SmoothedBandParams, numBands> smoothed;

    double sampleRate = 44100.0;
    bool firstBlock = true;
};

} // namespace ZeroEQ
