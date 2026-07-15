#include "IOPanel.h"
#include "ZeroEQLookAndFeel.h"

namespace ZeroEQ
{

IOPanel::IOPanel(ZeroEQAudioProcessor& processorToUse)
    : audioProcessor(processorToUse)
{
    inputLabel.setText("Input", juce::dontSendNotification);
    outputLabel.setText("Output", juce::dontSendNotification);
    for (auto* l : { &inputLabel, &outputLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        addAndMakeVisible(*l);
    }

    for (auto* s : { &inputSlider, &outputSlider })
    {
        s->setSliderStyle(juce::Slider::LinearVertical);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        addAndMakeVisible(*s);
    }

    addAndMakeVisible(eqActiveButton);

    auto& apvts = audioProcessor.apvts;
    inputAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::inputGain, inputSlider);
    outputAttachment = std::make_unique<SliderAttachment>(apvts, ParamIDs::outputGain, outputSlider);
    eqActiveAttachment = std::make_unique<ButtonAttachment>(apvts, ParamIDs::eqActive, eqActiveButton);

    startTimerHz(30);
}

IOPanel::~IOPanel()
{
    stopTimer();
}

void IOPanel::timerCallback()
{
    inputLevelDb = audioProcessor.getInputLevelDb();
    outputLevelDb = audioProcessor.getOutputLevelDb();
    repaint();
}

float IOPanel::dbToMeterProportion(float db)
{
    // -60 dB floor to 0 dB (peaks above 0 clip visually at the top).
    return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 60.0f);
}

void IOPanel::paint(juce::Graphics& g)
{
    g.setColour(ZeroEQLookAndFeel::backgroundPanel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 6.0f, 1.0f);

    auto drawMeter = [&](juce::Rectangle<int> bounds, float db)
    {
        auto meterBounds = bounds.toFloat();
        g.setColour(ZeroEQLookAndFeel::gridLine);
        g.fillRoundedRectangle(meterBounds, 2.0f);

        const float proportion = dbToMeterProportion(db);
        auto fillBounds = meterBounds.removeFromBottom(meterBounds.getHeight() * proportion);
        g.setColour(db > -0.5f ? juce::Colours::red : ZeroEQLookAndFeel::curveGreen);
        g.fillRoundedRectangle(fillBounds, 2.0f);
    };

    drawMeter(inputMeterBounds, inputLevelDb);
    drawMeter(outputMeterBounds, outputLevelDb);
}

void IOPanel::resized()
{
    auto area = getLocalBounds().reduced(10);

    auto eqRow = area.removeFromTop(24);
    eqActiveButton.setBounds(eqRow);

    area.removeFromTop(8);

    const int half = area.getWidth() / 2;
    auto inputArea = area.removeFromLeft(half);
    auto outputArea = area;

    inputLabel.setBounds(inputArea.removeFromTop(16));
    outputLabel.setBounds(outputArea.removeFromTop(16));

    inputMeterBounds = inputArea.removeFromRight(8).reduced(0, 4);
    outputMeterBounds = outputArea.removeFromRight(8).reduced(0, 4);

    inputSlider.setBounds(inputArea.reduced(4, 0));
    outputSlider.setBounds(outputArea.reduced(4, 0));
}

} // namespace ZeroEQ
