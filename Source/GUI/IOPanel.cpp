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

    for (auto* l : { &inputPeakLabel, &outputPeakLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setFont(juce::Font(juce::FontOptions(9.0f)));
        l->setColour(juce::Label::textColourId, ZeroEQLookAndFeel::textDim);
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
    auto& in = audioProcessor.getInputMeter();
    inputReading = { in.getPeakDb(), in.getTruePeakDb(), in.getVuDb(), in.isClipping() };

    auto& out = audioProcessor.getOutputMeter();
    outputReading = { out.getPeakDb(), out.getTruePeakDb(), out.getVuDb(), out.isClipping() };

    inputPeakLabel.setText(inputReading.peakDb <= -99.0f ? juce::String("-inf")
                                                          : juce::String(inputReading.peakDb, 1),
                            juce::dontSendNotification);
    outputPeakLabel.setText(outputReading.peakDb <= -99.0f ? juce::String("-inf")
                                                            : juce::String(outputReading.peakDb, 1),
                             juce::dontSendNotification);

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

    auto drawMeter = [&](juce::Rectangle<int> bounds, const MeterReading& reading)
    {
        auto meterBounds = bounds.toFloat();
        g.setColour(ZeroEQLookAndFeel::gridLine);
        g.fillRoundedRectangle(meterBounds, 2.0f);

        // VU body: slow-integrating "average loudness" fill.
        const float vuProportion = dbToMeterProportion(reading.vuDb);
        auto vuFillBounds = meterBounds.removeFromBottom(meterBounds.getHeight() * vuProportion);
        g.setColour(ZeroEQLookAndFeel::curveGreen.withAlpha(0.75f));
        g.fillRoundedRectangle(vuFillBounds, 2.0f);

        // Fast peak tick riding on top of the VU body.
        const float peakProportion = dbToMeterProportion(reading.peakDb);
        const float peakY = bounds.getY() + bounds.getHeight() * (1.0f - peakProportion);
        g.setColour(reading.peakDb > -0.1f ? juce::Colours::red : ZeroEQLookAndFeel::accentOrange);
        g.fillRect(juce::Rectangle<float>((float) bounds.getX(), peakY - 1.0f, (float) bounds.getWidth(), 2.0f));

        // Clip indicator: brief red cap at the very top of the meter.
        if (reading.clipping)
        {
            g.setColour(juce::Colours::red);
            g.fillRoundedRectangle(juce::Rectangle<float>((float) bounds.getX(), (float) bounds.getY(),
                                                            (float) bounds.getWidth(), 3.0f),
                                    1.0f);
        }
    };

    drawMeter(inputMeterBounds, inputReading);
    drawMeter(outputMeterBounds, outputReading);
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

    inputPeakLabel.setBounds(inputArea.removeFromBottom(14));
    outputPeakLabel.setBounds(outputArea.removeFromBottom(14));

    inputMeterBounds = inputArea.removeFromRight(8).reduced(0, 4);
    outputMeterBounds = outputArea.removeFromRight(8).reduced(0, 4);

    inputSlider.setBounds(inputArea.reduced(4, 0));
    outputSlider.setBounds(outputArea.reduced(4, 0));
}

} // namespace ZeroEQ
