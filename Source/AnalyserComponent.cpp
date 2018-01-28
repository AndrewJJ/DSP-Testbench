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
    x.allocate(1 << 12, true);
    y.allocate(1 << 12, true);
    this->setOpaque (true);
    this->setPaintingIsUnclipped (true);
}
FftScope::~FftScope ()
{
    if (fftProcessor != nullptr)
        fftProcessor->removeListener (this);
}
void FftScope::paint (Graphics& g)
{
    // TODO - plot axes and cache to bitmap
    // TODO - implement painting on another thread to avoid choking the message thread?
    paintScale (g);
    paintFft (g);
}
void FftScope::resized ()
{
    initialiseX();
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
    initialiseX();
}
void FftScope::paintFft (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level
    const auto n = fftProcessor->getCurrentBlockSize() / 2;
    const auto amplitudeCorrection = 1.0f / static_cast<float>(n);
    const auto yRatio = static_cast<float> (getHeight()) / dBmin;
    const auto bottomY = static_cast<float> (getHeight() - 1);

    for (auto ch = 0; ch < fftProcessor->getNumChannels(); ++ch)
    {
        // Draw a line representing the freq data for this channel
        Path p;
        p.preallocateSpace ((n + 1) * 3);
        fftProcessor->copyFrequencyData (y, ch);
        FloatVectorOperations::multiply (y, amplitudeCorrection, fftProcessor->getCurrentBlockSize());
        auto cy = (y[0] <= 0.0f) ? bottomY : convertTodBV (y[0]) * yRatio;
        p.startNewSubPath (0, cy);
        for (auto i = 1; i <= n; ++i)
        {
            cy = convertTodBV (y[i]) * yRatio;
            p.lineTo (x[i], cy - 1.0f);
        }
        p.lineTo (static_cast<float> (getWidth()), static_cast<float> (getHeight()));
        const auto pst = PathStrokeType (1.0f);
        g.setColour (getColourForChannel (ch));
        g.strokePath(p, pst);
    }
}
void FftScope::paintScale (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level

    g.setColour (Colours::black);
    g.fillRect (getLocalBounds());
    g.setColour (Colours::white.withAlpha (0.2f));
    g.drawRect (getLocalBounds());

    // TODO - draw scale
}
float FftScope::convertTodBV (const float linear) const
{
    if (linear <= 0.0f)
        return dBmin;
    else
        //return log10(x) * 20.0f;
        return jmax (dBmin, fasterlog2 (linear) * 6.0206f);
}
Colour FftScope::getColourForChannel (const int channel) const
{
    switch (channel % 6)
    {
        case 0: return Colours::green;
        case 1: return Colours::yellow;
        case 2: return Colours::blue;
        case 3: return Colours::cyan;
        case 4: return Colours::orange;
        case 5: return Colours::magenta;
        default: return Colours::red;
    }
}
void FftScope::initialiseX()
{
    const auto nyquist = static_cast<float> (samplingFreq * 0.5);
    const auto minFreq = 10.0f;     // TODO - make min frequency a property
    const auto maxFreq = nyquist;   // TODO - make the max frequency a property (limited to nyquist)
    const auto minLogFreq = log10 (minFreq);
    const auto logFreqSpan = log10 (maxFreq) - minLogFreq;
    const auto n = fftProcessor->getCurrentBlockSize() / 2;
    const auto xRatio = static_cast<float> (getWidth()) / logFreqSpan;
    const auto binToHz = nyquist / static_cast<float> (n);

    for (auto i = 1; i <= n; ++i)
        x[i] = (log10 (static_cast<float> (i) * binToHz) - minLogFreq) * xRatio;
}

AnalyserComponent::AnalyserComponent()
{
    lblTitle.setName ("Analyser label");
    lblTitle.setText ("Analyser", dontSendNotification);
    lblTitle.setFont (Font (GUI_SIZE_F(0.7), Font::bold));
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
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void AnalyserComponent::resized()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (GUI_SIZE_PX(3)) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (btnDisable),
                            GridItem (fftScope).withArea ({}, GridItem::Span (2))
                        });

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
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
{ }
bool AnalyserComponent::isActive () const noexcept
{
    return statusActive.get();
}
