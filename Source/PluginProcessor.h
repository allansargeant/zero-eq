#pragma once

#include <JuceHeader.h>
#include "PluginParameters.h"
#include "PresetManager.h"
#include "DSP/EQEngine.h"
#include "DSP/Compressor.h"
#include "DSP/SpectrumAnalyzer.h"
#include "DSP/LevelMeter.h"

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

    ZeroEQ::LevelMeter& getInputMeter() { return inputMeter; }
    ZeroEQ::LevelMeter& getOutputMeter() { return outputMeter; }

    // Retained for compatibility with existing GUI call sites; equivalent to
    // getInputMeter().getPeakDb() / getOutputMeter().getPeakDb().
    float getInputLevelDb() const { return inputMeter.getPeakDb(); }
    float getOutputLevelDb() const { return outputMeter.getPeakDb(); }

private:
    ZeroEQ::EQEngine eqEngine;
    ZeroEQ::Compressor compressor;
    ZeroEQ::SpectrumAnalyzer preSpectrum;
    ZeroEQ::SpectrumAnalyzer postSpectrum;
    ZeroEQ::LevelMeter inputMeter;
    ZeroEQ::LevelMeter outputMeter;

    int currentProgramIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZeroEQAudioProcessor)
};
