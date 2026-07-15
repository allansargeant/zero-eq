#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// Numeric editor for whichever band is currently selected in the EQCurveComponent.
class BandControlPanel : public juce::Component
{
public:
    explicit BandControlPanel(ZeroEQAudioProcessor& processorToUse);

    void resized() override;
    void paint(juce::Graphics&) override;

    void setSelectedBand(int band);

private:
    void rebuildAttachments();
    void updateSlopeVisibility();

    ZeroEQAudioProcessor& audioProcessor;
    int currentBand = 0;

    juce::Label bandTitle;
    juce::ComboBox typeBox, characterBox, slopeBox;
    juce::Slider freqSlider, gainSlider, qSlider;
    juce::Label typeLabel, freqLabel, gainLabel, qLabel, characterLabel, slopeLabel;
    juce::ToggleButton activeButton { "On" };
    juce::ToggleButton soloButton { "Solo" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> freqAttachment, gainAttachment, qAttachment;
    std::unique_ptr<ComboAttachment> typeAttachment, characterAttachment, slopeAttachment;
    std::unique_ptr<ButtonAttachment> activeAttachment, soloAttachment;
};

} // namespace ZeroEQ
