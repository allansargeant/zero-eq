#include "ZeroEQLookAndFeel.h"

namespace ZeroEQ
{

const juce::Colour ZeroEQLookAndFeel::backgroundDark  { 0xff14171c };
const juce::Colour ZeroEQLookAndFeel::backgroundPanel { 0xff1c2027 };
const juce::Colour ZeroEQLookAndFeel::gridLine        { 0xff2c323c };
const juce::Colour ZeroEQLookAndFeel::curveGreen      { 0xff5ee0a0 };
const juce::Colour ZeroEQLookAndFeel::spectrumPre     { 0x555ee0a0 };
const juce::Colour ZeroEQLookAndFeel::spectrumPost    { 0xaaffb454 };
const juce::Colour ZeroEQLookAndFeel::accentOrange    { 0xffffb454 };
const juce::Colour ZeroEQLookAndFeel::textDim         { 0xff8a93a3 };

ZeroEQLookAndFeel::ZeroEQLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, backgroundDark);
    setColour(juce::Slider::thumbColourId, accentOrange);
    setColour(juce::Slider::trackColourId, curveGreen);
    setColour(juce::Slider::backgroundColourId, gridLine);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxBackgroundColourId, backgroundPanel);
    setColour(juce::Slider::textBoxOutlineColourId, gridLine);
    setColour(juce::Label::textColourId, textDim);
    setColour(juce::ComboBox::backgroundColourId, backgroundPanel);
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, gridLine);
    setColour(juce::TextButton::buttonColourId, backgroundPanel);
    setColour(juce::TextButton::buttonOnColourId, accentOrange);
    setColour(juce::TextButton::textColourOffId, textDim);
    setColour(juce::TextButton::textColourOnId, backgroundDark);
    setColour(juce::ToggleButton::textColourId, textDim);
}

void ZeroEQLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(4.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // track
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(gridLine);
    g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // value arc
    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(slider.isEnabled() ? accentOrange : textDim);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // pointer
    juce::Path pointer;
    const float pointerLength = radius * 0.62f;
    pointer.addRectangle(-1.3f, -pointerLength, 2.6f, pointerLength);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre));
    g.setColour(juce::Colours::white);
    g.fillPath(pointer);
}

void ZeroEQLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

void ZeroEQLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool on = button.getToggleState();

    g.setColour(on ? accentOrange : backgroundPanel);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(gridLine);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    g.setColour(on ? backgroundDark : (shouldDrawButtonAsHighlighted ? juce::Colours::white : textDim));
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

} // namespace ZeroEQ
