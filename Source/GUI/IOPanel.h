#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// Input/output trim gain with ballistics-accurate level meters (VU-style body fill,
// fast peak tick, clip indicator), plus the master EQ bypass toggle.
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
    juce::Label inputPeakLabel, outputPeakLabel;
    juce::Slider inputSlider, outputSlider;
    juce::ToggleButton eqActiveButton { "EQ Active" };

    struct MeterReading
    {
        float peakDb = -100.0f;
        float truePeakDb = -100.0f;
        float vuDb = -100.0f;
        bool clipping = false;
    };
    MeterReading inputReading, outputReading;

    juce::Rectangle<int> inputMeterBounds, outputMeterBounds;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment, outputAttachment;
    std::unique_ptr<ButtonAttachment> eqActiveAttachment;
};

} // namespace ZeroEQ
