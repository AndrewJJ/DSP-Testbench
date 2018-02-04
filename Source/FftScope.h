/*
  ==============================================================================

    FftScope.h
    Created: 29 Jan 2018 10:17:00pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "FftProcessor.h"
#include "FastApproximations.h"

template <int Order>
class FftScope : public Component, public AudioProbe <typename FftProcessor<Order>::FftFrame>::Listener
{
public:

    FftScope ();
    ~FftScope ();

    void paint (Graphics& g) override;
    void resized() override;
    void mouseMove(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;

    void assignFftMult (FftProcessor<Order>* fftMultPtr);
    void audioProbeUpdated (AudioProbe<typename FftProcessor<Order>::FftFrame>* audioProbe) override;
    void prepare (const dsp::ProcessSpec& spec);

    // Set minimum dB value for y-axis (defaults to -80dB otherwise)
    void setDbMin (float minimumDb);
    float getDbMin() const;

    // Set maximum dB value for y-axis (defaults to 0dB otherwise)
    void setDbMax (float maximumDb);
    float getDbMax() const;

    // Set minimum frequency for x-axis (defaults to 10Hz otherwise)
    void setFreqMin (float minimumFreq);
    float getFreqMin() const;

    // Set maximum frequency value for x-axis (defaults to Nyquist otherwise)
    // Will be limited to Nyquist if set too high
    void setFreqMax (float maximumFreq);
    float getFreqMax() const;

private:
    
    class Background : public Component
    {
    public:
        Background (FftScope* parentFftScope);
        void paint (Graphics& g) override;
    private:
        FftScope* parentScope;
    };

    class Foreground : public Component
    {
    public:
        explicit Foreground (FftScope<Order>* parentFftScope);
        void paint (Graphics& g) override;
    private:
        FftScope* parentScope;
    };

    void paintFft (Graphics& g) const;
    void paintFftScale (Graphics& g) const;

    inline float toDbVFromLinear (const float linear) const;
    inline float toPxFromLinear (const float linear) const;
    inline float toPxFromDbV (const float dB) const;
    inline float toDbVFromPx (const float yInPixels) const;
    inline float toHzFromPx (const float xInPixels) const;
    inline float toPxFromHz (const float xInHz) const;

    String hertzToString (const double frequencyInHz, const int numDecimals, const bool appendHz, const bool includeSpace) const;
    static Colour getColourForChannel (const int channel);
    void initialise();

    Background background;
    Foreground foreground;
	FftProcessor<Order>* fftProcessor;
    HeapBlock<float> x, y;
	double samplingFreq = 48000; // will be set correctly in prepare()
    float dBmax = 0.0f;
    float dBmin = -80.0f;
    float minFreq = 10.0f;
    float maxFreq = 0.0f;
    float minLogFreq = 0.0f;
    float logFreqSpan = 0.0f;
    float xRatio = 1.0f;
    float yRatio = 1.0f;
    int currentX = -1;
    int currentY = -1;
    
    // Candidate frequencies for drawing the grid on the background
    Array<float> gridFrequencies = { 20.0f, 50.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f, 32000.0f, 64000.0f };
};


// ===========================================================================================
// Template implementations
// ===========================================================================================

template <int Order>
FftScope<Order>::Background::Background (FftScope* parentFftScope)
    :   parentScope (parentFftScope)
{
    setBufferedToImage (true);
}

template <int Order>
void FftScope<Order>::Background::paint (Graphics& g)
{
    parentScope->paintFftScale (g);
}

template <int Order>
FftScope<Order>::Foreground::Foreground (FftScope* parentFftScope)
    :   parentScope (parentFftScope)
{ }

template <int Order>
void FftScope<Order>::Foreground::paint (Graphics& g)
{
    parentScope->paintFft (g);
}

template <int Order>
FftScope<Order>::FftScope ()
    :   background (this),
        foreground (this),
        fftProcessor (nullptr)
{
    this->setOpaque (true);
    this->setPaintingIsUnclipped (true);

    addAndMakeVisible (background);
    addAndMakeVisible (foreground);

    addMouseListener (this, true);
    foreground.setMouseCursor (MouseCursor::CrosshairCursor);
}

template <int Order>
FftScope<Order>::~FftScope ()
{
    if (fftProcessor != nullptr)
        fftProcessor->removeListener (this);
}

template <int Order>
void FftScope<Order>::paint (Graphics&)
{ }

template <int Order>
void FftScope<Order>::resized ()
{
    initialise();
    background.setBounds (getLocalBounds());
    foreground.setBounds (getLocalBounds());
}

template <int Order>
void FftScope<Order>::mouseMove (const MouseEvent& event)
{
    currentX = event.x;
    currentY = event.y;
}

template <int Order>
void FftScope<Order>::mouseExit (const MouseEvent&)
{
    // Set to -1 to indicate out of bounds
    currentX = -1;
    currentY = -1;
}

template <int Order>
void FftScope<Order>::assignFftMult (FftProcessor<Order>* fftMultPtr)
{
    jassert (fftMultPtr != nullptr);
    fftProcessor = fftMultPtr;
    fftProcessor->addListener (this);
    x.allocate (fftProcessor->getMaximumBlockSize(), true);
    y.allocate (fftProcessor->getMaximumBlockSize(), true);

}

template <int Order>
void FftScope<Order>::audioProbeUpdated (AudioProbe<typename FftProcessor<Order>::FftFrame>*)
{
    repaint();
}

template <int Order>
void FftScope<Order>::prepare (const dsp::ProcessSpec& spec)
{
    samplingFreq = spec.sampleRate;
    initialise();
}

template <int Order>
void FftScope<Order>::setDbMin (float minimumDb)
{
    dBmin = minimumDb;
}

template <int Order>
float FftScope<Order>::getDbMin () const
{
    return dBmin;
}

template <int Order>
void FftScope<Order>::setDbMax (float maximumDb)
{
    dBmax = maximumDb;
}

template <int Order>
float FftScope<Order>::getDbMax () const
{
    return dBmax;
}

template <int Order>
void FftScope<Order>::setFreqMin (float minimumFreq)
{
    minFreq = minimumFreq;
}

template <int Order>
float FftScope<Order>::getFreqMin () const
{
    return minFreq;
}

template <int Order>
void FftScope<Order>::setFreqMax (float maximumFreq)
{
    maxFreq = maximumFreq;
}

template <int Order>
float FftScope<Order>::getFreqMax () const
{
    return maxFreq;
}

template <int Order>
void FftScope<Order>::paintFft (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level

    const auto n = fftProcessor->getMaximumBlockSize() / 2;
    const auto amplitudeCorrection = 2.0f / static_cast<float> (n); // For Hann window
    const auto bottomY = static_cast<float> (getHeight() - 1);

    for (auto ch = 0; ch < fftProcessor->getNumChannels(); ++ch)
    {
        // Copy frequency data and scale
        fftProcessor->copyFrequencyData (y, ch);

        // Draw a line representing the freq data for this channel
        Path p;
        
        // Deprecated code for plotting each point on path (including subpixel resolution)
        //p.preallocateSpace ((n + 1) * 3);
        //p.startNewSubPath (0, toPxFromLinear (y[0]));
        //for (auto i = 1; i <= n; ++i)
        //{
        //    p.lineTo (x[i], toPxFromLinear (y[i]));
        //}
        //p.lineTo (static_cast<float> (getWidth()), static_cast<float> (getHeight()));

        p.preallocateSpace ((getWidth() + 1) * 3); // Will generally be a lot less than this for log frequency scale
        
        // Find first positive x value (important if minFreq is set higher than default
        auto i = 0;
        while (x[i]<0)
            ++i;
        p.startNewSubPath (x[i], toPxFromLinear (y[i]));
        ++i;
        
        // Iterate through x and plot each point, but use max y if x interval is less than a pixel
        auto xPx = static_cast<int> (x[i]); // x co-ordinate in pixels
        while (xPx < getWidth() && i <= n)
        {
            const auto xPxNext = xPx + 1; // next pixel along on x-axis
            auto yMax = y[i];
            while (i < n && x[i+1] < xPxNext)
                yMax = jmax (yMax, y[++i]);
            p.lineTo (x[i], toPxFromLinear (yMax));
            ++i;
            xPx = static_cast<int> (x[i]);
        }
        
        const auto pst = PathStrokeType (1.0f);
        g.setColour (getColourForChannel (ch));
        g.strokePath(p, pst);
    }

    // Output mouse co-ordinates in Hz/dB
    if (currentX >= 0 && currentY >= 0)
    {
        g.setColour (Colours::white);
        g.setFont (Font (GUI_SIZE_F(0.5)));
        const auto freq = toHzFromPx (static_cast<float> (currentX));
        const auto freqStr = hertzToString (freq, 2, true, true);
        const auto dbStr = String (toDbVFromPx (static_cast<float> (currentY)), 1);
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
        g.drawText (txt, lblX, lblY, lblW, lblH, lblJust, false);
    }
}

template <int Order>
void FftScope<Order>::paintFftScale (Graphics& g) const
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
        const auto dBStr = String (static_cast<int> (toDbVFromPx (scaleY)));
        const auto lblX = GUI_SIZE_I(0.1);
        const auto lblY = static_cast<int> (scaleY) + GUI_SIZE_I(0.1);
        const auto lblW = GUI_SIZE_I(1.1);
        const auto lblH = static_cast<int> (scaleY) + GUI_SIZE_I(0.6);
        //g.drawFittedText (dB, lblX, lblY, lblW, lblH, Justification::topLeft, 1, 1.0f);
        g.drawText (dBStr, lblX, lblY, lblW, lblH, Justification::topLeft, false);
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
			const auto scaleX = static_cast<int> (toPxFromHz (f));
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

template <int Order>
inline float FftScope<Order>::toDbVFromLinear (const float linear) const
{
    if (linear <= 0.0f)
        return dBmin;
    else
        //return log10(x) * 20.0f;
        return jmax (dBmin, fasterlog2 (linear) * 6.0206f);
}

template <int Order>
inline float FftScope<Order>::toPxFromLinear (const float linear) const
{
    return toPxFromDbV (toDbVFromLinear (linear));
}

template <int Order>
inline float FftScope<Order>::toPxFromDbV(const float dB) const
{
    return jmax (1.0f, (dB - dBmax) * yRatio) - 1.0f;
}

template <int Order>
inline float FftScope<Order>::toDbVFromPx (const float yInPixels) const
{
    return (yInPixels + 1.0f) / yRatio  + dBmax;
}

template <int Order>
inline float FftScope<Order>::toHzFromPx (const float xInPixels) const
{
    //return powf(10.0f, xInPixels / xRatio + minLogFreq);
    return fastpow10 (xInPixels / xRatio + minLogFreq);
}

template <int Order>
inline float FftScope<Order>::toPxFromHz (const float xInHz) const
{
    // Only used occasionally so don't need performance
    return (log10 (xInHz) - minLogFreq) * xRatio;
}

template <int Order>
String FftScope<Order>::hertzToString (const double frequencyInHz, const int numDecimals, const bool appendHz, const bool includeSpace) const
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

template <int Order>
Colour FftScope<Order>::getColourForChannel (const int channel)
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

template <int Order>
void FftScope<Order>::initialise()
{
    const auto nyquist = static_cast<float> (samplingFreq * 0.5);
    if (maxFreq == 0.0f)
        maxFreq = nyquist;
    else
        maxFreq = jmin (maxFreq, nyquist);
    minLogFreq = log10 (minFreq);
    logFreqSpan = log10 (maxFreq) - minLogFreq;
    const auto n = fftProcessor->getMaximumBlockSize() / 2;
    xRatio = static_cast<float> (getWidth()) / logFreqSpan;
    const auto binToHz = nyquist / static_cast<float> (n);

    for (auto i = 1; i <= n; ++i)
        x[i] = toPxFromHz (static_cast<float> (i) * binToHz);

    yRatio = static_cast<float> (getHeight()) / (dBmin - dBmax);
}