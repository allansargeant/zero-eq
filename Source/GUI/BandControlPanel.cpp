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
    setupLabel(dynDirectionLabel, "Direction");
    setupLabel(dynThresholdLabel, "Thresh");
    setupLabel(dynRatioLabel, "Ratio");
    setupLabel(dynAttackLabel, "Attack");
    setupLabel(dynReleaseLabel, "Release");
    setupLabel(dynRangeLabel, "Range");
    setupLabel(harmonicBlendLabel, "Harmonic Blend (Even / Odd)");

    dynSectionLabel.setText("Dynamic EQ", juce::dontSendNotification);
    dynSectionLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    dynSectionLabel.setJustificationType(juce::Justification::centredLeft);
    dynSectionLabel.setColour(juce::Label::textColourId, ZeroEQLookAndFeel::textDim);
    addAndMakeVisible(dynSectionLabel);

    typeBox.addItemList(filterTypeNames(), 1);
    addAndMakeVisible(typeBox);

    characterBox.addItemList(filterCharacterNames(), 1);
    addAndMakeVisible(characterBox);

    slopeBox.addItemList(filterSlopeNames(), 1);
    addAndMakeVisible(slopeBox);

    dynDirectionBox.addItemList(dynamicDirectionNames(), 1);
    addAndMakeVisible(dynDirectionBox);

    for (auto* s : { &freqSlider, &gainSlider, &qSlider })
    {
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        addAndMakeVisible(*s);
    }

    for (auto* s : { &dynThresholdSlider, &dynRatioSlider, &dynAttackSlider, &dynReleaseSlider, &dynRangeSlider })
    {
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 16);
        addAndMakeVisible(*s);
    }

    harmonicBlendSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    harmonicBlendSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);
    addAndMakeVisible(harmonicBlendSlider);

    addAndMakeVisible(activeButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(dynActiveButton);
    addAndMakeVisible(dynSidechainButton);

    typeBox.onChange = [this] { updateControlAvailability(); };
    characterBox.onChange = [this] { updateControlAvailability(); };
    dynActiveButton.onClick = [this] { updateControlAvailability(); };

    startTimerHz(30);
    setSelectedBand(0);
}

BandControlPanel::~BandControlPanel()
{
    stopTimer();
}

void BandControlPanel::setSelectedBand(int band)
{
    currentBand = juce::jlimit(0, numBands - 1, band);
    bandTitle.setText("Band " + juce::String(currentBand + 1), juce::dontSendNotification);
    rebuildAttachments();
    updateControlAvailability();
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
    dynActiveAttachment.reset();
    dynSidechainAttachment.reset();
    dynDirectionAttachment.reset();
    dynThresholdAttachment.reset();
    dynRatioAttachment.reset();
    dynAttackAttachment.reset();
    dynReleaseAttachment.reset();
    dynRangeAttachment.reset();
    harmonicBlendAttachment.reset();

    freqAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandFreq(i), freqSlider);
    gainAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandGain(i), gainSlider);
    qAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandQ(i), qSlider);
    typeAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandType(i), typeBox);
    characterAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandCharacter(i), characterBox);
    slopeAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandSlope(i), slopeBox);
    activeAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bandActive(i), activeButton);
    soloAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bandSolo(i), soloButton);

    dynActiveAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bandDynActive(i), dynActiveButton);
    dynSidechainAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::bandDynSidechain(i), dynSidechainButton);
    dynDirectionAttachment = std::make_unique<ComboAttachment>(apvts, ParamIDs::bandDynDirection(i), dynDirectionBox);
    dynThresholdAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandDynThreshold(i), dynThresholdSlider);
    dynRatioAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandDynRatio(i), dynRatioSlider);
    dynAttackAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandDynAttack(i), dynAttackSlider);
    dynReleaseAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandDynRelease(i), dynReleaseSlider);
    dynRangeAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandDynRange(i), dynRangeSlider);

    harmonicBlendAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::bandHarmonicBlend(i), harmonicBlendSlider);
}

void BandControlPanel::updateControlAvailability()
{
    const auto type = (FilterType) typeBox.getSelectedItemIndex();
    const bool showSlope = (type == FilterType::HighPass || type == FilterType::LowPass);
    slopeBox.setVisible(showSlope);
    slopeLabel.setVisible(showSlope);

    const bool showGain = filterTypeHasGain(type);
    gainSlider.setEnabled(showGain);
    gainLabel.setEnabled(showGain);

    // Dynamic EQ only makes sense for gain-having band types.
    const bool dynAvailable = showGain;
    dynActiveButton.setEnabled(dynAvailable);

    const bool dynControlsEnabled = dynAvailable && dynActiveButton.getToggleState();
    for (auto* c : { (juce::Component*) &dynDirectionBox, (juce::Component*) &dynThresholdSlider,
                      (juce::Component*) &dynRatioSlider, (juce::Component*) &dynAttackSlider,
                      (juce::Component*) &dynReleaseSlider, (juce::Component*) &dynRangeSlider,
                      (juce::Component*) &dynSidechainButton })
        c->setEnabled(dynControlsEnabled);

    for (auto* l : { &dynDirectionLabel, &dynThresholdLabel, &dynRatioLabel, &dynAttackLabel,
                      &dynReleaseLabel, &dynRangeLabel })
        l->setEnabled(dynControlsEnabled);

    const auto character = (FilterCharacter) characterBox.getSelectedItemIndex();
    const bool harmonicEnabled = (character == FilterCharacter::Harmonic) && showGain;
    harmonicBlendSlider.setEnabled(harmonicEnabled);
    harmonicBlendLabel.setEnabled(harmonicEnabled);
}

void BandControlPanel::timerCallback()
{
    const float delta = audioProcessor.getEQEngine().getBandDynamicGainDeltaDb(currentBand);
    if (! juce::approximatelyEqual(delta, currentDynGainDeltaDb))
    {
        currentDynGainDeltaDb = delta;
        repaint();
    }

    // Reflect whether the host has actually connected the sidechain bus, so a band
    // left in sidechain mode with nothing feeding it doesn't look silently broken.
    const bool connected = audioProcessor.isSidechainConnected();
    const juce::String wantedText = connected ? "Sidechain" : "Sidechain (n/c)";
    if (dynSidechainButton.getButtonText() != wantedText)
        dynSidechainButton.setButtonText(wantedText);
}

void BandControlPanel::paint(juce::Graphics& g)
{
    g.setColour(ZeroEQLookAndFeel::backgroundPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 6.0f, 1.0f);

    if (dynActiveButton.getToggleState() && dynActiveButton.isEnabled())
    {
        g.setColour(ZeroEQLookAndFeel::accentOrange);
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        auto meterArea = dynSectionLabel.getBounds().withX(dynSectionLabel.getRight() + 8).withWidth(80);
        g.drawText(juce::String(currentDynGainDeltaDb, 1) + " dB", meterArea, juce::Justification::centredLeft);
    }
}

void BandControlPanel::resized()
{
    auto area = getLocalBounds().reduced(10);

    auto top = area.removeFromTop(24);
    bandTitle.setBounds(top.removeFromLeft(70));
    dynActiveButton.setBounds(top.removeFromRight(40).reduced(2));
    soloButton.setBounds(top.removeFromRight(50).reduced(2));
    activeButton.setBounds(top.removeFromRight(45).reduced(2));

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

    area.removeFromTop(6);
    auto harmonicRow = area.removeFromTop(20);
    harmonicBlendLabel.setBounds(harmonicRow.removeFromLeft(160));
    harmonicBlendSlider.setBounds(harmonicRow.reduced(2, 0));

    area.removeFromTop(6);

    auto placeKnob = [](juce::Rectangle<int> col, juce::Label& label, juce::Slider& slider, int knobDiameter)
    {
        label.setBounds(col.removeFromTop(14));
        const int w = juce::jmax(knobDiameter, 56);
        auto knobBounds = col.removeFromTop(knobDiameter + 18).withSizeKeepingCentre(w, knobDiameter + 18);
        slider.setBounds(knobBounds);
    };

    auto mainKnobRow = area.removeFromTop(140);
    const int mainColWidth = mainKnobRow.getWidth() / 3;
    const int mainKnobDiameter = juce::jlimit(48, 100, juce::jmin(mainColWidth - 10, mainKnobRow.getHeight() - 32));
    placeKnob(mainKnobRow.removeFromLeft(mainColWidth), freqLabel, freqSlider, mainKnobDiameter);
    placeKnob(mainKnobRow.removeFromLeft(mainColWidth), gainLabel, gainSlider, mainKnobDiameter);
    placeKnob(mainKnobRow, qLabel, qSlider, mainKnobDiameter);

    area.removeFromTop(8);
    auto dynHeaderRow = area.removeFromTop(16);
    dynSectionLabel.setBounds(dynHeaderRow.removeFromLeft(90));

    area.removeFromTop(4);
    auto dynDirRow = area.removeFromTop(38);
    auto dynSidechainCol = dynDirRow.removeFromRight(100);
    dynDirectionLabel.setBounds(dynDirRow.removeFromTop(14));
    dynDirectionBox.setBounds(dynDirRow.reduced(2, 0));
    dynSidechainButton.setBounds(dynSidechainCol.removeFromBottom(24).reduced(2, 0));

    area.removeFromTop(6);
    auto dynKnobRow = area;
    const int dynColWidth = dynKnobRow.getWidth() / 5;
    const int dynKnobDiameter = juce::jlimit(36, 70, juce::jmin(dynColWidth - 8, dynKnobRow.getHeight() - 30));
    placeKnob(dynKnobRow.removeFromLeft(dynColWidth), dynThresholdLabel, dynThresholdSlider, dynKnobDiameter);
    placeKnob(dynKnobRow.removeFromLeft(dynColWidth), dynRatioLabel, dynRatioSlider, dynKnobDiameter);
    placeKnob(dynKnobRow.removeFromLeft(dynColWidth), dynAttackLabel, dynAttackSlider, dynKnobDiameter);
    placeKnob(dynKnobRow.removeFromLeft(dynColWidth), dynReleaseLabel, dynReleaseSlider, dynKnobDiameter);
    placeKnob(dynKnobRow, dynRangeLabel, dynRangeSlider, dynKnobDiameter);
}

} // namespace ZeroEQ
