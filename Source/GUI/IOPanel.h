#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// Input/output trim gain with live level meters, plus the master EQ bypass toggle.
class IOPanel : public juce::Component, private juce::Timer
{
public:
    explicit IOPanel(ZeroEQAudioProcessor& processorToUse);
    ~IOPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    static float dbToMeterProportion(float db);

    ZeroEQAudioProcessor& audioProcessor;

    juce::Label inputLabel, outputLabel;
    juce::Slider inputSlider, outputSlider;
    juce::ToggleButton eqActiveButton { "EQ Active" };

    float inputLevelDb = -100.0f;
    float outputLevelDb = -100.0f;

    juce::Rectangle<int> inputMeterBounds, outputMeterBounds;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment, outputAttachment;
    std::unique_ptr<ButtonAttachment> eqActiveAttachment;
};

} // namespace ZeroEQ
