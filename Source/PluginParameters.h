#pragma once

#include <JuceHeader.h>

namespace ZeroEQ
{

constexpr int numBands = 8;

enum class FilterType
{
    Bell = 0,
    LowShelf,
    HighShelf,
    HighPass,
    LowPass,
    Notch,
    BandPass,
    TiltShelf,
    numTypes
};

enum class FilterCharacter
{
    Modern = 0,   // independent Q, textbook RBJ response
    Vintage,      // proportional Q: bandwidth widens with applied gain, console/passive-style
    Harmonic,     // Modern-style linear response + gain-driven even/odd harmonic saturation
    numCharacters
};

enum class FilterSlope
{
    Slope12 = 0,  // 1 cascaded biquad
    Slope24,      // 2
    Slope36,      // 3
    Slope48,      // 4
    numSlopes
};

enum class DetectorType
{
    Peak = 0,
    RMS,
    numDetectors
};

enum class DynamicDirection
{
    Downward = 0,  // duck: gain reduces as the band's own signal rises above threshold
    Upward,        // boost: gain increases as the band's own signal falls below threshold
    numDirections
};

// Only gain-having filter types (Bell/shelf/tilt) can run in dynamic mode - HP/LP/
// Notch/BandPass have no gain to modulate. Shared between the DSP detector and the GUI.
inline bool filterTypeHasGain(FilterType type)
{
    return type == FilterType::Bell || type == FilterType::LowShelf
        || type == FilterType::HighShelf || type == FilterType::TiltShelf;
}

inline juce::StringArray filterTypeNames()
{
    return { "Bell", "Low Shelf", "High Shelf", "High Pass", "Low Pass", "Notch", "Band Pass", "Tilt Shelf" };
}

inline juce::StringArray filterCharacterNames()
{
    return { "Modern", "Vintage", "Harmonic" };
}

inline juce::StringArray filterSlopeNames()
{
    return { "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" };
}

inline juce::StringArray detectorNames()
{
    return { "Peak", "RMS" };
}

inline juce::StringArray dynamicDirectionNames()
{
    return { "Downward", "Upward" };
}

namespace ParamIDs
{
    inline juce::String bandType(int i)      { return "band" + juce::String(i) + "_type"; }
    inline juce::String bandFreq(int i)      { return "band" + juce::String(i) + "_freq"; }
    inline juce::String bandGain(int i)      { return "band" + juce::String(i) + "_gain"; }
    inline juce::String bandQ(int i)         { return "band" + juce::String(i) + "_q"; }
    inline juce::String bandCharacter(int i) { return "band" + juce::String(i) + "_character"; }
    inline juce::String bandSlope(int i)     { return "band" + juce::String(i) + "_slope"; }
    inline juce::String bandActive(int i)    { return "band" + juce::String(i) + "_active"; }
    inline juce::String bandSolo(int i)      { return "band" + juce::String(i) + "_solo"; }

    inline juce::String bandDynActive(int i)    { return "band" + juce::String(i) + "_dyn_active"; }
    inline juce::String bandDynDirection(int i) { return "band" + juce::String(i) + "_dyn_direction"; }
    inline juce::String bandDynThreshold(int i) { return "band" + juce::String(i) + "_dyn_threshold"; }
    inline juce::String bandDynRatio(int i)     { return "band" + juce::String(i) + "_dyn_ratio"; }
    inline juce::String bandDynAttack(int i)    { return "band" + juce::String(i) + "_dyn_attack"; }
    inline juce::String bandDynRelease(int i)   { return "band" + juce::String(i) + "_dyn_release"; }
    inline juce::String bandDynRange(int i)     { return "band" + juce::String(i) + "_dyn_range"; }
    inline juce::String bandDynSidechain(int i) { return "band" + juce::String(i) + "_dyn_sidechain"; }

    inline juce::String bandHarmonicBlend(int i) { return "band" + juce::String(i) + "_harmonic_blend"; }

    static const juce::String inputGain  = "input_gain";
    static const juce::String outputGain = "output_gain";
    static const juce::String eqActive   = "eq_active";

    static const juce::String compActive    = "comp_active";
    static const juce::String compThreshold = "comp_threshold";
    static const juce::String compRatio     = "comp_ratio";
    static const juce::String compAttack    = "comp_attack";
    static const juce::String compRelease   = "comp_release";
    static const juce::String compKnee      = "comp_knee";
    static const juce::String compMakeup    = "comp_makeup";
    static const juce::String compAutoMakeup= "comp_auto_makeup";
    static const juce::String compDetector  = "comp_detector";
}

// Every parameter ID this plugin registers, in the same order as createParameterLayout().
// Used by the preset system for reset-to-default and applying named-value overrides
// without needing to touch AudioProcessorValueTreeState internals.
inline std::vector<juce::String> getAllParameterIDs()
{
    std::vector<juce::String> ids;
    for (int i = 0; i < numBands; ++i)
    {
        ids.push_back(ParamIDs::bandType(i));
        ids.push_back(ParamIDs::bandFreq(i));
        ids.push_back(ParamIDs::bandGain(i));
        ids.push_back(ParamIDs::bandQ(i));
        ids.push_back(ParamIDs::bandCharacter(i));
        ids.push_back(ParamIDs::bandSlope(i));
        ids.push_back(ParamIDs::bandActive(i));
        ids.push_back(ParamIDs::bandSolo(i));
        ids.push_back(ParamIDs::bandDynActive(i));
        ids.push_back(ParamIDs::bandDynDirection(i));
        ids.push_back(ParamIDs::bandDynThreshold(i));
        ids.push_back(ParamIDs::bandDynRatio(i));
        ids.push_back(ParamIDs::bandDynAttack(i));
        ids.push_back(ParamIDs::bandDynRelease(i));
        ids.push_back(ParamIDs::bandDynRange(i));
        ids.push_back(ParamIDs::bandDynSidechain(i));
        ids.push_back(ParamIDs::bandHarmonicBlend(i));
    }
    ids.push_back(ParamIDs::inputGain);
    ids.push_back(ParamIDs::outputGain);
    ids.push_back(ParamIDs::eqActive);
    ids.push_back(ParamIDs::compActive);
    ids.push_back(ParamIDs::compThreshold);
    ids.push_back(ParamIDs::compRatio);
    ids.push_back(ParamIDs::compAttack);
    ids.push_back(ParamIDs::compRelease);
    ids.push_back(ParamIDs::compKnee);
    ids.push_back(ParamIDs::compMakeup);
    ids.push_back(ParamIDs::compAutoMakeup);
    ids.push_back(ParamIDs::compDetector);
    return ids;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto freqRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.01f, 0.25f);
    auto gainRange = juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f);
    auto qRange    = juce::NormalisableRange<float>(0.1f, 18.0f, 0.001f, 0.4f);

    // spread default band frequencies musically across the spectrum
    static const float defaultFreqs[numBands] = { 60.0f, 150.0f, 400.0f, 1000.0f, 2500.0f, 5000.0f, 9000.0f, 15000.0f };
    static const FilterType defaultTypes[numBands] =
    {
        FilterType::HighPass, FilterType::Bell, FilterType::Bell, FilterType::Bell,
        FilterType::Bell, FilterType::Bell, FilterType::Bell, FilterType::LowPass
    };

    for (int i = 0; i < numBands; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::bandType(i), 1), "Band " + juce::String(i + 1) + " Type",
            filterTypeNames(), (int) defaultTypes[i]));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandFreq(i), 1), "Band " + juce::String(i + 1) + " Freq",
            freqRange, defaultFreqs[i],
            juce::AudioParameterFloatAttributes().withLabel("Hz")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandGain(i), 1), "Band " + juce::String(i + 1) + " Gain",
            gainRange, 0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandQ(i), 1), "Band " + juce::String(i + 1) + " Q",
            qRange, 0.707f, juce::AudioParameterFloatAttributes()));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::bandCharacter(i), 1), "Band " + juce::String(i + 1) + " Character",
            filterCharacterNames(), 0));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::bandSlope(i), 1), "Band " + juce::String(i + 1) + " Slope",
            filterSlopeNames(), 1));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(ParamIDs::bandActive(i), 1), "Band " + juce::String(i + 1) + " Active", true));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(ParamIDs::bandSolo(i), 1), "Band " + juce::String(i + 1) + " Solo", false));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(ParamIDs::bandDynActive(i), 1), "Band " + juce::String(i + 1) + " Dyn Active", false));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(ParamIDs::bandDynDirection(i), 1), "Band " + juce::String(i + 1) + " Dyn Direction",
            dynamicDirectionNames(), 0));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandDynThreshold(i), 1), "Band " + juce::String(i + 1) + " Dyn Threshold",
            juce::NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -24.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandDynRatio(i), 1), "Band " + juce::String(i + 1) + " Dyn Ratio",
            juce::NormalisableRange<float>(1.0f, 20.0f, 0.01f, 0.5f), 2.0f,
            juce::AudioParameterFloatAttributes().withLabel(":1")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandDynAttack(i), 1), "Band " + juce::String(i + 1) + " Dyn Attack",
            juce::NormalisableRange<float>(0.1f, 200.0f, 0.01f, 0.3f), 10.0f,
            juce::AudioParameterFloatAttributes().withLabel("ms")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandDynRelease(i), 1), "Band " + juce::String(i + 1) + " Dyn Release",
            juce::NormalisableRange<float>(5.0f, 1000.0f, 0.01f, 0.3f), 100.0f,
            juce::AudioParameterFloatAttributes().withLabel("ms")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandDynRange(i), 1), "Band " + juce::String(i + 1) + " Dyn Range",
            juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 12.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(ParamIDs::bandDynSidechain(i), 1), "Band " + juce::String(i + 1) + " Dyn Sidechain", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ParamIDs::bandHarmonicBlend(i), 1), "Band " + juce::String(i + 1) + " Harmonic Blend",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f,
            juce::AudioParameterFloatAttributes()));
    }

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::inputGain, 1), "Input Gain", gainRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::outputGain, 1), "Output Gain", gainRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::eqActive, 1), "EQ Active", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::compActive, 1), "Compressor Active", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compThreshold, 1), "Comp Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.01f), -18.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compRatio, 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.01f, 0.5f), 2.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compAttack, 1), "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 200.0f, 0.01f, 0.3f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compRelease, 1), "Comp Release",
        juce::NormalisableRange<float>(5.0f, 1000.0f, 0.01f, 0.3f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compKnee, 1), "Comp Knee",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 6.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(ParamIDs::compMakeup, 1), "Comp Makeup",
        gainRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(ParamIDs::compAutoMakeup, 1), "Comp Auto Makeup", true));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(ParamIDs::compDetector, 1), "Comp Detector", detectorNames(), 1));

    return { params.begin(), params.end() };
}

} // namespace ZeroEQ
