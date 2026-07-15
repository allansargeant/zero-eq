#pragma once

#include <JuceHeader.h>

namespace ZeroEQ
{

class ZeroEQLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ZeroEQLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    static const juce::Colour backgroundDark;
    static const juce::Colour backgroundPanel;
    static const juce::Colour gridLine;
    static const juce::Colour curveGreen;
    static const juce::Colour spectrumPre;
    static const juce::Colour spectrumPost;
    static const juce::Colour accentOrange;
    static const juce::Colour textDim;
};

} // namespace ZeroEQ
