/*
  ==============================================================================

    Goniometer.cpp
    Created: 16 Mar 2019 1:13:13pm
    Author:  Andrew

  ==============================================================================
*/

#include "Goniometer.h"
#include "AnalyserComponent.h"

Goniometer::Background::Background (Goniometer* parentGoniometer)
    :   parentScope (parentGoniometer)
{
    this->setOpaque (true);
    setBufferedToImage (true);
}
void Goniometer::Background::paint (Graphics& g)
{
    parentScope->paintScale (g);
}

Goniometer::Foreground::Foreground (Goniometer* parentGoniometer)
    :   parentScope (parentGoniometer)
{
    this->setPaintingIsUnclipped (true);
}
void Goniometer::Foreground::paint (Graphics& g)
{
    parentScope->paintWaveform (g);
}

Goniometer::Goniometer ()
    :   background (this),
        foreground (this),
        audioScopeProcessor (nullptr)
{
    this->setOpaque (true);

    addAndMakeVisible (background);
    addAndMakeVisible (foreground);

    addMouseListener (this, true);

    dataFrameReady.set(false);
    startTimer (5);
}
Goniometer::~Goniometer ()
{
    masterReference.clear();
    // Remove listener callbacks so we don't leave anything hanging if we pop up an Goniometer then remove it
    removeListenerCallback();
}
void Goniometer::paint (Graphics&)
{
    for (auto ch = 0; ch < audioScopeProcessor->getNumChannels(); ++ch)
        audioScopeProcessor->copyFrame (buffer.getWritePointer(ch), ch);
}
void Goniometer::resized ()
{
    background.setBounds (getLocalBounds());
    foreground.setBounds (getLocalBounds());
}
void Goniometer::timerCallback()
{
    // Only repaint if a new data frame is ready (flag is set by a listener callback from the audio thread)
    if (dataFrameReady.get())
    {
        repaint();
        dataFrameReady.set (false);
    }
}
void Goniometer::assignAudioScopeProcessor (AudioScopeProcessor* audioScopeProcessorPtr)
{
    jassert (audioScopeProcessorPtr != nullptr);
    audioScopeProcessor = audioScopeProcessorPtr;
}
void Goniometer::prepare()
{
    jassert (audioScopeProcessor != nullptr); // audioScopeProcessor should be assigned & prepared first
    buffer.setSize (audioScopeProcessor->getNumChannels(), audioScopeProcessor->getMaximumBlockSize());
    WeakReference<Goniometer> weakThis = this;
    removeListenerCallback = audioScopeProcessor->addListenerCallback ([this, weakThis]
    {
        // Check the WeakReference because the callback may live longer than this Goniometer
        if (weakThis)
            dataFrameReady.set (true);
    });
}
void Goniometer::paintWaveform (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level

    if (audioScopeProcessor->getNumChannels() < 2)
        return;

    // Only use first two channels (ignore extra channels)
    const auto* x = buffer.getReadPointer (0);
    const auto* y = buffer.getReadPointer (1);

    const auto plotBoundsFloat = getPlotBounds().toFloat();
    const auto cx = plotBoundsFloat.getCentreX() - 0.5f;
    const auto cy = plotBoundsFloat.getCentreY() - 0.5f;
    const auto unitRadius = plotBoundsFloat.getWidth() / 2.0f;

    g.setColour (Colour(0xFFFFFF00).withMultipliedAlpha (0.5f));

    // Iterate through samples
    for (auto i = 0; i < buffer.getNumSamples(); i++)
    {
        // Convert cartesian to polar coordinate
        const auto amplitude = sqrt (x[i] * x[i] + y[i] * y[i]) + 1e-15f; // Anti-denormal float
        const auto radius = amplitude * unitRadius;

        if (radius <= unitRadius) // Don't draw points beyond our bounds
        {
            const auto angle = std::atan2 (y[i], x[i]) + 0.78539816f; // Rotate coordinate by 45 degrees counter clockwise
            const auto xx = cx + radius * cos (angle);
            const auto yy = cy + radius * sin (angle);
            g.fillRect (xx, yy, 1.0f, 1.0f);
        }
    }
}
void Goniometer::paintScale (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level
    g.setColour (Colours::black);
    g.fillRect (getLocalBounds());

    const auto axisColour = Colours::darkgrey.darker();
    const auto textColour = Colours::grey.darker();

    const auto plotBounds = getPlotBounds();
    const auto plotBoundsFloat = plotBounds.toFloat();

    // Plot axes
    g.setColour (axisColour);
    g.drawRect (plotBoundsFloat);
    g.drawLine (plotBoundsFloat.getX(), plotBoundsFloat.getY(), plotBoundsFloat.getRight(), plotBoundsFloat.getBottom());
    g.drawLine (plotBoundsFloat.getX(), plotBoundsFloat.getBottom(), plotBoundsFloat.getRight(), plotBoundsFloat.getY());

    // TODO: draw labels
    //g.setFont (Font (GUI_SIZE_I(0.4)));
    //g.setColour (textColour);
    //String lbl = "0";
    //lbl = String (yAmp) + ", " + ampStrDb;
    //const auto lblX = GUI_SIZE_I(0.1);
    //const auto lblY = static_cast<int> (scaleY) + GUI_SIZE_I(0.1);
    //const auto lblW = GUI_SIZE_I(3.0);
    //const auto lblH = static_cast<int> (scaleY) + GUI_SIZE_I(0.6);
    //g.drawText (lbl, lblX, lblY, lblW, lblH, Justification::topLeft, false);
}
Rectangle<int> Goniometer::getPlotBounds() const
{
    const auto smallestDimension = jmin (getLocalBounds().getWidth(), getLocalBounds().getHeight());
    return Rectangle<int> (getLocalBounds().withSizeKeepingCentre (smallestDimension, smallestDimension));
}
