#include "PluginProcessor.h"
#include "PluginEditor.h"

ZeroEQAudioProcessor::ZeroEQAudioProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", ZeroEQ::createParameterLayout()),
      presetManager(apvts)
{
}

void ZeroEQAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;
    presetManager.loadFactoryPreset(index);
    currentProgramIndex = index;
}

const juce::String ZeroEQAudioProcessor::getProgramName(int index)
{
    const auto& presets = ZeroEQ::PresetManager::getFactoryPresets();
    if (index < 0 || index >= (int) presets.size())
        return {};
    return presets[(size_t) index].name;
}

void ZeroEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) juce::jmax(1, getTotalNumOutputChannels());

    eqEngine.prepare(spec);
    compressor.prepare(spec);
    preSpectrum.prepare(sampleRate);
    postSpectrum.prepare(sampleRate);
}

void ZeroEQAudioProcessor::releaseResources()
{
}

bool ZeroEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainIn == mainOut;
}

void ZeroEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int totalOut = getTotalNumOutputChannels();
    for (int ch = getTotalNumInputChannels(); ch < totalOut; ++ch)
        buffer.clear(ch, 0, numSamples);

    // --- Input gain + metering ---
    const float inputGainDb = apvts.getRawParameterValue(ZeroEQ::ParamIDs::inputGain)->load();
    buffer.applyGain(juce::Decibels::decibelsToGain(inputGainDb));
    inputLevelDb.store(juce::Decibels::gainToDecibels(buffer.getMagnitude(0, numSamples), -100.0f));

    preSpectrum.pushBuffer(buffer);

    // --- EQ ---
    const bool eqActive = apvts.getRawParameterValue(ZeroEQ::ParamIDs::eqActive)->load() > 0.5f;
    if (eqActive)
        eqEngine.updateAndProcess(buffer, apvts);

    // --- Compressor ---
    const bool compActive = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compActive)->load() > 0.5f;
    if (compActive)
    {
        ZeroEQ::Compressor::Params p;
        p.thresholdDb = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compThreshold)->load();
        p.ratio       = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compRatio)->load();
        p.attackMs    = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compAttack)->load();
        p.releaseMs   = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compRelease)->load();
        p.kneeDb      = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compKnee)->load();
        p.makeupDb    = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compMakeup)->load();
        p.autoMakeup  = apvts.getRawParameterValue(ZeroEQ::ParamIDs::compAutoMakeup)->load() > 0.5f;
        p.detector    = (ZeroEQ::DetectorType) (int) apvts.getRawParameterValue(ZeroEQ::ParamIDs::compDetector)->load();
        compressor.process(buffer, p);
    }
    else
    {
        compressor.reset();
    }

    // --- Output gain + metering ---
    const float outputGainDb = apvts.getRawParameterValue(ZeroEQ::ParamIDs::outputGain)->load();
    buffer.applyGain(juce::Decibels::decibelsToGain(outputGainDb));
    outputLevelDb.store(juce::Decibels::gainToDecibels(buffer.getMagnitude(0, numSamples), -100.0f));

    postSpectrum.pushBuffer(buffer);
}

juce::AudioProcessorEditor* ZeroEQAudioProcessor::createEditor()
{
    return new ZeroEQAudioProcessorEditor(*this);
}

void ZeroEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ZeroEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// This creates the platform-specific instance of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZeroEQAudioProcessor();
}
