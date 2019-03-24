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
    scaling    = getHeight() / (maxExp - minExp);
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
        return (maxExp - dbExp) * scaling;
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
    const auto channelHeight = getHeight() - 2 * verticalMargin;
    scaling = channelHeight / (maxExp - minExp);
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
    const auto numSteps = static_cast<int> (scaleSpan / scaleStep);
    const auto fontHeight = GUI_SIZE_F(0.4);

    // Draw background
    g.setColour(Colour::fromRGB (20, 20, 20));
    g.fillRoundedRectangle (0.0f, 0.0f, backingWidth, static_cast<float> (getHeight()), static_cast<float> (gap) * 0.5f);

    g.setFont (fontHeight);

    // Exponentially mapped dB scale
    for (auto i = 0; i <= numSteps; ++i)
    {
        g.setColour (scaleColour);
        const auto dB = scaleMax - scaleStep * static_cast<float>(i);
        // (max_exp - db_exp) / span_exp * getHeight()
        const auto y = verticalMargin + (maxExp - dsp::FastMathApproximations::exp (dB / scaleSpan)) * scaling;
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

ClipStatsComponent::ClipStatsComponent()
{
    lblClippedSamplesTitle.setText ("Clipped samples", dontSendNotification);
    lblClipEventsTitle.setText ("Clip events", dontSendNotification);
    lblAvgEventLengthTitle.setText ("Avg clip length", dontSendNotification);
    lblMaxEventLengthTitle.setText ("Max clip length", dontSendNotification);

    btnReset.setButtonText ("Reset");
    btnReset.onClick = [this]
    {
        if (processor)
            processor->reset();
    };

    addAndMakeVisible (lblClippedSamplesTitle);
    addAndMakeVisible (lblClipEventsTitle);
    addAndMakeVisible (lblAvgEventLengthTitle);
    addAndMakeVisible (lblMaxEventLengthTitle);
    addAndMakeVisible (btnReset);

    setSize (500,200);
}
void ClipStatsComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}
void ClipStatsComponent::resized ()
{
    using Track = Grid::TrackInfo;
    const auto rowHeight = 0.6;
    Grid grid;
    grid.rowGap = GUI_GAP_PX(2);
    grid.columnGap = GUI_GAP_PX(2);
    grid.templateRows = {
        Track (1_fr),
        Track (GUI_BASE_SIZE_PX),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (1_fr)
    };
    grid.templateColumns = { Track (GUI_SIZE_PX(3.7)) };
    //grid.autoRows = Track (GUI_SIZE_PX(0.6));
    grid.autoColumns = Track (1_fr);
    //grid.autoFlow = Grid::AutoFlow::column;
    grid.items.addArray ({
        GridItem ().withArea (1, GridItem::Span (numChannels + 1)),
        GridItem (btnReset).withArea (2, 1),
        GridItem (lblClippedSamplesTitle).withArea (3, 1),
        GridItem (lblClipEventsTitle).withArea (4, 1),
        GridItem (lblAvgEventLengthTitle).withArea (5, 1),
        GridItem (lblMaxEventLengthTitle).withArea (6, 1)
    });
    for (auto ch = 0; ch < numChannels; ++ch)
    {
        grid.items.addArray ({
            GridItem (lblChannelHeadings[ch]).withArea (2, ch + 2),
            GridItem (lblClippedSamples[ch]).withArea (3, ch + 2),
            GridItem (lblClipEvents[ch]).withArea (4, ch + 2),
            GridItem (lblAvgEventLength[ch]).withArea (5, ch + 2),
            GridItem (lblMaxEventLength[ch]).withArea (6, ch + 2)
        });
    }
    grid.performLayout (getLocalBounds());
}
void ClipStatsComponent::setNumChannels(const int numberOfChannels)
{
    numChannels = numberOfChannels;

    lblChannelHeadings.clear();
    lblClippedSamples.clear();
    lblClipEvents.clear();
    lblAvgEventLength.clear();
    lblMaxEventLength.clear();

    for (auto ch = 0; ch < numberOfChannels; ++ch)
    {
        addAndMakeVisible (lblChannelHeadings.add (new Label(String(), "Ch " + String(ch + 1))));
        addAndMakeVisible (lblClippedSamples.add (new Label()));
        addAndMakeVisible (lblClipEvents.add (new Label()));
        addAndMakeVisible (lblAvgEventLength.add (new Label()));
        addAndMakeVisible (lblMaxEventLength.add (new Label()));
        
        lblChannelHeadings[ch]->setFont( Font(GUI_SIZE_I(0.7), Font::bold));
        lblChannelHeadings[ch]->setJustificationType (Justification::centred);
        lblClippedSamples[ch]->setJustificationType (Justification::centred);
        lblClipEvents[ch]->setJustificationType (Justification::centred);
        lblAvgEventLength[ch]->setJustificationType (Justification::centred);
        lblMaxEventLength[ch]->setJustificationType (Justification::centred);
    }
}
void ClipStatsComponent::assignProcessor (ClipCounterProcessor* clipCounterProcessor)
{
    processor = clipCounterProcessor;
}
void ClipStatsComponent::updateStats()
{
    jassert (numChannels == processor->getNumChannels());
    for (auto ch = 0; ch < numChannels; ++ch)
    {
        lblClippedSamples[ch]->setText (String (processor->getNumClippedSamples (ch)), dontSendNotification);
        lblClipEvents[ch]->setText (String (processor->getNumClipEvents (ch)), dontSendNotification);
        lblAvgEventLength[ch]->setText (String (processor->getAvgClipLength (ch), 1), dontSendNotification);
        lblMaxEventLength[ch]->setText (String (processor->getMaxClipLength (ch)), dontSendNotification);
    }
    repaint();
}
