#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// Preset browsing/saving row: a combo box listing factory presets (kept in sync with
// the host's VST3/AU "program" selection) and user presets (scanned from the standard
// per-user preset directory), prev/next arrows, and a Save button. Loading a preset
// just writes parameter values through the same APVTS every control in this plugin is
// already attached to, so every knob/combo/toggle refreshes itself automatically.
class PresetBar : public juce::Component
{
public:
    explicit PresetBar(ZeroEQAudioProcessor& processorToUse);

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    void refreshComboItems();
    void selectByProgramIndex(int index);
    void showSaveDialog();

    ZeroEQAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::ComboBox presetBox;
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::TextButton saveButton { "Save" };

    juce::StringArray userPresetNamesCache;
    std::unique_ptr<juce::AlertWindow> saveDialog;
};

} // namespace ZeroEQ
