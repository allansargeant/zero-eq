#pragma once

#include <JuceHeader.h>

namespace ZeroEQ
{

// Lock-free-ish producer (audio thread) / consumer (GUI timer thread) spectrum feed.
// The audio thread only ever writes into a plain circular buffer and publishes a write
// index; the GUI thread reads the most recent window and runs the FFT itself, so no FFT
// work (and no locking) ever happens on the audio thread.
class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int scopeSize = fftSize / 2;

    void prepare(double sampleRate);

    // Audio thread.
    void pushBuffer(const juce::AudioBuffer<float>& buffer);

    // GUI/timer thread. Fills outMagnitudesDb[0..scopeSize) with the current spectrum.
    void computeSpectrum(std::array<float, scopeSize>& outMagnitudesDb);

    double getSampleRate() const { return sampleRate; }

private:
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::vector<float> circularBuffer = std::vector<float>((size_t) fftSize, 0.0f);
    std::atomic<int> writeIndex { 0 };
    double sampleRate = 44100.0;
};

} // namespace ZeroEQ
