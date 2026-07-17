#pragma once

#include <JuceHeader.h>
#include "PluginParameters.h"
#include "PresetManager.h"
#include "DSP/EQEngine.h"
#include "DSP/Compressor.h"
#include "DSP/SpectrumAnalyzer.h"

class ZeroEQAudioProcessor : public juce::AudioProcessor
{
public:
    ZeroEQAudioProcessor();
    ~ZeroEQAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return (int) ZeroEQ::PresetManager::getFactoryPresets().size(); }
    int getCurrentProgram() override { return currentProgramIndex; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    ZeroEQ::PresetManager presetManager;

    ZeroEQ::EQEngine& getEQEngine() { return eqEngine; }
    ZeroEQ::Compressor& getCompressor() { return compressor; }
    ZeroEQ::SpectrumAnalyzer& getPreSpectrum() { return preSpectrum; }
    ZeroEQ::SpectrumAnalyzer& getPostSpectrum() { return postSpectrum; }
    ZeroEQ::PresetManager& getPresetManager() { return presetManager; }

    float getInputLevelDb() const { return inputLevelDb.load(); }
    float getOutputLevelDb() const { return outputLevelDb.load(); }

private:
    ZeroEQ::EQEngine eqEngine;
    ZeroEQ::Compressor compressor;
    ZeroEQ::SpectrumAnalyzer preSpectrum;
    ZeroEQ::SpectrumAnalyzer postSpectrum;

    int currentProgramIndex = 0;

    std::atomic<float> inputLevelDb { -100.0f };
    std::atomic<float> outputLevelDb { -100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZeroEQAudioProcessor)
};
