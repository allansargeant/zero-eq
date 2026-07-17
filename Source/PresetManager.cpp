#include "PresetManager.h"

namespace ZeroEQ
{

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvtsToUse)
    : apvts(apvtsToUse)
{
}

void PresetManager::setNativeParam(const juce::String& id, float nativeValue)
{
    auto* p = apvts.getParameter(id);
    jassert(p != nullptr);
    if (p != nullptr)
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(nativeValue));
}

void PresetManager::resetToDefaults()
{
    for (auto& id : getAllParameterIDs())
    {
        auto* p = apvts.getParameter(id);
        if (p != nullptr)
            p->setValueNotifyingHost(p->getDefaultValue());
    }
}

void PresetManager::loadFactoryPreset(int index)
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= (int) presets.size())
        return;

    resetToDefaults();
    for (auto& override_ : presets[(size_t) index].overrides)
        setNativeParam(override_.first, override_.second);

    currentPresetName = presets[(size_t) index].name;
}

juce::File PresetManager::getUserPresetDirectory() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                   .getChildFile("Library/Audio/Presets")
                   .getChildFile(JucePlugin_Manufacturer)
                   .getChildFile(JucePlugin_Name);
    return dir;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray names;
    auto dir = getUserPresetDirectory();
    if (! dir.isDirectory())
        return names;

    for (const auto& entry : juce::RangedDirectoryIterator(dir, false, "*.xml"))
        names.add(entry.getFile().getFileNameWithoutExtension());

    names.sort(true);
    return names;
}

bool PresetManager::saveUserPreset(const juce::String& presetName)
{
    auto dir = getUserPresetDirectory();
    if (! dir.isDirectory())
    {
        auto result = dir.createDirectory();
        if (result.failed())
            return false;
    }

    auto file = dir.getChildFile(juce::File::createLegalFileName(presetName) + ".xml");
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml == nullptr)
        return false;

    const bool ok = xml->writeTo(file);
    if (ok)
        currentPresetName = presetName;
    return ok;
}

bool PresetManager::loadUserPreset(const juce::String& presetName)
{
    auto file = getUserPresetDirectory().getChildFile(juce::File::createLegalFileName(presetName) + ".xml");
    return loadUserPresetFile(file);
}

bool PresetManager::loadUserPresetFile(const juce::File& file)
{
    if (! file.existsAsFile())
        return false;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || ! xml->hasTagName(apvts.state.getType()))
        return false;

    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    currentPresetName = file.getFileNameWithoutExtension();
    return true;
}

const std::vector<FactoryPreset>& PresetManager::getFactoryPresets()
{
    // Band indices below are 0-based (Band 1 = index 0 ... Band 8 = index 7). Default
    // per-band frequencies are 60/150/400/1000/2500/5000/9000/15000 Hz, band 0 defaults
    // to High Pass and band 7 to Low Pass, everything else Bell - see
    // PluginParameters.h. Every preset starts from a full reset-to-defaults, so any
    // parameter not listed here just keeps its default value.
    static const std::vector<FactoryPreset> presets = {
        { "Init", {} },

        { "Vocal Presence", {
            { ParamIDs::bandFreq(0), 100.0f },
            { ParamIDs::bandFreq(1), 250.0f },  { ParamIDs::bandGain(1), -2.0f }, { ParamIDs::bandQ(1), 1.0f },
            { ParamIDs::bandFreq(2), 3500.0f }, { ParamIDs::bandGain(2), 3.0f },  { ParamIDs::bandQ(2), 1.0f },
            { ParamIDs::bandActive(3), 0.0f },
            { ParamIDs::bandActive(4), 0.0f },
            { ParamIDs::bandType(5), (float) FilterType::HighShelf }, { ParamIDs::bandFreq(5), 10000.0f },
            { ParamIDs::bandGain(5), 2.0f }, { ParamIDs::bandCharacter(5), (float) FilterCharacter::Vintage },
            { ParamIDs::bandActive(6), 0.0f },
        } },

        { "De-Esser (Dynamic)", {
            { ParamIDs::bandFreq(0), 80.0f },
            { ParamIDs::bandActive(1), 0.0f },
            { ParamIDs::bandActive(2), 0.0f },
            { ParamIDs::bandActive(3), 0.0f },
            { ParamIDs::bandActive(4), 0.0f },
            { ParamIDs::bandFreq(5), 6500.0f }, { ParamIDs::bandQ(5), 4.0f },
            { ParamIDs::bandDynActive(5), 1.0f }, { ParamIDs::bandDynDirection(5), (float) DynamicDirection::Downward },
            { ParamIDs::bandDynThreshold(5), -28.0f }, { ParamIDs::bandDynRatio(5), 6.0f },
            { ParamIDs::bandDynAttack(5), 1.0f }, { ParamIDs::bandDynRelease(5), 60.0f }, { ParamIDs::bandDynRange(5), 10.0f },
            { ParamIDs::bandActive(6), 0.0f },
        } },

        { "Warm Bus (Harmonic)", {
            { ParamIDs::bandFreq(0), 30.0f },
            { ParamIDs::bandGain(1), 2.0f }, { ParamIDs::bandCharacter(1), (float) FilterCharacter::Harmonic },
            { ParamIDs::bandHarmonicBlend(1), 0.2f },
            { ParamIDs::bandActive(2), 0.0f },
            { ParamIDs::bandActive(3), 0.0f },
            { ParamIDs::bandActive(4), 0.0f },
            { ParamIDs::bandType(5), (float) FilterType::HighShelf }, { ParamIDs::bandFreq(5), 8000.0f },
            { ParamIDs::bandGain(5), 1.5f }, { ParamIDs::bandCharacter(5), (float) FilterCharacter::Harmonic },
            { ParamIDs::bandHarmonicBlend(5), 0.7f },
            { ParamIDs::bandActive(6), 0.0f },
            { ParamIDs::compActive, 1.0f }, { ParamIDs::compThreshold, -20.0f }, { ParamIDs::compRatio, 2.0f },
            { ParamIDs::compAttack, 15.0f }, { ParamIDs::compRelease, 150.0f }, { ParamIDs::compKnee, 6.0f },
            { ParamIDs::compAutoMakeup, 1.0f }, { ParamIDs::compDetector, (float) DetectorType::RMS },
        } },

        { "Podcast Voice", {
            { ParamIDs::bandFreq(0), 90.0f },
            { ParamIDs::bandActive(1), 0.0f },
            { ParamIDs::bandGain(2), -1.5f }, { ParamIDs::bandQ(2), 1.0f },
            { ParamIDs::bandActive(3), 0.0f },
            { ParamIDs::bandGain(4), 2.0f }, { ParamIDs::bandQ(4), 1.2f },
            { ParamIDs::bandFreq(5), 4000.0f }, { ParamIDs::bandQ(5), 3.0f },
            { ParamIDs::bandDynActive(5), 1.0f }, { ParamIDs::bandDynDirection(5), (float) DynamicDirection::Downward },
            { ParamIDs::bandDynThreshold(5), -22.0f }, { ParamIDs::bandDynRatio(5), 3.0f },
            { ParamIDs::bandDynAttack(5), 5.0f }, { ParamIDs::bandDynRelease(5), 100.0f }, { ParamIDs::bandDynRange(5), 6.0f },
            { ParamIDs::bandActive(6), 0.0f },
            { ParamIDs::compActive, 1.0f }, { ParamIDs::compThreshold, -18.0f }, { ParamIDs::compRatio, 3.0f },
            { ParamIDs::compAttack, 8.0f }, { ParamIDs::compRelease, 120.0f }, { ParamIDs::compKnee, 6.0f },
            { ParamIDs::compAutoMakeup, 1.0f }, { ParamIDs::compDetector, (float) DetectorType::RMS },
        } },

        { "Broadcast Loudness", {
            { ParamIDs::bandFreq(0), 40.0f },
            { ParamIDs::bandActive(1), 0.0f },
            { ParamIDs::bandActive(2), 0.0f },
            { ParamIDs::bandActive(3), 0.0f },
            { ParamIDs::bandActive(4), 0.0f },
            { ParamIDs::bandType(5), (float) FilterType::HighShelf }, { ParamIDs::bandFreq(5), 6000.0f },
            { ParamIDs::bandGain(5), 1.5f },
            { ParamIDs::bandActive(6), 0.0f },
            { ParamIDs::compActive, 1.0f }, { ParamIDs::compThreshold, -16.0f }, { ParamIDs::compRatio, 4.0f },
            { ParamIDs::compAttack, 5.0f }, { ParamIDs::compRelease, 80.0f }, { ParamIDs::compKnee, 4.0f },
            { ParamIDs::compAutoMakeup, 1.0f }, { ParamIDs::compDetector, (float) DetectorType::RMS },
        } },

        { "Telephone / Lo-Fi", {
            { ParamIDs::bandFreq(0), 300.0f }, { ParamIDs::bandSlope(0), (float) FilterSlope::Slope48 },
            { ParamIDs::bandActive(1), 0.0f },
            { ParamIDs::bandFreq(2), 1000.0f }, { ParamIDs::bandGain(2), 6.0f }, { ParamIDs::bandQ(2), 1.5f },
            { ParamIDs::bandCharacter(2), (float) FilterCharacter::Harmonic }, { ParamIDs::bandHarmonicBlend(2), 0.85f },
            { ParamIDs::bandActive(3), 0.0f },
            { ParamIDs::bandActive(4), 0.0f },
            { ParamIDs::bandActive(5), 0.0f },
            { ParamIDs::bandActive(6), 0.0f },
            { ParamIDs::bandFreq(7), 3400.0f }, { ParamIDs::bandSlope(7), (float) FilterSlope::Slope48 },
            { ParamIDs::compActive, 1.0f }, { ParamIDs::compThreshold, -24.0f }, { ParamIDs::compRatio, 6.0f },
            { ParamIDs::compAttack, 2.0f }, { ParamIDs::compRelease, 60.0f }, { ParamIDs::compKnee, 2.0f },
            { ParamIDs::compAutoMakeup, 1.0f }, { ParamIDs::compDetector, (float) DetectorType::Peak },
        } },
    };
    return presets;
}

} // namespace ZeroEQ
