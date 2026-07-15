#pragma once

#include <JuceHeader.h>

namespace ZeroEQ
{

// Per-channel harmonic saturation stage for the "Harmonic" band character. Blends
// between an even-harmonic generator (asymmetric quadratic waveshaper, tube-like
// warmth, followed by a DC blocker since asymmetric shaping introduces a DC offset)
// and an odd-harmonic generator (tanh, transistor-like grit), driven by how much
// gain the band is applying.
//
// Uses first-order antiderivative anti-aliasing (ADAA - the trapezoidal rule applied
// to each waveshaper's antiderivative) instead of oversampling, so it needs only the
// previous sample - no lookahead, no added latency, matching every other stage in
// this plugin. See Parker/Zavalishin/Bilbao "Antiderivative Antialiasing for
// Memoryless Nonlinearities" for the general technique; the shaping functions here
// (asymmetric quadratic, tanh) are a straightforward original choice for this
// project, not a copy of any specific commercial product's exact curve.
class HarmonicShaper
{
public:
    void reset();

    // driveAmount: 0 = bypass (dry), up to 1 = maximum drive into the nonlinearity.
    // blend: 0 = pure even harmonics, 1 = pure odd harmonics.
    float processSample(float x, float driveAmount, float blend);

private:
    static constexpr float evenCurve = 0.5f; // quadratic coefficient for the even-harmonic shaper
    static constexpr float driveScale = 4.0f; // driveAmount=1 -> input driven up to 5x
    static constexpr float dcBlockR = 0.9995f; // one-pole DC blocker coefficient

    static float evenAntiderivative(float x);
    static float oddAntiderivative(float x);
    static float evenFunction(float x);
    static float oddFunction(float x);
    static float adaaStep(float x, float xPrev, float (*F)(float), float (*f)(float));

    float xPrevDriven = 0.0f;

    float dcBlockerXPrev = 0.0f;
    float dcBlockerYPrev = 0.0f;
};

} // namespace ZeroEQ
