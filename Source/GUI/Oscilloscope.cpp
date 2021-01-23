/*
  ==============================================================================

    Oscilloscope.cpp
    Created: 4 Feb 2018 2:34:56pm
    Author:  Andrew

  ==============================================================================
*/

#include "Oscilloscope.h"
#include "LookAndFeel.h"

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
        audioScopeProcessor (nullptr)
{
    this->setOpaque (true);
    this->setPaintingIsUnclipped (true);

    addAndMakeVisible (background);
    addAndMakeVisible (foreground);

    addMouseListener (this, true);
    foreground.setMouseCursor (MouseCursor::CrosshairCursor);

    dataFrameReady.set(false);
    startTimer (5);
}
Oscilloscope::~Oscilloscope ()
{
    masterReference.clear();
    // Remove listener callbacks so we don't leave anything hanging if we pop up an Oscilloscope then remove it
    removeListenerCallback();
}
void Oscilloscope::paint (Graphics&)
{
    for (auto ch = 0; ch < audioScopeProcessor->getNumChannels(); ++ch)
        audioScopeProcessor->copyFrame (buffer.getWritePointer(ch), ch);
}
void Oscilloscope::resized ()
{
    preCalculateVariables();
    background.setBounds (getLocalBounds());
    foreground.setBounds (getLocalBounds());
}
void Oscilloscope::mouseDown (const MouseEvent&)
{
    xMinAtLastMouseDown = getXMin();
    xMaxAtLastMouseDown = getXMax();
}
void Oscilloscope::mouseDrag (const MouseEvent& event)
{   
    // Pan according to horizontal mouse movement
    const auto span = xMaxAtLastMouseDown - xMinAtLastMouseDown;
    const auto delta = event.getDistanceFromDragStartX() * span / getWidth();
    if (delta < 0)
    {
        maxXSamples = jlimit (128, getMaximumBlockSize() - 1, xMaxAtLastMouseDown - delta);
        // Subtracting a negative delta means we are increasing minXSamples, in which case we don't have limit check it
        minXSamples = maxXSamples - span;
    }
    else if (delta > 0)
    {
        minXSamples = jlimit (0, getMaximumBlockSize() - 128 - 1, xMinAtLastMouseDown - delta);
        // Subtracting a positive delta means we are decreasing maxXSamples, in which case we don't have limit check it
        maxXSamples = minXSamples + span;
    }
    background.repaint();

    // NOTE - vertical panning deliberately not implemented
}
void Oscilloscope::mouseDoubleClick (const MouseEvent& /*event*/)
{
    // Reset default zoom
    setXMin (0);
    setXMax (defaultMaxXSamples);
    setMaxAmplitude (1.0f);
    preCalculateVariables();
    background.repaint();
}
void Oscilloscope::mouseMove (const MouseEvent& event)
{
    currentX = event.x;
    currentY = event.y;

    // Allow mouse move repaints even if audio is not triggering repaints
    if (mouseMoveRepaintsEnabled)
        repaint();
}
void Oscilloscope::mouseExit (const MouseEvent&)
{
    // Set to -1 to indicate out of bounds
    currentX = -1;
    currentY = -1;

    // Force repaint to make sure cursor co-ordinates are removed
    if (mouseMoveRepaintsEnabled)
        repaint();
}
void Oscilloscope::mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel)
{
    if (getHeight() <= controlSize)
        return;

    if (ComponentPeer::getCurrentModifiersRealtime().isShiftDown())
    {
        // Zoom amplitude axis, centred about zero
        const auto newAmplitudeDb = Decibels::gainToDecibels(getMaxAmplitude()) - wheel.deltaY * 2;
        const auto newAmplitude = Decibels::decibelsToGain (newAmplitudeDb, -150.0f);
        setMaxAmplitude (newAmplitude);
        preCalculateVariables();
        background.repaint();
        repaint();
    }
    else
    {
        // Zoom x axis, centred on current position
        const auto delta = static_cast<int> (wheel.deltaY * 100);
        const auto span = maxXSamples - minXSamples;
        const auto fraction = static_cast<float> (event.x) / static_cast<float> (getWidth());
        const auto zoomPos = getXMin() + static_cast<int> (static_cast<float> (span) * fraction);
        const auto newSpan = span - delta;
        const auto newMinX = zoomPos - static_cast<int> (fraction * static_cast<float>(newSpan));
        const auto newMaxX = newMinX + newSpan;
        if (newSpan < 128) // Limit max zoom so we don't go in closer than 128 samples
            return;
        else if (newSpan >= getMaximumBlockSize())
        {
            minXSamples = 0;
            maxXSamples = getMaximumBlockSize() - 1;
        }
        else
        {
            if (newMinX >= 0 && newMinX < getMaximumBlockSize() - 128 && newMaxX >= 128 && newMaxX < getMaximumBlockSize())
            {
                minXSamples = newMinX;
                maxXSamples = newMaxX;               
            }
            else if (newMinX < 0 && newSpan < getMaximumBlockSize())
            {
                minXSamples = 0;
                maxXSamples = newSpan;
            }
            else if (newMaxX >= getMaximumBlockSize() && maxXSamples - newSpan >= 0)
            {
                maxXSamples = getMaximumBlockSize();
                minXSamples = maxXSamples - newSpan;
            }
        }
        preCalculateVariables();
        background.repaint();
        repaint();
    }
}
void Oscilloscope::timerCallback()
{
    // Only repaint if a new data frame is ready (flag is set by a listener callback from the audio thread)
    if (dataFrameReady.get())
    {
        repaint();
        dataFrameReady.set (false);
    }
}
void Oscilloscope::assignAudioScopeProcessor (AudioScopeProcessor* audioScopeProcessorPtr)
{
    jassert (audioScopeProcessorPtr != nullptr);
    audioScopeProcessor = audioScopeProcessorPtr;
    if (maxXSamples == 0)
        maxXSamples = audioScopeProcessor->getMaximumBlockSize();
}
void Oscilloscope::prepare()
{
    jassert (audioScopeProcessor != nullptr); // audioScopeProcessor should be assigned & prepared first
    buffer.setSize (audioScopeProcessor->getNumChannels(), audioScopeProcessor->getMaximumBlockSize());
    preCalculateVariables();
    WeakReference<Oscilloscope> weakThis = this;
    removeListenerCallback = audioScopeProcessor->addListenerCallback ([this, weakThis]
    {
        // Check the WeakReference because the callback may live longer than this Oscilloscope
        if (weakThis)
            dataFrameReady.set (true);
    });
}
void Oscilloscope::setMaxAmplitude(const float maximumAmplitude)
{
    const auto minimum = Decibels::decibelsToGain(-100.0f, -150.0f);
    amplitudeMax = jlimit (minimum, 2.0f, maximumAmplitude);
    preCalculateVariables();
    background.repaint();
}
float Oscilloscope::getMaxAmplitude () const
{
    return amplitudeMax;
}
void Oscilloscope::setXMin (const int minimumX)
{
    minXSamples = jlimit (0, audioScopeProcessor->getMaximumBlockSize() - 128, minimumX);
    preCalculateVariables();
    background.repaint();
}
int Oscilloscope::getXMin() const
{
    return minXSamples;
}
void Oscilloscope::setXMax (const int maximumX)
{
    maxXSamples = jlimit (128, audioScopeProcessor->getMaximumBlockSize(), maximumX);
    preCalculateVariables();
    background.repaint();
}
int Oscilloscope::getXMax() const
{
    return maxXSamples;
}
int Oscilloscope::getMaximumBlockSize() const
{
    return audioScopeProcessor->getMaximumBlockSize();
}
int Oscilloscope::getDefaultXMaximum() const
{
    return defaultMaxXSamples;
}
Oscilloscope::AggregationMethod Oscilloscope::getAggregationMethod () const
{
    return aggregationMethod;
}
void Oscilloscope::setAggregationMethod (const AggregationMethod method)
{
    aggregationMethod = method;
}
void Oscilloscope::setMouseMoveRepaintEnablement(const bool enableRepaints)
{
    mouseMoveRepaintsEnabled = enableRepaints;
}
void Oscilloscope::paintWaveform (Graphics& g) const
{
    // To speed things up we make sure we stay within the graphics context so we can disable clipping at the component level
    
    for (auto ch = 0; ch < audioScopeProcessor->getNumChannels(); ++ch)
    {
        auto* y = buffer.getReadPointer (ch);

        if (isnan (y[0]))
            break;

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

    // Output mouse co-ordinates in Hz/linear amplitude
    if (currentX >= 0 && currentY >= 0 && !isMouseButtonDown (true))
    {
        g.setColour (Colours::white);
        g.setFont (Font (GUI_SIZE_F(0.5)));
        const auto time = toTimeFromPx (static_cast<float> (currentX));

        const auto yAmp = toAmpFromPx (static_cast<float> (currentY));
        const auto ampStr = String (yAmp, 3);
        const auto yAmpDb = Decibels::gainToDecibels (std::abs (yAmp), -100.0f);
        const auto ampStrDb = String (yAmpDb, 1) + " dB";
        const auto txt =  String (time) + ", " + ampStr + " | " + ampStrDb;
        const auto offset = GUI_GAP_I(2);
        auto lblX = currentX + offset;
        auto lblY = currentY + offset;
        const auto lblW = GUI_SIZE_I(5.0);
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

    // Calculate number of divisions for amplitude scale (just halves, quarters or eighths)
    const auto maxDivsY = getHeight() / GUI_SIZE_I(2);
    int numDivsY;
    if (maxDivsY >= 8) numDivsY = 8;
    else if (maxDivsY >= 4) numDivsY = 4;
    else numDivsY = 2;

    // Draw y scale for amplitude
    for (auto t = 0; t < numDivsY; ++t)
    {
        const auto scaleY = static_cast<float> (getHeight()) / static_cast<float> (numDivsY) * static_cast<float> (t);
        g.setColour (axisColour);
        if (t > 0)
            g.drawHorizontalLine (static_cast<int> (scaleY), 0.0f, static_cast<float> (getWidth()));
        g.setColour (textColour);

        const auto yAmp = toAmpFromPx (scaleY);
        const auto yAmpDb = Decibels::gainToDecibels (std::abs (yAmp), -100.0f);
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
   
    // Calculate number of divisions for x axis
    const auto maxDivsX = getWidth() / GUI_SIZE_I(2);
    int numDivsX = 1;
    if (maxDivsX >= 16) numDivsX = 16;
    else if (maxDivsX >= 8) numDivsX = 8;
    else if (maxDivsX >= 4) numDivsX = 4;
    else if (maxDivsX >= 2) numDivsX = 2;
    //else return;

    // Calculate scale and offset for x axis tick marks
    const auto spanX = maxXSamples - minXSamples;
    const auto scaleX = static_cast<float> (getWidth()) / static_cast<float> (spanX);
    auto tickStepX = spanX / numDivsX;
    //tickStepX -= tickStepX % 2; // Adjust tick to be placed at nearest multiple of 16 samples
    //if (tickStepX == 0) return;
    const auto offsetX = minXSamples % tickStepX;

    // Draw x scale
    for (auto t = minXSamples - offsetX; t < maxXSamples; t += tickStepX)
    {
        const auto x = static_cast<int>(scaleX * static_cast<float>(t - minXSamples));
        g.setColour (axisColour);
        if (t > minXSamples)
            g.drawVerticalLine (x, 0.0f, static_cast<float> (getHeight()));
        g.setColour (textColour);
        const auto timeStr = String (t);
        const auto lblX = static_cast<int> (x) + GUI_SIZE_I(0.1);
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
void Oscilloscope::preCalculateVariables()
{
    xRatio = static_cast<float> (getWidth()) / static_cast<float> (maxXSamples - minXSamples);
    xRatioInv = 1.0f / xRatio;
    yRatio = static_cast<float> (getHeight()) / (amplitudeMax * 2.0f);
    yRatioInv = 1.0f / yRatio;
}