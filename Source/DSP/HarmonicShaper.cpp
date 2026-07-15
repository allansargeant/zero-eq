#include "HarmonicShaper.h"

namespace ZeroEQ
{

void HarmonicShaper::reset()
{
    xPrevDriven = 0.0f;
    dcBlockerXPrev = 0.0f;
    dcBlockerYPrev = 0.0f;
}

float HarmonicShaper::evenFunction(float x)
{
    return x + evenCurve * x * x;
}

float HarmonicShaper::evenAntiderivative(float x)
{
    return x * x * 0.5f + evenCurve * x * x * x / 3.0f;
}

float HarmonicShaper::oddFunction(float x)
{
    return std::tanh(x);
}

float HarmonicShaper::oddAntiderivative(float x)
{
    return std::log(std::cosh(x));
}

float HarmonicShaper::adaaStep(float x, float xPrev, float (*F)(float), float (*f)(float))
{
    const float dx = x - xPrev;
    if (std::abs(dx) < 1.0e-5f)
        return f(0.5f * (x + xPrev));
    return (F(x) - F(xPrev)) / dx;
}

float HarmonicShaper::processSample(float x, float driveAmount, float blend)
{
    if (driveAmount <= 0.0005f)
    {
        xPrevDriven = x;
        return x;
    }

    const float driveGain = 1.0f + driveAmount * driveScale;
    const float xd = juce::jlimit(-8.0f, 8.0f, x * driveGain);

    const float evenOut = adaaStep(xd, xPrevDriven, evenAntiderivative, evenFunction);
    const float oddOut  = adaaStep(xd, xPrevDriven, oddAntiderivative, oddFunction);
    xPrevDriven = xd;

    const float wet = (1.0f - blend) * evenOut + blend * oddOut;
    const float wetNormalized = wet / driveGain;

    // One-pole DC blocker: the even (asymmetric) shaper introduces a DC offset that
    // needs removing; harmless no-op for the odd (symmetric) component.
    const float dcOut = wetNormalized - dcBlockerXPrev + dcBlockR * dcBlockerYPrev;
    dcBlockerXPrev = wetNormalized;
    dcBlockerYPrev = dcOut;

    return (1.0f - driveAmount) * x + driveAmount * dcOut;
}

} // namespace ZeroEQ
