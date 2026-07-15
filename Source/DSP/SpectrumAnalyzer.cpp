#include "SpectrumAnalyzer.h"

namespace ZeroEQ
{

void SpectrumAnalyzer::prepare(double sr)
{
    sampleRate = sr;
    std::fill(circularBuffer.begin(), circularBuffer.end(), 0.0f);
    writeIndex.store(0);
}

void SpectrumAnalyzer::pushBuffer(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    if (numSamples <= 0)
        return;

    int idx = writeIndex.load(std::memory_order_relaxed);
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;
        for (int ch = 0; ch < numCh; ++ch)
            sample += buffer.getSample(ch, i);
        if (numCh > 0)
            sample /= (float) numCh;

        circularBuffer[(size_t) idx] = sample;
        idx = (idx + 1) % fftSize;
    }
    writeIndex.store(idx, std::memory_order_release);
}

void SpectrumAnalyzer::computeSpectrum(std::array<float, scopeSize>& outMagnitudesDb)
{
    const int idx = writeIndex.load(std::memory_order_acquire);

    std::array<float, fftSize * 2> fftData {};
    for (int i = 0; i < fftSize; ++i)
        fftData[(size_t) i] = circularBuffer[(size_t) ((idx + i) % fftSize)];

    window.multiplyWithWindowingTable(fftData.data(), (size_t) fftSize);
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    for (int i = 0; i < scopeSize; ++i)
    {
        const float magnitude = fftData[(size_t) i] / (float) fftSize;
        outMagnitudesDb[(size_t) i] = juce::Decibels::gainToDecibels(magnitude, -100.0f);
    }
}

} // namespace ZeroEQ
