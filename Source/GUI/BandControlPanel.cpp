#include "BandControlPanel.h"
#include "ZeroEQLookAndFeel.h"

namespace ZeroEQ
{

BandControlPanel::BandControlPanel(ZeroEQAudioProcessor& processorToUse)
    : audioProcessor(processorToUse)
{
    bandTitle.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    bandTitle.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(bandTitle);

    auto setupLabel = [this](juce::Label& l, const juce::String& text)
    {
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setFont(juce::Font(juce::FontOptions(11.0f)));
        addAndMakeVisible(l);
    };
    setupLabel(typeLabel, "Type");
    setupLabel(freqLabel, "Freq");
    setupLabel(gainLabel, "Gain");
    setupLabel(qLabel, "Q");
    setupLabel(characterLabel, "Character");
    setupLabel(slopeLabel, "Slope");

    typeBox.addItemList(filterTypeNames(), 1);
    addAndMakeVisible(typeBox);

    characterBox.addItemList(filterCharacterNames(), 1);
    addAndMakeVisible(characterBox);

    slopeBox.addItemList(filterSlopeNames(), 1);
    addAndMakeVisible(slopeBox);

    for (auto* s : { &freqSlider, &gainSlider, &qSlider })
    {
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        addAndMakeVisible(*s);
    }

    addAndMakeVisible(activeButton);
    addAndMakeVisible(soloButton);

    typeBox.onChange = [this] { updateSlopeVisibility(); };

    setSelectedBand(0);
}

void BandControlPanel::setSelectedBand(int band)
{
    currentBand = juce::jlimit(0, numBands - 1, band);
    bandTitle.setText("Band " + juce::String(currentBand + 1), juce::dontSendNotification);
    rebuildAttachments();
    updateSlopeVisibility();
}

void BandControlPanel::rebuildAttachments()
{
    auto& apvts = audioProcessor.apvts;
    const int i = currentBand;

    freqAttachment.reset();
    gainAttachment.reset();
    qAttachment.reset();
    typeAttachment.reset();
    characterAttachment.reset();
    slopeAttachment.reset();
    activeAttachment.reset();
    soloAttachment.reset();

    freqAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandFreq(i), freqSlider);
    gainAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandGain(i), gainSlider);
    qAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandQ(i), qSlider);
    typeAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandType(i), typeBox);
    characterAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandCharacter(i), characterBox);
    slopeAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandSlope(i), slopeBox);
    activeAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bandActive(i), activeButton);
    soloAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bandSolo(i), soloButton);
}

void BandControlPanel::updateSlopeVisibility()
{
    const auto type = (FilterType) typeBox.getSelectedItemIndex();
    const bool showSlope = (type == FilterType::HighPass || type == FilterType::LowPass);
    slopeBox.setVisible(showSlope);
    slopeLabel.setVisible(showSlope);

    const bool showGain = (type == FilterType::Bell || type == FilterType::LowShelf
                         || type == FilterType::HighShelf || type == FilterType::TiltShelf);
    gainSlider.setEnabled(showGain);
    gainLabel.setEnabled(showGain);
}

void BandControlPanel::paint(juce::Graphics& g)
{
    g.setColour(ZeroEQLookAndFeel::backgroundPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 6.0f, 1.0f);
}

void BandControlPanel::resized()
{
    auto area = getLocalBounds().reduced(10);

    auto top = area.removeFromTop(24);
    bandTitle.setBounds(top.removeFromLeft(80));
    activeButton.setBounds(top.removeFromRight(50).reduced(2));
    soloButton.setBounds(top.removeFromRight(55).reduced(2));

    area.removeFromTop(6);
    auto comboRow = area.removeFromTop(42);
    const int comboWidth = comboRow.getWidth() / 3;

    auto typeArea = comboRow.removeFromLeft(comboWidth);
    typeLabel.setBounds(typeArea.removeFromTop(14));
    typeBox.setBounds(typeArea.reduced(2, 0));

    auto charArea = comboRow.removeFromLeft(comboWidth);
    characterLabel.setBounds(charArea.removeFromTop(14));
    characterBox.setBounds(charArea.reduced(2, 0));

    auto slopeArea = comboRow;
    slopeLabel.setBounds(slopeArea.removeFromTop(14));
    slopeBox.setBounds(slopeArea.reduced(2, 0));

    area.removeFromTop(10);
    const int colWidth = area.getWidth() / 3;
    const int knobDiameter = juce::jlimit(48, 90, juce::jmin(colWidth - 10, area.getHeight() - 32));

    auto placeKnob = [&](juce::Rectangle<int> col, juce::Label& label, juce::Slider& slider)
    {
        label.setBounds(col.removeFromTop(14));
        const int w = juce::jmax(knobDiameter, 60);
        auto knobBounds = col.removeFromTop(knobDiameter + 18).withSizeKeepingCentre(w, knobDiameter + 18);
        slider.setBounds(knobBounds);
    };

    placeKnob(area.removeFromLeft(colWidth), freqLabel, freqSlider);
    placeKnob(area.removeFromLeft(colWidth), gainLabel, gainSlider);
    placeKnob(area, qLabel, qSlider);
}

} // namespace ZeroEQ
