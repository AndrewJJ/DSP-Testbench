/*
  ==============================================================================

    AnalyserComponent.cpp
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "AnalyserComponent.h"

FftScope::Background::Background (FftScope* parentFftScope)
    :   parentScope (parentFftScope)
{
    setBufferedToImage (true);
}
void FftScope::Background::paint (Graphics& g)
{
    parentScope->paintFftScale (g);
}

FftScope::Foreground::Foreground (FftScope* parentFftScope)
    :   parentScope (parentFftScope)
{ }
void FftScope::Foreground::paint (Graphics& g)
{
    // TODO - implement painting on another thread to avoid choking the message thread?
    parentScope->paintFft (g);
}

FftScope::FftScope ()
    :   background (this),
        foreground (this),
        fftProcessor (nullptr)
{
    x.allocate(1 << 12, true);
    y.allocate(1 << 12, true);
    this->setOpaque (true);
    this->setPaintingIsUnclipped (true);

    addAndMakeVisible (background);
    addAndMakeVisible (foreground);

    addMouseListener (this, true);
    foreground.setMouseCursor (MouseCursor::CrosshairCursor);
}
FftScope::~FftScope ()
{
    if (fftProcessor != nullptr)
        fftProcessor->removeListener (this);
}
void FftScope::paint (Graphics&)
{ }
void FftScope::resized ()
{
    initialise();
    background.setBounds (getLocalBounds());
    foreground.setBounds (getLocalBounds());
}
void FftScope::mouseMove (const MouseEvent& event)
{
    currentX = event.x;
    currentY = event.y;
}
void FftScope::mouseExit (const MouseEvent&)
{
    // Set to -1 to indicate out of bounds
    currentX = -1;
    currentY = -1;
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
    initialise();
}
void FftScope::paintFft (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level

    const auto n = fftProcessor->getCurrentBlockSize() / 2;
    const auto amplitudeCorrection = 2.0f / static_cast<float> (n); // For Hann window
    const auto bottomY = static_cast<float> (getHeight() - 1);

    for (auto ch = 0; ch < fftProcessor->getNumChannels(); ++ch)
    {
        // Draw a line representing the freq data for this channel
        Path p;
        p.preallocateSpace ((n + 1) * 3);
        fftProcessor->copyFrequencyData (y, ch);
        FloatVectorOperations::multiply (y, amplitudeCorrection, fftProcessor->getCurrentBlockSize());
        p.startNewSubPath (0, convertToPxFromLinear (y[0]));
        for (auto i = 1; i <= n; ++i)
            p.lineTo (x[i], convertToPxFromLinear (y[i]));
        p.lineTo (static_cast<float> (getWidth()), static_cast<float> (getHeight()));
        const auto pst = PathStrokeType (1.0f);
        g.setColour (getColourForChannel (ch));
        g.strokePath(p, pst);
    }

    // Output mouse co-ordinates in Hz/dB
    if (currentX >= 0 && currentY >= 0)
    {
        g.setColour (Colours::white);
        g.setFont (Font (GUI_SIZE_F(0.5)));
        const auto freq = convertToHzFromPx (static_cast<float> (currentX));
        const auto freqStr = hertzToString (freq, 2, true, true);
        const auto dbStr = String (convertToDbVFromPx (static_cast<float> (currentY)), 1);
        const auto txt =  freqStr + ", " + dbStr + " dB";
        const auto offset = GUI_GAP_I(2);
        auto lblX = currentX + offset;
        auto lblY = currentY + offset;
        const auto lblW = GUI_SIZE_I(4.1);
        const auto lblH = GUI_SIZE_I(0.6);
        auto lblJust = Justification::centredLeft;
        if (lblX + lblW > getWidth())
        {
            lblX = currentX - offset - lblW;
            lblJust = Justification::centredRight;
        }
        if (lblY + lblH > getHeight())
            lblY = currentY - offset - lblH;
        g.drawText (txt, lblX, lblY, lblW, lblH, lblJust);
    }
}
void FftScope::paintFftScale (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level

    g.setColour (Colours::black);
    g.fillRect (getLocalBounds());

    const auto axisColour = Colours::darkgrey.darker();
    const auto textColour = Colours::grey.darker();

    g.setColour (axisColour);
    g.drawRect (getLocalBounds().toFloat());

    g.setFont (Font (GUI_SIZE_I(0.4)));

    // Plot dB scale (just halves, quarters or eighths)
    const auto maxTicks = getHeight() / GUI_SIZE_I(2);
    auto numTicks = 0;
    if (maxTicks >= 8)
        numTicks = 8;
    else if (maxTicks >= 4)
        numTicks = 4;
    else if (maxTicks >= 2)
        numTicks = 2;
    for (auto t = 0; t < numTicks; ++t)
    {
        const auto scaleY = static_cast<float> (getHeight()) / static_cast<float> (numTicks) * static_cast<float> (t);
        g.setColour (axisColour);
        if (t > 0)
            g.drawHorizontalLine (static_cast<int> (scaleY), 0.0f, static_cast<float> (getWidth()));
        g.setColour (textColour);
        const auto dB = String (static_cast<int> (convertToDbVFromPx (scaleY)));
        const auto lblX = GUI_SIZE_I(0.1);
        const auto lblY = static_cast<int> (scaleY) + GUI_SIZE_I(0.1);
        const auto lblW = GUI_SIZE_I(1.1);
        const auto lblH = static_cast<int> (scaleY) + GUI_SIZE_I(0.6);
        g.drawFittedText (dB, lblX, lblY, lblW, lblH, Justification::topLeft, 1, 1.0f);
    }
   
    // Plot frequency scale
    auto nextThreshX = GUI_BASE_SIZE_I;
    const auto h = static_cast<float> (getHeight());
    const auto ty = getHeight() - GUI_SIZE_I(0.6);
    // Assume there is room to show minFreq
    g.drawFittedText (hertzToString (minFreq, 0, false, false), GUI_SIZE_I(0.1), ty, GUI_BASE_SIZE_I, GUI_SIZE_I(0.5), Justification::topLeft, 1, 1.0f);
	for (auto f : gridFrequencies)
	{
		if (f >= minFreq && f <= maxFreq)
		{
			const auto scaleX = static_cast<int> (convertToPxFromHz (f));
			// Only draw if we have enough separation
			if (scaleX >= nextThreshX)
			{
                g.setColour (axisColour);
			    g.drawVerticalLine (scaleX, 0.0f, h);
                g.setColour (textColour);
                g.drawFittedText (hertzToString (f, 0, false, false), scaleX + GUI_SIZE_I(0.1), ty, GUI_BASE_SIZE_I, GUI_SIZE_I(0.5), Justification::topLeft, 1, 1.0f);
                nextThreshX += GUI_BASE_SIZE_I;
			}
		}
	}
}
float FftScope::convertToDbVFromLinear (const float linear) const
{
    if (linear <= 0.0f)
        return dBmin;
    else
        //return log10(x) * 20.0f;
        return jmax (dBmin, fasterlog2 (linear) * 6.0206f);
}
float FftScope::convertToPxFromLinear (const float linear) const
{
    //return jmax (1.0f, (convertTodBV (linear) - dBmax) * yRatio) - 1.0f;
    return convertToPxFromDbV (convertToDbVFromLinear (linear));
}
inline float FftScope::convertToPxFromDbV(const float dB) const
{
    return jmax (1.0f, (dB - dBmax) * yRatio) - 1.0f;
}
float FftScope::convertToDbVFromPx (const float yInPixels) const
{
    return (yInPixels + 1.0f) / yRatio  + dBmax;
}
float FftScope::convertToHzFromPx (const float xInPixels) const
{
    //return powf(10.0f, xInPixels / xRatio + minLogFreq);
    return fastpow10 (xInPixels / xRatio + minLogFreq);
}
float FftScope::convertToPxFromHz (const float xInHz) const
{
    // Only used occasionally so don't need performance
    return (log10 (xInHz) - minLogFreq) * xRatio;
}
String FftScope::hertzToString (const double frequencyInHz, const int numDecimals, const bool appendHz, const bool includeSpace) const
{
    String space(includeSpace ? " " : "");
    String units("");
    String frequency;

    if (frequencyInHz < 1000.0)
    {
        frequency = String (roundToInt<double>(frequencyInHz));
        units = appendHz ? "Hz" : "";
    }
    else if (frequencyInHz < 1000000.0)
    {
        frequency = String (frequencyInHz * 0.001, numDecimals);
        units = appendHz ? "kHz" : "K";
    }
    else
    {
        frequency = String (frequencyInHz * .000001, numDecimals);
        units = appendHz ? "MHz" : "M";
    }
    if (units == "") space = "";
    return frequency + space + units;
}
Colour FftScope::getColourForChannel (const int channel)
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
void FftScope::initialise()
{
    const auto nyquist = static_cast<float> (samplingFreq * 0.5);
    minFreq = 10.0f;
    maxFreq = nyquist;
    minLogFreq = log10 (minFreq);
    logFreqSpan = log10 (maxFreq) - minLogFreq;
    const auto n = fftProcessor->getCurrentBlockSize() / 2;
    xRatio = static_cast<float> (getWidth()) / logFreqSpan;
    const auto binToHz = nyquist / static_cast<float> (n);

    for (auto i = 1; i <= n; ++i)
        x[i] = convertToPxFromHz (static_cast<float> (i) * binToHz);

    yRatio = static_cast<float> (getHeight()) / (dBmin - dBmax);
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
    grid.rowGap = GUI_GAP_PX(2);
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
