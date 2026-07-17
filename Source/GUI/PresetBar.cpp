#include "PresetBar.h"
#include "ZeroEQLookAndFeel.h"

namespace ZeroEQ
{

PresetBar::PresetBar(ZeroEQAudioProcessor& processorToUse)
    : audioProcessor(processorToUse)
{
    titleLabel.setText("Preset", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    titleLabel.setColour(juce::Label::textColourId, ZeroEQLookAndFeel::textDim);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(presetBox);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(nextButton);
    addAndMakeVisible(saveButton);

    refreshComboItems();
    presetBox.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);

    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id <= 0)
            return;

        const int factoryCount = (int) PresetManager::getFactoryPresets().size();
        if (id <= factoryCount)
        {
            audioProcessor.setCurrentProgram(id - 1);
        }
        else
        {
            const int userIndex = id - 1000;
            if (userIndex >= 0 && userIndex < userPresetNamesCache.size())
                audioProcessor.getPresetManager().loadUserPreset(userPresetNamesCache[userIndex]);
        }
    };

    prevButton.onClick = [this]
    {
        const int factoryCount = (int) PresetManager::getFactoryPresets().size();
        const int newIndex = (audioProcessor.getCurrentProgram() - 1 + factoryCount) % factoryCount;
        selectByProgramIndex(newIndex);
    };

    nextButton.onClick = [this]
    {
        const int factoryCount = (int) PresetManager::getFactoryPresets().size();
        const int newIndex = (audioProcessor.getCurrentProgram() + 1) % factoryCount;
        selectByProgramIndex(newIndex);
    };

    saveButton.onClick = [this] { showSaveDialog(); };
}

void PresetBar::refreshComboItems()
{
    presetBox.clear(juce::dontSendNotification);

    presetBox.addSectionHeading("Factory");
    const auto& factory = PresetManager::getFactoryPresets();
    for (int i = 0; i < (int) factory.size(); ++i)
        presetBox.addItem(factory[(size_t) i].name, i + 1);

    userPresetNamesCache = audioProcessor.getPresetManager().getUserPresetNames();
    if (! userPresetNamesCache.isEmpty())
    {
        presetBox.addSectionHeading("User");
        for (int i = 0; i < userPresetNamesCache.size(); ++i)
            presetBox.addItem(userPresetNamesCache[i], 1000 + i);
    }
}

void PresetBar::selectByProgramIndex(int index)
{
    audioProcessor.setCurrentProgram(index);
    presetBox.setSelectedId(index + 1, juce::dontSendNotification);
}

void PresetBar::showSaveDialog()
{
    saveDialog = std::make_unique<juce::AlertWindow>("Save Preset", "Enter a name for this preset:",
                                                       juce::MessageBoxIconType::NoIcon);
    saveDialog->addTextEditor("name", audioProcessor.getPresetManager().getCurrentPresetName(), "Name:");
    saveDialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    saveDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    saveDialog->enterModalState(true, juce::ModalCallbackFunction::create([this](int result)
    {
        if (result == 1)
        {
            const auto name = saveDialog->getTextEditorContents("name").trim();
            if (name.isNotEmpty())
            {
                audioProcessor.getPresetManager().saveUserPreset(name);
                refreshComboItems();

                for (int i = 0; i < userPresetNamesCache.size(); ++i)
                    if (userPresetNamesCache[i] == name)
                        presetBox.setSelectedId(1000 + i, juce::dontSendNotification);
            }
        }
        saveDialog.reset();
    }), true);
}

void PresetBar::paint(juce::Graphics& g)
{
    g.setColour(ZeroEQLookAndFeel::backgroundPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

void PresetBar::resized()
{
    auto area = getLocalBounds().reduced(4, 3);

    titleLabel.setBounds(area.removeFromLeft(50));
    area.removeFromLeft(4);

    saveButton.setBounds(area.removeFromRight(56).reduced(0, 1));
    area.removeFromRight(4);
    nextButton.setBounds(area.removeFromRight(24).reduced(0, 1));
    area.removeFromRight(2);
    prevButton.setBounds(area.removeFromRight(24).reduced(0, 1));
    area.removeFromRight(6);

    presetBox.setBounds(area.reduced(0, 1));
}

} // namespace ZeroEQ
