#include "EQEngine.h"

namespace ZeroEQ
{

void EQEngine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    for (auto& band : bands)
        band.prepare(spec);

    for (auto& s : smoothed)
    {
        s.freq.reset(spec.sampleRate, 0.02);
        s.gain.reset(spec.sampleRate, 0.02);
        s.q.reset(spec.sampleRate, 0.02);
    }

    firstBlock = true;
}

void EQEngine::reset()
{
    for (auto& band : bands)
        band.reset();
    firstBlock = true;
}

void EQEngine::updateAndProcess(juce::AudioBuffer<float>& buffer, juce::AudioProcessorValueTreeState& apvts)
{
    bool anySolo = false;
    for (int i = 0; i < numBands; ++i)
        if (apvts.getRawParameterValue(ParamIDs::bandSolo(i))->load() > 0.5f)
            anySolo = true;

    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numBands; ++i)
    {
        const auto type      = (FilterType) (int) apvts.getRawParameterValue(ParamIDs::bandType(i))->load();
        const float freq     = apvts.getRawParameterValue(ParamIDs::bandFreq(i))->load();
        const float gain     = apvts.getRawParameterValue(ParamIDs::bandGain(i))->load();
        const float q        = apvts.getRawParameterValue(ParamIDs::bandQ(i))->load();
        const auto character = (FilterCharacter) (int) apvts.getRawParameterValue(ParamIDs::bandCharacter(i))->load();
        const auto slope     = (FilterSlope) (int) apvts.getRawParameterValue(ParamIDs::bandSlope(i))->load();
        const bool active    = apvts.getRawParameterValue(ParamIDs::bandActive(i))->load() > 0.5f;
        const bool solo      = apvts.getRawParameterValue(ParamIDs::bandSolo(i))->load() > 0.5f;

        auto& s = smoothed[(size_t) i];
        if (firstBlock)
        {
            s.freq.setCurrentAndTargetValue(freq);
            s.gain.setCurrentAndTargetValue(gain);
            s.q.setCurrentAndTargetValue(q);
        }
        else
        {
            s.freq.setTargetValue(freq);
            s.gain.setTargetValue(gain);
            s.q.setTargetValue(q);
        }

        if (numSamples > 1)
        {
            s.freq.skip(numSamples - 1);
            s.gain.skip(numSamples - 1);
            s.q.skip(numSamples - 1);
        }
        const float curFreq = s.freq.getNextValue();
        const float curGain = s.gain.getNextValue();
        const float curQ    = s.q.getNextValue();

        bands[(size_t) i].isActive = active && (! anySolo || solo);
        bands[(size_t) i].update(type, curFreq, curGain, curQ, character, slope);
    }

    firstBlock = false;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    for (auto& band : bands)
        band.process(context);
}

EQEngine::BandSnapshot EQEngine::readSnapshot(juce::AudioProcessorValueTreeState& apvts, int i)
{
    BandSnapshot s;
    s.type      = (FilterType) (int) apvts.getRawParameterValue(ParamIDs::bandType(i))->load();
    s.freqHz    = apvts.getRawParameterValue(ParamIDs::bandFreq(i))->load();
    s.gainDb    = apvts.getRawParameterValue(ParamIDs::bandGain(i))->load();
    s.q         = apvts.getRawParameterValue(ParamIDs::bandQ(i))->load();
    s.character = (FilterCharacter) (int) apvts.getRawParameterValue(ParamIDs::bandCharacter(i))->load();
    s.slope     = (FilterSlope) (int) apvts.getRawParameterValue(ParamIDs::bandSlope(i))->load();
    s.active    = apvts.getRawParameterValue(ParamIDs::bandActive(i))->load() > 0.5f;
    s.solo      = apvts.getRawParameterValue(ParamIDs::bandSolo(i))->load() > 0.5f;
    return s;
}

std::array<EQEngine::BandSnapshot, numBands> EQEngine::readAllSnapshots(juce::AudioProcessorValueTreeState& apvts)
{
    std::array<BandSnapshot, numBands> result;
    for (int i = 0; i < numBands; ++i)
        result[(size_t) i] = readSnapshot(apvts, i);
    return result;
}

float EQEngine::getCompositeMagnitude(const std::array<BandSnapshot, numBands>& snapshots, double frequencyHz, double sr)
{
    bool anySolo = false;
    for (auto& b : snapshots)
        if (b.solo)
            anySolo = true;

    double magnitude = 1.0;
    for (auto& b : snapshots)
    {
        if (! b.active || (anySolo && ! b.solo))
            continue;

        magnitude *= EQBand::computeMagnitudeForFrequency(b.type, b.freqHz, b.gainDb, b.q, b.character, b.slope,
                                                            frequencyHz, sr);
    }

    return (float) magnitude;
}

} // namespace ZeroEQ
