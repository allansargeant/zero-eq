#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// Numeric editor for whichever band is currently selected in the EQCurveComponent,
// including the per-band dynamic EQ controls (frequency-selective compression/
// expansion) when the band type supports gain.
class BandControlPanel : public juce::Component, private juce::Timer
{
public:
    explicit BandControlPanel(ZeroEQAudioProcessor& processorToUse);
    ~BandControlPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    void setSelectedBand(int band);

private:
    void timerCallback() override;
    void rebuildAttachments();
    void updateControlAvailability();

    ZeroEQAudioProcessor& audioProcessor;
    int currentBand = 0;
    float currentDynGainDeltaDb = 0.0f;

    juce::Label bandTitle;
    juce::ComboBox typeBox, characterBox, slopeBox;
    juce::Slider freqSlider, gainSlider, qSlider;
    juce::Label typeLabel, freqLabel, gainLabel, qLabel, characterLabel, slopeLabel;
    juce::ToggleButton activeButton { "On" };
    juce::ToggleButton soloButton { "Solo" };

    juce::Label dynSectionLabel;
    juce::ToggleButton dynActiveButton { "Dyn" };
    juce::ToggleButton dynSidechainButton { "Sidechain" };
    juce::ComboBox dynDirectionBox;
    juce::Slider dynThresholdSlider, dynRatioSlider, dynAttackSlider, dynReleaseSlider, dynRangeSlider;
    juce::Label dynDirectionLabel, dynThresholdLabel, dynRatioLabel, dynAttackLabel, dynReleaseLabel, dynRangeLabel;

    juce::Label harmonicBlendLabel;
    juce::Slider harmonicBlendSlider;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> freqAttachment, gainAttachment, qAttachment;
    std::unique_ptr<ComboAttachment> typeAttachment, characterAttachment, slopeAttachment;
    std::unique_ptr<ButtonAttachment> activeAttachment, soloAttachment;

    std::unique_ptr<ButtonAttachment> dynActiveAttachment;
    std::unique_ptr<ButtonAttachment> dynSidechainAttachment;
    std::unique_ptr<ComboAttachment> dynDirectionAttachment;
    std::unique_ptr<SliderAttachment> dynThresholdAttachment, dynRatioAttachment, dynAttackAttachment,
                                       dynReleaseAttachment, dynRangeAttachment;
    std::unique_ptr<SliderAttachment> harmonicBlendAttachment;
};

} // namespace ZeroEQ
