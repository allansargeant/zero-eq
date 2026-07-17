#pragma once

#include <JuceHeader.h>

namespace ZeroEQ
{

// Ballistics-accurate level metering. A read-only side path - it reads a copy of
// whatever buffer it's given and never touches the actual signal chain, so nothing
// it does can add latency to the plugin's real audio output.
//
//  - Peak: instant attack, ~20dB/s exponential release. A practical fast-peak-meter
//    approximation, not a claim of matching any specific broadcast PPM standard's
//    exact integration/decay spec.
//  - VU: a one-pole RMS-based integrating meter tuned for ~300ms to reach 99% of a
//    step input, approximating classic VU meter response (not an exact model of a
//    real VU meter's 2nd-order damped needle movement).
//  - True peak: a lightweight 4x oversampled peak estimate using Catmull-Rom cubic
//    interpolation between samples, catching inter-sample peaks a simple sample-peak
//    read misses. A practical estimate, not a full ITU-R BS.1770-compliant true-peak
//    filter implementation.
class LevelMeter
{
public:
    void prepare(double sampleRate);
    void reset();

    void process(const juce::AudioBuffer<float>& buffer);

    float getPeakDb() const { return peakDb.load(); }
    float getTruePeakDb() const { return truePeakDb.load(); }
    float getVuDb() const { return vuDb.load(); }
    bool isClipping() const { return clipHoldSamplesRemaining.load() > 0; }

private:
    double sampleRate = 44100.0;

    float peakEnvelopeLinear = 0.0f;
    float vuEnvelopeLinear = 0.0f;

    std::atomic<float> peakDb { -100.0f };
    std::atomic<float> truePeakDb { -100.0f };
    std::atomic<float> vuDb { -100.0f };
    std::atomic<int> clipHoldSamplesRemaining { 0 };
};

} // namespace ZeroEQ
