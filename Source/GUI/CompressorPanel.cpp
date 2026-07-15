#include "CompressorPanel.h"
#include "ZeroEQLookAndFeel.h"

namespace ZeroEQ
{

CompressorPanel::CompressorPanel(ZeroEQAudioProcessor& processorToUse)
    : audioProcessor(processorToUse)
{
    title.setText("Compressor", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    addAndMakeVisible(title);

    addAndMakeVisible(activeButton);
    addAndMakeVisible(autoMakeupButton);

    auto setupLabel = [this](juce::Label& l, const juce::String& text)
    {
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setFont(juce::Font(juce::FontOptions(11.0f)));
        addAndMakeVisible(l);
    };
    setupLabel(thresholdLabel, "Threshold");
    setupLabel(ratioLabel, "Ratio");
    setupLabel(attackLabel, "Attack");
    setupLabel(releaseLabel, "Release");
    setupLabel(kneeLabel, "Knee");
    setupLabel(makeupLabel, "Makeup");
    setupLabel(detectorLabel, "Detector");

    for (auto* s : { &thresholdSlider, &ratioSlider, &attackSlider, &releaseSlider, &kneeSlider, &makeupSlider })
    {
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 18);
        addAndMakeVisible(*s);
    }

    detectorBox.addItemList(detectorNames(), 1);
    addAndMakeVisible(detectorBox);

    auto& apvts = audioProcessor.apvts;
    thresholdAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::compThreshold, thresholdSlider);
    ratioAttachment     = std::make_unique<SliderAttachment>(apvts, ParamIDs::compRatio, ratioSlider);
    attackAttachment    = std::make_unique<SliderAttachment>(apvts, ParamIDs::compAttack, attackSlider);
    releaseAttachment   = std::make_unique<SliderAttachment>(apvts, ParamIDs::compRelease, releaseSlider);
    kneeAttachment      = std::make_unique<SliderAttachment>(apvts, ParamIDs::compKnee, kneeSlider);
    makeupAttachment    = std::make_unique<SliderAttachment>(apvts, ParamIDs::compMakeup, makeupSlider);
    detectorAttachment  = std::make_unique<ComboAttachment>(apvts, ParamIDs::compDetector, detectorBox);
    activeAttachment     = std::make_unique<ButtonAttachment>(apvts, ParamIDs::compActive, activeButton);
    autoMakeupAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::compAutoMakeup, autoMakeupButton);

    startTimerHz(30);
}

CompressorPanel::~CompressorPanel()
{
    stopTimer();
}

void CompressorPanel::timerCallback()
{
    const float target = audioProcessor.getCompressor().getCurrentGainReductionDb();
    if (! juce::approximatelyEqual(target, currentGainReductionDb))
    {
        currentGainReductionDb = target;
        repaint();
    }
}

void CompressorPanel::paint(juce::Graphics& g)
{
    g.setColour(ZeroEQLookAndFeel::backgroundPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 6.0f, 1.0f);

    // gain reduction meter, right-hand vertical strip
    auto meterBounds = getLocalBounds().removeFromRight(18).reduced(0, 30).toFloat();
    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.fillRoundedRectangle(meterBounds, 2.0f);

    const float maxReductionDb = 24.0f;
    const float proportion = juce::jlimit(0.0f, 1.0f, -currentGainReductionDb / maxReductionDb);
    auto fillBounds = meterBounds.removeFromTop(meterBounds.getHeight() * proportion);
    g.setColour(ZeroEQLookAndFeel::accentOrange);
    g.fillRoundedRectangle(fillBounds, 2.0f);
}

void CompressorPanel::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromRight(20); // reserve meter strip

    auto top = area.removeFromTop(24);
    title.setBounds(top.removeFromLeft(100));
    activeButton.setBounds(top.removeFromRight(50).reduced(2));

    area.removeFromTop(6);
    auto detectorRow = area.removeFromBottom(40);
    detectorLabel.setBounds(detectorRow.removeFromTop(14));
    detectorBox.setBounds(detectorRow.removeFromLeft(detectorRow.getWidth() * 2 / 3).reduced(2));
    autoMakeupButton.setBounds(detectorRow.reduced(2));
    area.removeFromBottom(6);

    const int colWidth = area.getWidth() / 3;
    const int rowHeight = area.getHeight() / 2;
    const int knobDiameter = juce::jlimit(40, 80, juce::jmin(colWidth - 10, rowHeight - 32));

    auto placeKnob = [&](juce::Rectangle<int> col, juce::Label& label, juce::Slider& slider)
    {
        label.setBounds(col.removeFromTop(14));
        const int w = juce::jmax(knobDiameter, 56);
        auto knobBounds = col.removeFromTop(knobDiameter + 18).withSizeKeepingCentre(w, knobDiameter + 18);
        slider.setBounds(knobBounds);
    };

    auto row1 = area.removeFromTop(rowHeight);
    placeKnob(row1.removeFromLeft(colWidth), thresholdLabel, thresholdSlider);
    placeKnob(row1.removeFromLeft(colWidth), ratioLabel, ratioSlider);
    placeKnob(row1, kneeLabel, kneeSlider);

    auto row2 = area;
    placeKnob(row2.removeFromLeft(colWidth), attackLabel, attackSlider);
    placeKnob(row2.removeFromLeft(colWidth), releaseLabel, releaseSlider);
    placeKnob(row2, makeupLabel, makeupSlider);
}

} // namespace ZeroEQ
