/*
  ==============================================================================

    MeteringComponents.cpp
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MeteringComponents.h"

MeterBar::MeterBar()
{
    setPaintingIsUnclipped(true);
    setOpaque (backgroundColour.isOpaque());
}
MeterBar::MeterBar (const float minimumLevelDb, const float cautionLevelDb,
                                                      const float alertLevelDb, const float maximumLevelDb)
{
    minDb       = minimumLevelDb;
    cautionDb   = cautionLevelDb;
    alertDb     = alertLevelDb;
    maxDb       = maximumLevelDb;
    rangeDb     = maximumLevelDb - minimumLevelDb;
}
MeterBar::~MeterBar()
= default;
void MeterBar::paint (Graphics& g)
{
    auto colourNormal  = Colour (0xff00ff00);
	auto colourCaution = Colours::yellow;
	auto colourAlert   = Colour (0xffff0000);

    const auto fullHeight = static_cast<float> (getHeight());
	const auto gradientHeight = fullHeight * 0.5f;
	const auto fullWidth = static_cast<float> (getWidth());
	const auto darkShadow = 0.5f;

    g.setColour (backgroundColour);
    g.fillRect (0.0f, 0.0f, fullWidth, fullHeight);

    auto y = jmax (currentLevelY, cautionY);
	//g.setColour (colourNormal);
	g.setGradientFill (	ColourGradient	(	colourNormal, 0.0f, gradientHeight,
											colourNormal.darker (darkShadow), fullWidth, gradientHeight,
											false) );
    g.fillRect(0.0f, y, fullWidth, fullHeight - y);
	
    if (currentLevelY < cautionY)
	{
        y = jmax (currentLevelY, alertY);
		//g.setColour (colourCaution);
		g.setGradientFill (	ColourGradient	(	colourCaution, 0.0f, gradientHeight,
												colourCaution.darker(darkShadow), fullWidth, gradientHeight,
												false) );
        g.fillRect (0.0f, y, fullWidth, cautionY - y);
	}

    if (currentLevelY < alertY)
	{
		//g.setColour (colourAlert);
		g.setGradientFill (	ColourGradient	(	colourAlert, 0.0f, gradientHeight,
												colourAlert.darker (darkShadow), fullWidth, gradientHeight,
												false) );
        g.fillRect (0.0f, currentLevelY, fullWidth, alertY - currentLevelY);
	}
}
void MeterBar::resized ()
{
    rangeDb = maxDb - minDb;
    maxExp = dsp::FastMathApproximations::exp (maxDb / rangeDb);
    const auto minExp = dsp::FastMathApproximations::exp (minDb / rangeDb);
    conversionFactor    = getHeight() / (maxExp - minExp);
    currentLevelY       = dBtoPx (currentLevelDb);
    cautionY            = dBtoPx (cautionDb);
    alertY              = dBtoPx (alertDb);
}
void MeterBar::setLevel (const float dB)
{
    currentLevelDb = dB;
    currentLevelY = dBtoPx (dB);
    repaint();
}
void MeterBar::setMinDb (const float dB)
{
    minDb = dB;
    resized();
}
void MeterBar::setMaxDb (const float dB)
{
    maxDb = dB;
    resized();
}
void MeterBar::setBackgroundColour (const Colour newBackgroundColour)
{
    backgroundColour = newBackgroundColour;
    setOpaque (backgroundColour.isOpaque());
}
float MeterBar::dBtoPx (const float dB) const
{
    if (dB <= minDb)
        return static_cast<float> (getHeight());
    else
    {
        const auto dbClipped = jlimit (minDb, maxDb, dB);
        const auto dbExp = dsp::FastMathApproximations::exp (dbClipped / rangeDb);
        return (maxExp - dbExp) * conversionFactor;
    }
}

MainMeterBackground::MainMeterBackground ()
{
    setBufferedToImage (true);
    setOpaque (true);
}
void MainMeterBackground::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    drawScale(g);
}
void MainMeterBackground::resized()
{
}
Grid::Px MainMeterBackground::getDesiredWidth (const int numChannels) const
{
    if (numChannels <1 )
        return  Grid::Px (0);

    return Grid::Px (dBScaleWidth + numChannels * desiredBarWidth + (numChannels - 1) * gap);
}
Rectangle<int> MainMeterBackground::getMeterBarBoundsInParent(const int channel, const int numChannels, const bool isPeakMeter) const
{
    const auto barClientArea = getBarMeterAreaInParent();
    if (numChannels < 1)
        return barClientArea;

    const auto channelWidth = ((barClientArea.getWidth() - gap * (numChannels + 1)) / numChannels);
    int meterWidth;
    int offset;
    if (isPeakMeter)
    {
        meterWidth = channelWidth * 2 / 3;
        offset = channelWidth - meterWidth;
    }
    else
    {
        meterWidth = channelWidth / 3;
        offset = 0;
    }
    const auto barLeft = barClientArea.getX() + (channelWidth + gap) * channel + offset + gap;
    return barClientArea.withLeft (barLeft).withWidth (meterWidth);
}
float MainMeterBackground::getScaleMax() const
{
    return scaleMax;
}
float MainMeterBackground::getScaleMin() const
{
    return scaleMin;
}
Rectangle<int> MainMeterBackground::getBarMeterAreaInParent() const
{
    if (dBScaleWidth == 0)
        return this->getBoundsInParent().reduced (GUI_BASE_GAP_I, GUI_GAP_I(1.5));
    else
        return this->getBoundsInParent().reduced (GUI_BASE_GAP_I, GUI_GAP_I(1.5)).withTrimmedRight (dBScaleWidth);
}
Rectangle<int> MainMeterBackground::getBarMeterArea() const
{
    return this->getBounds().reduced (GUI_BASE_GAP_I, GUI_GAP_I(1.5)).withTrimmedRight (dBScaleWidth);
}
void MainMeterBackground::drawScale (Graphics& g) const
{
    const auto scaleColour = Colours::white.withAlpha (0.5f);
    const auto textColour = Colours::grey;
    const auto backingWidth = static_cast<float>(getWidth() - static_cast<float> (dBScaleWidth));
    const auto tickWidth = backingWidth - static_cast<float> (gap * 2);
    const auto textX = getWidth() - dBScaleWidth + gap;
    const auto textWidth = dBScaleWidth - 2 * gap;
    const auto channelHeight = getHeight() - 3 * gap;
    const auto scaleSpan = scaleMax - scaleMin;
    const auto numSteps = static_cast<int> (scaleSpan / scaleStep);
    const auto fontHeight = GUI_SIZE_F(0.4);

    // Draw background
    g.setColour(Colour::fromRGB (20, 20, 20));
    g.fillRoundedRectangle (0.0f, 0.0f, backingWidth, static_cast<float> (getHeight()), static_cast<float> (gap) * 0.5f);

    g.setFont (fontHeight);

    // Linearly mapped dB scale
    /*const auto heightStep = static_cast<float> (channelHeight) / static_cast<float> (numSteps);
    const auto labelHeight = heightStep / 2;
    const auto fontHeight = jmin (static_cast<float> (dBScaleWidth) * 0.5f, static_cast<float> (labelHeight));    

    for (auto i = 0; i <= numSteps; ++i)
    {
        g.setColour (scaleColour);
        const auto tickY = static_cast<float> (gap) * 1.5f + static_cast<float>(i) * heightStep;
        g.drawRect (static_cast<float> (gap), tickY - 0.5f, tickWidth, 1.0f);
        if (dBScaleWidth > 0)
        {
            g.setColour (textColour);
            g.drawFittedText (String (scaleMax - i * scaleStep),
                textX,
                static_cast<int> (tickY - labelHeight * 0.5f),
                textWidth,
                static_cast<int> (labelHeight),
                Justification::centredLeft,
                1);
        }
    }*/
    
    // Exponentially mapped dB scale
    // TODO: DRY the exponential scale conversion algorithm
    const auto max_exp = dsp::FastMathApproximations::exp (scaleMax / scaleSpan);
    const auto min_exp = dsp::FastMathApproximations::exp (scaleMin / scaleSpan);
    const auto conv = channelHeight / (max_exp - min_exp);
    const auto top = static_cast<float> (gap) * 1.5f;

    for (auto i = 0; i <= numSteps; ++i)
    {
        g.setColour (scaleColour);
        const auto dB = scaleMax - scaleStep * static_cast<float>(i);
        // (max_exp - db_exp) / span_exp * getHeight()
        const auto y = top + (max_exp - dsp::FastMathApproximations::exp (dB / scaleSpan)) * conv;
        g.drawRect (static_cast<float> (gap), y - 0.5f, tickWidth, 1.0f);
        if (dBScaleWidth > 0)
        {
            g.setColour (textColour);
            auto labelText = String(dB);
            if (dB>0)
                labelText = "+" + labelText;
            g.drawFittedText (
                labelText,
                textX,
                static_cast<int> (y - fontHeight * 0.5f),
                textWidth,
                static_cast<int> (fontHeight),
                Justification::centredLeft,
                1);
        }
    }
}