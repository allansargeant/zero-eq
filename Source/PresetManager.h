#pragma once

#include <JuceHeader.h>
#include "PluginParameters.h"

namespace ZeroEQ
{

// A factory preset is a name plus a sparse list of parameter overrides (in each
// parameter's native range, e.g. Hz/dB/ms) applied on top of a full reset-to-defaults.
// Any parameter not listed keeps its default value.
struct FactoryPreset
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> overrides;
};

// Owns preset load/save for the plugin: a curated set of built-in factory presets
// (covering static EQ shaping, dynamic EQ, harmonic saturation, and the compressor),
// plus save/scan/load of user presets as XML files in the standard per-user preset
// directory. All loading goes through the same APVTS the rest of the plugin already
// uses, so it's exercised by the same state save/restore path a host uses for session
// recall - no separate serialization format to keep in sync.
class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& apvtsToUse);

    static const std::vector<FactoryPreset>& getFactoryPresets();

    void resetToDefaults();
    void loadFactoryPreset(int index);

    juce::String getCurrentPresetName() const { return currentPresetName; }

    juce::File getUserPresetDirectory() const;
    juce::StringArray getUserPresetNames() const;
    bool saveUserPreset(const juce::String& presetName);
    bool loadUserPreset(const juce::String& presetName);
    bool loadUserPresetFile(const juce::File& file);

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String currentPresetName = "Init";

    void setNativeParam(const juce::String& id, float nativeValue);
};

} // namespace ZeroEQ
