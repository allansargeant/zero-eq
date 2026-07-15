#include "EQCurveComponent.h"
#include "ZeroEQLookAndFeel.h"

namespace ZeroEQ
{

namespace
{
    const juce::Colour bandPalette[numBands] =
    {
        juce::Colour(0xffff6b6b), juce::Colour(0xffffb454), juce::Colour(0xffffe66d),
        juce::Colour(0xff5ee0a0), juce::Colour(0xff4ecdc4), juce::Colour(0xff5b9dff),
        juce::Colour(0xffb28dff), juce::Colour(0xffff8dc7)
    };
}

EQCurveComponent::EQCurveComponent(ZeroEQAudioProcessor& processorToUse)
    : audioProcessor(processorToUse)
{
    setWantsKeyboardFocus(false);
    startTimerHz(30);
}

EQCurveComponent::~EQCurveComponent()
{
    stopTimer();
}

void EQCurveComponent::setSelectedBand(int band)
{
    selectedBand = band;
    repaint();
}

bool EQCurveComponent::typeHasGain(FilterType type)
{
    return type == FilterType::Bell || type == FilterType::LowShelf
        || type == FilterType::HighShelf || type == FilterType::TiltShelf;
}

juce::Colour EQCurveComponent::colourForBand(int index) const
{
    if (index < 0 || index >= numBands)
        return juce::Colours::grey;
    return bandPalette[(size_t) index];
}

float EQCurveComponent::freqToX(float freqHz) const
{
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float logF = std::log10(juce::jlimit(minFreq, maxFreq, freqHz));
    return getWidth() * (logF - logMin) / (logMax - logMin);
}

float EQCurveComponent::xToFreq(float x) const
{
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const float proportion = juce::jlimit(0.0f, 1.0f, x / (float) juce::jmax(1, getWidth()));
    return std::pow(10.0f, logMin + proportion * (logMax - logMin));
}

float EQCurveComponent::dbToY(float db) const
{
    const float proportion = (db - minDb) / (maxDb - minDb);
    return getHeight() * (1.0f - proportion);
}

float EQCurveComponent::yToDb(float y) const
{
    const float proportion = 1.0f - juce::jlimit(0.0f, 1.0f, y / (float) juce::jmax(1, getHeight()));
    return minDb + proportion * (maxDb - minDb);
}

int EQCurveComponent::findNodeAt(juce::Point<float> pos) const
{
    auto snapshots = EQEngine::readAllSnapshots(audioProcessor.apvts);
    int best = -1;
    float bestDist = nodeRadius * 2.2f;

    for (int i = 0; i < numBands; ++i)
    {
        const auto& s = snapshots[(size_t) i];
        const float nx = freqToX(s.freqHz);
        const float ny = typeHasGain(s.type) ? dbToY(s.gainDb) : dbToY(0.0f);
        const float dist = pos.getDistanceFrom({ nx, ny });
        if (dist < bestDist)
        {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

void EQCurveComponent::timerCallback()
{
    audioProcessor.getPreSpectrum().computeSpectrum(preSpectrumDb);
    audioProcessor.getPostSpectrum().computeSpectrum(postSpectrumDb);
    repaint();
}

void EQCurveComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(ZeroEQLookAndFeel::backgroundPanel);

    // --- frequency grid ---
    g.setColour(ZeroEQLookAndFeel::gridLine);
    const float gridFreqs[] = { 20, 30, 50, 100, 200, 300, 500, 1000, 2000, 3000, 5000, 10000, 20000 };
    for (float f : gridFreqs)
    {
        const float x = freqToX(f);
        g.drawVerticalLine((int) x, 0.0f, bounds.getHeight());
    }

    const float gridDbs[] = { -24, -18, -12, -6, 0, 6, 12, 18, 24 };
    for (float db : gridDbs)
    {
        const float y = dbToY(db);
        g.setColour(juce::approximatelyEqual(db, 0.0f) ? ZeroEQLookAndFeel::textDim.withAlpha(0.5f) : ZeroEQLookAndFeel::gridLine);
        g.drawHorizontalLine((int) y, 0.0f, bounds.getWidth());
    }

    // --- spectrum analyzer overlay (pre = dim fill, post = bright line) ---
    const double sr = audioProcessor.getPreSpectrum().getSampleRate();
    const float binWidth = (float) (sr / SpectrumAnalyzer::fftSize);

    auto buildSpectrumPath = [&](const std::array<float, SpectrumAnalyzer::scopeSize>& data, bool closePath) -> juce::Path
    {
        juce::Path p;
        bool started = false;
        for (int bin = 1; bin < SpectrumAnalyzer::scopeSize; ++bin)
        {
            const float freq = bin * binWidth;
            if (freq < minFreq || freq > maxFreq)
                continue;

            const float x = freqToX(freq);
            const float dbNorm = juce::jmap(data[(size_t) bin], -90.0f, 6.0f, minDb, maxDb);
            const float y = dbToY(juce::jlimit(minDb, maxDb, dbNorm));

            if (! started)
            {
                p.startNewSubPath(x, y);
                started = true;
            }
            else
            {
                p.lineTo(x, y);
            }
        }
        if (closePath && started)
        {
            p.lineTo(bounds.getWidth(), bounds.getHeight());
            p.lineTo(0.0f, bounds.getHeight());
            p.closeSubPath();
        }
        return p;
    };

    auto prePath = buildSpectrumPath(preSpectrumDb, true);
    g.setColour(ZeroEQLookAndFeel::spectrumPre);
    g.fillPath(prePath);

    auto postPath = buildSpectrumPath(postSpectrumDb, false);
    g.setColour(ZeroEQLookAndFeel::spectrumPost);
    g.strokePath(postPath, juce::PathStrokeType(1.4f));

    // --- composite EQ curve ---
    auto snapshots = EQEngine::readAllSnapshots(audioProcessor.apvts);
    juce::Path curve;
    const int steps = juce::jmax(2, getWidth());
    for (int i = 0; i <= steps; ++i)
    {
        const float x = (float) i / (float) steps * bounds.getWidth();
        const float freq = xToFreq(x);
        const float magnitude = EQEngine::getCompositeMagnitude(snapshots, freq, sr > 0 ? sr : 44100.0);
        const float db = juce::Decibels::gainToDecibels(magnitude, -60.0f);
        const float y = dbToY(juce::jlimit(minDb, maxDb, db));

        if (i == 0)
            curve.startNewSubPath(x, y);
        else
            curve.lineTo(x, y);
    }

    juce::Path fillPath = curve;
    fillPath.lineTo(bounds.getWidth(), dbToY(0.0f));
    fillPath.lineTo(0.0f, dbToY(0.0f));
    fillPath.closeSubPath();
    g.setColour(ZeroEQLookAndFeel::curveGreen.withAlpha(0.12f));
    g.fillPath(fillPath);

    g.setColour(ZeroEQLookAndFeel::curveGreen);
    g.strokePath(curve, juce::PathStrokeType(2.2f));

    // --- per-band nodes ---
    for (int i = 0; i < numBands; ++i)
    {
        const auto& s = snapshots[(size_t) i];
        const float nx = freqToX(s.freqHz);
        const float ny = typeHasGain(s.type) ? dbToY(s.gainDb) : dbToY(0.0f);

        const bool isSelected = (i == selectedBand);
        const bool isHovered = (i == hoveredBand);
        const float radius = nodeRadius * (isSelected ? 1.25f : (isHovered ? 1.1f : 1.0f));

        auto colour = colourForBand(i);
        if (! s.active)
            colour = colour.withAlpha(0.35f);

        const bool dynEngaged = s.dynActive && s.active && typeHasGain(s.type);
        if (dynEngaged)
        {
            // Ring marks the band as dynamic; the live tick shows this instant's
            // actual modulated gain on top of the static curve position.
            g.setColour(ZeroEQLookAndFeel::accentOrange.withAlpha(0.8f));
            g.drawEllipse(nx - radius - 4.0f, ny - radius - 4.0f, (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f, 1.5f);

            const float liveDelta = audioProcessor.getEQEngine().getBandDynamicGainDeltaDb(i);
            if (std::abs(liveDelta) > 0.05f)
            {
                const float liveY = dbToY(juce::jlimit(minDb, maxDb, s.gainDb + liveDelta));
                g.setColour(ZeroEQLookAndFeel::accentOrange);
                g.drawLine(nx, ny, nx, liveY, 2.5f);
                g.fillEllipse(nx - 3.0f, liveY - 3.0f, 6.0f, 6.0f);
            }
        }

        if (isSelected)
        {
            g.setColour(colour.withAlpha(0.3f));
            g.fillEllipse(nx - radius - 5.0f, ny - radius - 5.0f, (radius + 5.0f) * 2.0f, (radius + 5.0f) * 2.0f);
        }

        g.setColour(colour);
        g.fillEllipse(nx - radius, ny - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(ZeroEQLookAndFeel::backgroundDark);
        g.drawEllipse(nx - radius, ny - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        g.setColour(ZeroEQLookAndFeel::backgroundDark);
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(juce::String(i + 1), juce::Rectangle<float>(nx - radius, ny - radius, radius * 2.0f, radius * 2.0f),
                   juce::Justification::centred);
    }

    g.setColour(ZeroEQLookAndFeel::gridLine);
    g.drawRect(bounds, 1.0f);
}

void EQCurveComponent::resized()
{
}

void EQCurveComponent::mouseDown(const juce::MouseEvent& e)
{
    const int idx = findNodeAt(e.position);
    dragStartMouse = e.position;

    if (idx >= 0)
    {
        selectedBand = idx;
        draggingBand = idx;
        if (onBandSelected)
            onBandSelected(idx);

        auto snapshot = EQEngine::readSnapshot(audioProcessor.apvts, idx);
        dragStartQ = snapshot.q;
        draggingQ = ! typeHasGain(snapshot.type);

        audioProcessor.apvts.getParameter(ParamIDs::bandFreq(idx))->beginChangeGesture();
        if (! draggingQ)
            audioProcessor.apvts.getParameter(ParamIDs::bandGain(idx))->beginChangeGesture();
        else
            audioProcessor.apvts.getParameter(ParamIDs::bandQ(idx))->beginChangeGesture();
    }
    else
    {
        draggingBand = -1;
    }
    repaint();
}

void EQCurveComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingBand < 0)
        return;

    const int idx = draggingBand;
    auto* freqParam = audioProcessor.apvts.getParameter(ParamIDs::bandFreq(idx));

    const float freq = juce::jlimit(minFreq, maxFreq, xToFreq(e.position.x));
    freqParam->setValueNotifyingHost(freqParam->getNormalisableRange().convertTo0to1(freq));

    if (! draggingQ)
    {
        auto* gainParam = audioProcessor.apvts.getParameter(ParamIDs::bandGain(idx));
        const float gainDb = juce::jlimit(minDb, maxDb, yToDb(e.position.y));
        gainParam->setValueNotifyingHost(gainParam->getNormalisableRange().convertTo0to1(gainDb));
    }
    else
    {
        auto* qParam = audioProcessor.apvts.getParameter(ParamIDs::bandQ(idx));
        const float dy = dragStartMouse.y - e.position.y;
        const float newQ = juce::jlimit(0.1f, 18.0f, dragStartQ * std::pow(2.0f, dy / 150.0f));
        qParam->setValueNotifyingHost(qParam->getNormalisableRange().convertTo0to1(newQ));
    }
}

void EQCurveComponent::mouseUp(const juce::MouseEvent&)
{
    if (draggingBand >= 0)
    {
        const int idx = draggingBand;
        audioProcessor.apvts.getParameter(ParamIDs::bandFreq(idx))->endChangeGesture();
        if (! draggingQ)
            audioProcessor.apvts.getParameter(ParamIDs::bandGain(idx))->endChangeGesture();
        else
            audioProcessor.apvts.getParameter(ParamIDs::bandQ(idx))->endChangeGesture();
    }
    draggingBand = -1;
}

void EQCurveComponent::mouseMove(const juce::MouseEvent& e)
{
    const int idx = findNodeAt(e.position);
    if (idx != hoveredBand)
    {
        hoveredBand = idx;
        setMouseCursor(idx >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void EQCurveComponent::mouseExit(const juce::MouseEvent&)
{
    hoveredBand = -1;
    repaint();
}

void EQCurveComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    const int idx = findNodeAt(e.position);
    if (idx < 0)
        return;

    auto* qParam = audioProcessor.apvts.getParameter(ParamIDs::bandQ(idx));
    auto snapshot = EQEngine::readSnapshot(audioProcessor.apvts, idx);
    const float newQ = juce::jlimit(0.1f, 18.0f, snapshot.q * std::pow(2.0f, wheel.deltaY * 3.0f));

    qParam->beginChangeGesture();
    qParam->setValueNotifyingHost(qParam->getNormalisableRange().convertTo0to1(newQ));
    qParam->endChangeGesture();
}

void EQCurveComponent::mouseDoubleClick(const juce::MouseEvent& e)
{
    const int idx = findNodeAt(e.position);
    if (idx >= 0)
    {
        auto* activeParam = audioProcessor.apvts.getParameter(ParamIDs::bandActive(idx));
        const bool currentlyActive = activeParam->getValue() > 0.5f;
        activeParam->beginChangeGesture();
        activeParam->setValueNotifyingHost(currentlyActive ? 0.0f : 1.0f);
        activeParam->endChangeGesture();
        repaint();
        return;
    }

    // empty space: activate the next inactive band at this frequency/gain
    auto snapshots = EQEngine::readAllSnapshots(audioProcessor.apvts);
    for (int i = 0; i < numBands; ++i)
    {
        if (! snapshots[(size_t) i].active)
        {
            const float freq = xToFreq(e.position.x);
            const float gainDb = yToDb(e.position.y);

            auto* freqParam = audioProcessor.apvts.getParameter(ParamIDs::bandFreq(i));
            auto* gainParam = audioProcessor.apvts.getParameter(ParamIDs::bandGain(i));
            auto* activeParam = audioProcessor.apvts.getParameter(ParamIDs::bandActive(i));

            freqParam->setValueNotifyingHost(freqParam->getNormalisableRange().convertTo0to1(juce::jlimit(minFreq, maxFreq, freq)));
            gainParam->setValueNotifyingHost(gainParam->getNormalisableRange().convertTo0to1(juce::jlimit(minDb, maxDb, gainDb)));
            activeParam->setValueNotifyingHost(1.0f);

            selectedBand = i;
            if (onBandSelected)
                onBandSelected(i);
            repaint();
            break;
        }
    }
}

} // namespace ZeroEQ
