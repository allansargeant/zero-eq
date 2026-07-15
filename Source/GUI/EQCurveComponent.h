#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

namespace ZeroEQ
{

// The main interactive area: live spectrum analyzer underneath a draggable
// per-band curve. Drag a node to set frequency (X) and gain (Y); for gain-less bands
// (HP/LP/notch/bandpass) vertical drag sets Q instead. Scroll wheel always adjusts Q.
class EQCurveComponent : public juce::Component, private juce::Timer
{
public:
    explicit EQCurveComponent(ZeroEQAudioProcessor& processorToUse);
    ~EQCurveComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;

    std::function<void(int)> onBandSelected;
    int getSelectedBand() const { return selectedBand; }
    void setSelectedBand(int band);

private:
    void timerCallback() override;

    float freqToX(float freqHz) const;
    float xToFreq(float x) const;
    float dbToY(float db) const;
    float yToDb(float y) const;

    int findNodeAt(juce::Point<float> pos) const;
    static bool typeHasGain(FilterType type);

    juce::Colour colourForBand(int index) const;

    ZeroEQAudioProcessor& audioProcessor;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float minDb = -24.0f;
    static constexpr float maxDb = 24.0f;
    static constexpr float nodeRadius = 8.0f;

    int selectedBand = -1;
    int hoveredBand = -1;
    int draggingBand = -1;
    bool draggingQ = false;

    juce::Point<float> dragStartMouse;
    float dragStartQ = 0.707f;

    std::array<float, SpectrumAnalyzer::scopeSize> preSpectrumDb {};
    std::array<float, SpectrumAnalyzer::scopeSize> postSpectrumDb {};
};

} // namespace ZeroEQ
