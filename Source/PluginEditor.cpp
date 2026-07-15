#include "PluginEditor.h"

ZeroEQAudioProcessorEditor::ZeroEQAudioProcessorEditor(ZeroEQAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      ioPanel(p),
      eqCurve(p),
      bandControlPanel(p),
      compressorPanel(p)
{
    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(ioPanel);
    addAndMakeVisible(eqCurve);
    addAndMakeVisible(bandControlPanel);
    addAndMakeVisible(compressorPanel);

    eqCurve.onBandSelected = [this](int band)
    {
        bandControlPanel.setSelectedBand(band);
    };

    setResizable(true, true);
    setResizeLimits(950, 760, 1700, 1150);
    setSize(1150, 900);
}

ZeroEQAudioProcessorEditor::~ZeroEQAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void ZeroEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(ZeroEQ::ZeroEQLookAndFeel::backgroundDark);

    auto titleArea = getLocalBounds().removeFromTop(32);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
    g.drawText("Zero EQ", titleArea.reduced(12, 0), juce::Justification::centredLeft);

    g.setColour(ZeroEQ::ZeroEQLookAndFeel::textDim);
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText("zero-latency EQ + compressor", titleArea.reduced(12, 0), juce::Justification::centredRight);
}

void ZeroEQAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(32); // title bar

    auto ioArea = area.removeFromLeft(110).reduced(8);
    ioPanel.setBounds(ioArea);

    area.reduce(0, 8);
    auto curveArea = area.removeFromTop((int) ((float) area.getHeight() * 0.48f));
    eqCurve.setBounds(curveArea.reduced(8, 0));

    auto bottomArea = area.reduced(8, 8);
    auto bandArea = bottomArea.removeFromLeft((int) ((float) bottomArea.getWidth() * 0.42f));
    bottomArea.removeFromLeft(8);

    bandControlPanel.setBounds(bandArea);
    compressorPanel.setBounds(bottomArea);
}
