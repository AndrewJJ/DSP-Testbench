/*
  ==============================================================================

    AnalyserComponent.cpp
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "AnalyserComponent.h"

FftScope::FftScope (): fftProcessor (nullptr)
{
    this->setOpaque (true);
}
FftScope::~FftScope ()
{
    if (fftProcessor != nullptr)
        fftProcessor->removeListener (this);
}
void FftScope::paint (Graphics& g)
{
    // TODO - optimise & make pretty
    // TODO - plot axes and cache to bitmap
    // TODO - implement painting on another thread to avoid choking the message thread?

    g.fillAll(Colours::black);

    const auto nyquist = static_cast<float> (samplingFreq * 0.5);

    const auto strokeWidth = 1.0f;  // TODO - make strokewidth a property, or delete?
    const auto dbMin = -80.0f;      // TODO - make min dB a property
    const auto minFreq = 10.0f;     // TODO - make min frequency a property
    const auto maxFreq = nyquist;   // TODO - make the max frequency a property (limited to nyquist)
    const auto minLogFreq = log10 (minFreq);
    const auto logFreqSpan = log10 (maxFreq) - minLogFreq;
    const auto n = fftProcessor->getCurrentBlockSize() / 2;
    const auto xRatio = static_cast<float> (getWidth()) / logFreqSpan;
    const auto yRatio = static_cast<float> (getHeight()) / dbMin;
    const auto amplitudeCorrection = 1.0f / static_cast<float>(n);
    const auto binToHz = nyquist / static_cast<float> (n);

    for (auto ch = 0; ch < fftProcessor->getNumChannels(); ++ch)
    {
        // Draw a line representing the freq data for this channel
        Path p;
        p.preallocateSpace ((n + 1) * 3);
        const auto offscreenX = -1.0f - strokeWidth;
        const auto offscreenY = static_cast<float> (getHeight()) + strokeWidth;
        fftProcessor->copyFrequencyData (f, ch);
        auto x = offscreenX;
        auto y = (f[0] <= 0.0f) ? offscreenY : todBVoltsFromLinear (f[0] * amplitudeCorrection) * yRatio;
        p.startNewSubPath(x, y);
        for (auto i = 1; i <= n; ++i)
        {
            y = (f[i] <= 0.0f) ? offscreenY : todBVoltsFromLinear (f[i] * amplitudeCorrection) * yRatio;
            x = (log10 (static_cast<float> (i) * binToHz) - minLogFreq) * xRatio;
            p.lineTo (x, y);
        }
        const auto pst = PathStrokeType (strokeWidth);
        // TODO - different colours for different channels
        if (ch == 0)
            g.setColour (Colours::green);
        else
            g.setColour (Colours::yellow);
        g.strokePath(p, pst);
    }
}
void FftScope::assignFftMult (FftProcessor<12>* fftMultPtr)
{
    jassert (fftMultPtr != nullptr);
    fftProcessor = fftMultPtr;
    fftProcessor->addListener (this);
}
void FftScope::audioProbeUpdated (AudioProbe<FftProcessor<12>::FftFrame>*)
{
    repaint();
}
void FftScope::prepare (const dsp::ProcessSpec& spec)
{
    samplingFreq = spec.sampleRate;
}
float FftScope::todBVoltsFromLinear (const float x) const
{
    if (x <= 0.0f)
        return 0.0f;
    else
        return 20.0f * log10(x);
}

AnalyserComponent::AnalyserComponent()
{
    lblTitle.setName ("Analyser label");
    lblTitle.setText ("Analyser", dontSendNotification);
    lblTitle.setFont (Font (15.00f, Font::bold));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    addAndMakeVisible (lblTitle);
    
    btnDisable.setButtonText ("Disable");
    btnDisable.setClickingTogglesState (true);
    btnDisable.setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnDisable.onClick = [this] { statusActive = !btnDisable.getToggleState(); };
    addAndMakeVisible (btnDisable);

    addAndMakeVisible (fftScope);
    fftScope.assignFftMult (&fftMult);
}
AnalyserComponent::~AnalyserComponent() = default;
void AnalyserComponent::paint (Graphics& g)
{
    g.setColour (Colours::black);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), 10.000f);
}
void AnalyserComponent::resized()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr),
                            Track (6_fr)
                        };

    //grid.templateColumns = { Track (1_fr), Track (6_fr), Track (1_fr), Track (1_fr) };
    grid.templateColumns = { Track (9_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (btnDisable).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f)),
                            GridItem (fftScope).withArea ({}, GridItem::Span (2))
                        });

    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void AnalyserComponent::prepare (const dsp::ProcessSpec& spec)
{
    fftMult.prepare (spec);
    fftScope.prepare (spec);
}
void AnalyserComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    auto* inputBlock = &context.getInputBlock();
    for (size_t ch = 0; ch < inputBlock->getNumChannels(); ++ch)
        fftMult.appendData (static_cast<int> (ch), static_cast<int> (inputBlock->getNumSamples()), inputBlock->getChannelPointer (ch));
}
void AnalyserComponent::reset ()
{
}
bool AnalyserComponent::isActive () const
{
    return statusActive;
}
