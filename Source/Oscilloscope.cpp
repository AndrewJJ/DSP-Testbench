/*
  ==============================================================================

    Oscilloscope.cpp
    Created: 4 Feb 2018 2:34:56pm
    Author:  Andrew

  ==============================================================================
*/

#include "Oscilloscope.h"

Oscilloscope::Background::Background (Oscilloscope* parentOscilloscope)
    :   parentScope (parentOscilloscope)
{
    this->setOpaque (true);
    setBufferedToImage (true);
}
void Oscilloscope::Background::paint (Graphics& g)
{
    parentScope->paintScale (g);
}

Oscilloscope::Foreground::Foreground (Oscilloscope* parentOscilloscope)
    :   parentScope (parentOscilloscope)
{
    this->setPaintingIsUnclipped (true);
}
void Oscilloscope::Foreground::paint (Graphics& g)
{
    parentScope->paintWaveform (g);
}

Oscilloscope::Oscilloscope ()
    :   background (this),
        foreground (this),
        oscProcessor (nullptr)
{
    this->setOpaque (true);

    addAndMakeVisible (background);
    addAndMakeVisible (foreground);

    addMouseListener (this, true);
    foreground.setMouseCursor (MouseCursor::CrosshairCursor);

    dataFrameReady.set(false);
    startTimer (5);
}
Oscilloscope::~Oscilloscope ()
{
    // Remove listener callbacks so we don't leave anything hanging if we pop up an Oscilloscope then remove it
    if (removeListenerCallback)
        removeListenerCallback();
}
void Oscilloscope::paint (Graphics&)
{
    for (auto ch = 0; ch < oscProcessor->getNumChannels(); ++ch)
        oscProcessor->copyFrame (buffer.getWritePointer(ch), ch);
}
void Oscilloscope::resized ()
{
    calculateRatios();
    background.setBounds (getLocalBounds());
    foreground.setBounds (getLocalBounds());
}
void Oscilloscope::mouseMove (const MouseEvent& event)
{
    currentX = event.x;
    currentY = event.y;
}
void Oscilloscope::mouseExit (const MouseEvent&)
{
    // Set to -1 to indicate out of bounds
    currentX = -1;
    currentY = -1;
}
void Oscilloscope::timerCallback()
{
    // Only repaint if a new data frame is ready
    if (dataFrameReady.get())
    {
        repaint();
        dataFrameReady.set (false);
    }
}
void Oscilloscope::assignOscProcessor (OscilloscopeProcessor* oscProcessorPtr)
{
    jassert (oscProcessorPtr != nullptr);
    oscProcessor = oscProcessorPtr;
    if (maxXSamples == 0)
        maxXSamples = oscProcessor->getMaximumBlockSize();

    prepare();

    // As the frame size for the oscProcessor is set to 4096, updates arrive at ~11 Hz for a sample rate of 44.1 KHz.
    // Instead of repainting with a fixed timer we'll make a callback whenever a new data frame is delivered.
    removeListenerCallback = oscProcessor->addListenerCallback ([this] { dataFrameReady.set (true); });
}
void Oscilloscope::prepare()
{
    jassert (oscProcessor != nullptr); // oscProcessor should be assigned & prepared first
    buffer.setSize (oscProcessor->getNumChannels(), oscProcessor->getMaximumBlockSize());
    calculateRatios();
}
void Oscilloscope::setMaxAmplitude(const float maximumAmplitude)
{
    amplitudeMax = maximumAmplitude;
    calculateRatios();
    background.repaint();
}
float Oscilloscope::getMaxAmplitude () const
{
    return amplitudeMax;
}
void Oscilloscope::setXMin (const int minimumX)
{
    minXSamples = minimumX;
    calculateRatios();
    background.repaint();
}
int Oscilloscope::getXMin () const
{
    return minXSamples;
}
void Oscilloscope::setXMax (const int maximumX)
{
    maxXSamples = maximumX;
    calculateRatios();
    background.repaint();
}
int Oscilloscope::getXMax () const
{
    return maxXSamples;
}
int Oscilloscope::getMaximumBlockSize() const
{
    return oscProcessor->getMaximumBlockSize();
}
Oscilloscope::AggregationMethod Oscilloscope::getAggregationMethod () const
{
    return aggregationMethod;
}
void Oscilloscope::setAggregationMethod (const AggregationMethod method)
{
    aggregationMethod = method;
}
void Oscilloscope::paintWaveform (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level
    
    for (auto ch = 0; ch < oscProcessor->getNumChannels(); ++ch)
    {
        auto* y = buffer.getReadPointer (ch);

        // Draw a line representing the wave data for this channel
        Path p;
        p.preallocateSpace ((getWidth() + 1) * 3);
        p.startNewSubPath (0.0f, toPxFromAmp (y[minXSamples]));

        if (aggregationMethod == AggregationMethod::NearestSample)
        {
            // Iterate through pixels on x axis, plotting nearest sample
            auto lastXInSamples = -1;
            for (auto xPx = 1; xPx < getWidth(); xPx++)
            {
                const auto xInSamples = toTimeFromPx (static_cast<float> (xPx));
                // Avoid stair-casing by omitting points where x sample hasn't advanced from last pixel
                if (xInSamples != lastXInSamples)
                    p.lineTo (static_cast<float> (xPx), toPxFromAmp (y[xInSamples]));
                lastXInSamples = xInSamples;
            }
        }
        else
        {
            // Start path at first value
            auto i = minXSamples;
            const auto limit = maxXSamples - 1; // Reduce by 1 because of way while loop is structured
            auto curPx = toPxFromTime (i);

            // Iterate through samples, aggregating to pixels if sample interval is less than a pixel
            while (i < limit)
            {
                const auto nextPx = curPx + 1;
                if (aggregationMethod == AggregationMethod::Average)
                {
                    auto ySum = y[i];
                    auto count = 1;
                    while (i < limit && curPx < nextPx)
                    {
                        i++;
                        curPx = toPxFromTime (i);
                        ySum += y[i];
                        count++;
                    }
                    i++;
                    p.lineTo (curPx, toPxFromAmp (ySum / static_cast<float> (count)));
                }
                else //if (aggregationMethod == AggregationMethod::Maximum)
                {
                    auto yMax = y[i];
                    auto yMaxAbs = std::abs(y[i]);

                    while (i < limit && curPx < nextPx)
                    {
                        i++;
                        curPx = toPxFromTime (i);
                        if (std::abs(y[i]) > yMaxAbs)
                        {
                            yMax = y[i];
                            yMaxAbs = std::abs(y[i]);
                        }
                    }
                    i++;
                    p.lineTo (curPx, toPxFromAmp (yMax));
                }
            }
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
        const auto time = toTimeFromPx (static_cast<float> (currentX));
        const auto ampStr = String (toAmpFromPx (static_cast<float> (currentY)), 1);
        const auto txt =  String (time) + ", " + ampStr;
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
void Oscilloscope::paintScale (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level

    g.setColour (Colours::black);
    g.fillRect (getLocalBounds());

    const auto axisColour = Colours::darkgrey.darker();
    const auto textColour = Colours::grey.darker();

    g.setColour (axisColour);
    g.drawRect (getLocalBounds().toFloat());

    g.setFont (Font (GUI_SIZE_I(0.4)));

    // Plot amplitude scale (just halves, quarters or eighths)
    auto maxTicks = getHeight() / GUI_SIZE_I(2);
    int numTicks;
    if (maxTicks >= 8)
        numTicks = 8;
    else if (maxTicks >= 4)
        numTicks = 4;
    else
        numTicks = 2;

    // Draw y scale for amplitude
    for (auto t = 0; t < numTicks; ++t)
    {
        const auto scaleY = static_cast<float> (getHeight()) / static_cast<float> (numTicks) * static_cast<float> (t);
        g.setColour (axisColour);
        if (t > 0)
            g.drawHorizontalLine (static_cast<int> (scaleY), 0.0f, static_cast<float> (getWidth()));
        g.setColour (textColour);

        const auto yAmp = toAmpFromPx (scaleY);
        const auto yAmpDb = Decibels::gainToDecibels (std::abs(yAmp), -100.0f);
        const auto ampStrDb = String (static_cast<int> (yAmpDb)) + "dB";
        String lbl = "0";
        if (yAmpDb >-100.0f)
            lbl = String (yAmp) + ", " + ampStrDb;
        const auto lblX = GUI_SIZE_I(0.1);
        const auto lblY = static_cast<int> (scaleY) + GUI_SIZE_I(0.1);
        const auto lblW = GUI_SIZE_I(3.0);
        const auto lblH = static_cast<int> (scaleY) + GUI_SIZE_I(0.6);
        g.drawText (lbl, lblX, lblY, lblW, lblH, Justification::topLeft, false);
    }
   
    // Plot time scale (in samples)
    maxTicks = getWidth() / GUI_SIZE_I(2);
    numTicks = 0;
    if (maxTicks >= 16)
        numTicks = 16;
    else if (maxTicks >= 8)
        numTicks = 8;
    else if (maxTicks >= 4)
        numTicks = 4;
    else if (maxTicks >= 2)
        numTicks = 2;
    for (auto t = 0; t < numTicks; ++t)
    {
        const auto scaleX = static_cast<float> (getWidth()) / static_cast<float> (numTicks) * static_cast<float> (t);
        g.setColour (axisColour);
        if (t > 0)
            g.drawVerticalLine (static_cast<int> (scaleX), 0.0f, static_cast<float> (getWidth()));
        g.setColour (textColour);
        const auto timeStr = String (static_cast<int> (toTimeFromPx (scaleX)));
        const auto lblX = static_cast<int> (scaleX) + GUI_SIZE_I(0.1);
        const auto lblY = getHeight() - GUI_SIZE_I(0.6);
        const auto lblW = GUI_BASE_SIZE_I;
        const auto lblH = GUI_SIZE_I(0.5);
        g.drawText (timeStr, lblX, lblY, lblW, lblH, Justification::topLeft, false);
    }
}
inline float Oscilloscope::toAmpFromPx (const float yInPixels) const
{
    return amplitudeMax - yInPixels * yRatioInv;
}
inline float Oscilloscope::toPxFromAmp(const float amplitude) const
{
    return (amplitudeMax - jlimit (-amplitudeMax, amplitudeMax, amplitude)) * yRatio;
}
inline int Oscilloscope::toTimeFromPx (const float xInPixels) const
{
    return static_cast<int> (xInPixels * xRatioInv) + minXSamples;
}
inline float Oscilloscope::toPxFromTime (const int xInSamples) const
{
    return (xInSamples - minXSamples) * xRatio;
}
Colour Oscilloscope::getColourForChannel (const int channel)
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
void Oscilloscope::calculateRatios()
{
    maxXSamples = jmin (maxXSamples, oscProcessor->getMaximumBlockSize());
    xRatio = static_cast<float> (getWidth()) / static_cast<float> (maxXSamples - minXSamples);
    xRatioInv = 1.0f / xRatio;
    yRatio = static_cast<float> (getHeight()) / (amplitudeMax * 2.0f);
    yRatioInv = 1.0f / yRatio;
}