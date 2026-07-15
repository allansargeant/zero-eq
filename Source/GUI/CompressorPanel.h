#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// Post-EQ compressor controls plus a live gain-reduction meter.
class CompressorPanel : public juce::Component, private juce::Timer
{
public:
    explicit CompressorPanel(ZeroEQAudioProcessor& processorToUse);
    ~CompressorPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    ZeroEQAudioProcessor& audioProcessor;

    juce::Label title;
    juce::ToggleButton activeButton { "Comp" };
    juce::ToggleButton autoMakeupButton { "Auto" };

    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider, kneeSlider, makeupSlider;
    juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel, kneeLabel, makeupLabel, detectorLabel;
    juce::ComboBox detectorBox;

    float currentGainReductionDb = 0.0f;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> thresholdAttachment, ratioAttachment, attackAttachment,
                                       releaseAttachment, kneeAttachment, makeupAttachment;
    std::unique_ptr<ComboAttachment> detectorAttachment;
    std::unique_ptr<ButtonAttachment> activeAttachment, autoMakeupAttachment;
};

} // namespace ZeroEQ
