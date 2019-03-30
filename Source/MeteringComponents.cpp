/*
  ==============================================================================

    MeteringComponents.cpp
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MeteringComponents.h"
#include "AnalyserComponent.h"

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

MainMeterBackground::MainMeterBackground()
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
    const auto channelHeight = getHeight() - topMargin - bottomMargin - clipIndicatorHeight;
    scaling = channelHeight / (maxExp - minExp);

    clientArea =  getBoundsInParent()
        .reduced (GUI_BASE_GAP_I, 0)
        .withTrimmedTop (static_cast<int> (topMargin))
        .withTrimmedBottom (static_cast<int> (bottomMargin));
    if (dBScaleWidth > 0)
        clientArea = clientArea.withTrimmedRight (dBScaleWidth);
        
    channelWidth = clientArea.getWidth();
    if (numChannels > 0)
        channelWidth = (clientArea.getWidth() - gap * (numChannels + 1)) / numChannels;
}
void MainMeterBackground::setNumChannels(const int numberOfChannels)
{
    numChannels = numberOfChannels;
}
Grid::Px MainMeterBackground::getDesiredWidth() const
{
    if (numChannels <1 )
        return  Grid::Px (0);

    return Grid::Px (dBScaleWidth + numChannels * desiredBarWidth + (numChannels - 1) * gap);
}
Rectangle<int> MainMeterBackground::getMeterBarBoundsInParent (const int channel, const bool isPeakMeter) const
{
    const auto barClientArea = clientArea.withTrimmedTop (clipIndicatorHeight);
    auto meterWidth = channelWidth / 3;
    auto offset = 0;
    if (isPeakMeter)
    {
        meterWidth = channelWidth * 2 / 3;
        offset = channelWidth - meterWidth;
    }
    return barClientArea.withLeft (getChannelLeft (channel, offset)).withWidth (meterWidth);
}
Rectangle<int> MainMeterBackground::getClipIndicatorBoundsInParent (const int channel) const
{
    const auto clipIndicatorClientArea = clientArea.withHeight (clipIndicatorHeight);
    return clipIndicatorClientArea.withLeft (getChannelLeft (channel)).withWidth (channelWidth);
}
float MainMeterBackground::getScaleMax() const
{
    return scaleMax;
}
float MainMeterBackground::getScaleMin() const
{
    return scaleMin;
}
int MainMeterBackground::getChannelLeft (const int channel, const int offset) const
{
    return clientArea.getX() + (channelWidth + gap) * channel + offset + gap;
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
    const auto backingHeight = static_cast<float> (getHeight());

    // Draw background
    g.setColour(Colour::fromRGB (20, 20, 20));
    g.fillRoundedRectangle (0.0f, 0.0f, backingWidth, backingHeight, static_cast<float> (gap) * 0.5f);

    g.setFont (fontHeight);

    // Exponentially mapped dB scale
    for (auto i = 0; i <= numSteps; ++i)
    {
        g.setColour (scaleColour);
        const auto dB = scaleMax - scaleStep * static_cast<float>(i);
        const auto y = topMargin + clipIndicatorHeight + (maxExp - dsp::FastMathApproximations::exp (dB / scaleSpan)) * scaling;
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
    }  // NOLINT(hicpp-member-init)
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
}
void ClipStatsComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}
void ClipStatsComponent::resized ()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_GAP_PX(gap);
    grid.columnGap = GUI_GAP_PX(gap);
    grid.templateRows = {
        Track (1_fr),
        Track (GUI_BASE_SIZE_PX),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (GUI_SIZE_PX (rowHeight)),
        Track (1_fr)
    };
    grid.templateColumns = { Track (GUI_SIZE_PX(headingWidth)) };
    grid.autoColumns = Track (1_fr);
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
    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I (gap)));
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

    setSize (getDesiredWidth(), getDesiredHeight());
}
void ClipStatsComponent::assignProcessor (ClipCounterProcessor* clipCounterProcessor)
{
    processor = clipCounterProcessor;
}
void ClipStatsComponent::updateStats()
{
    if (!isShowing())
        return;

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
int ClipStatsComponent::getDesiredWidth() const
{
    const auto channelWidth = 2.0;
    const auto gaps = gap * (numChannels + 2);
    const auto width = GUI_SIZE_I (headingWidth + channelWidth * numChannels) + GUI_GAP_I (gaps);
    // TODO - implement scrolling then limit max width to 800
    const auto maxWidth = Desktop::getInstance().getDisplays().getMainDisplay().userArea.getWidth();
    return jlimit(100, maxWidth, width);
}
int ClipStatsComponent::getDesiredHeight() const
{
    return GUI_SIZE_I (1.0 + rowHeight * 4.0) + GUI_GAP_I (gap * 8.0);
}
ClipIndicatorComponent::ClipIndicatorComponent (const int channel, ClipCounterProcessor* clipCounterProcessorToReferTo)
{
    channelNumber = channel;
    clipCounterProcessor = clipCounterProcessorToReferTo;
}
void ClipIndicatorComponent::paint (Graphics& g)
{
    if (clipCounterProcessor && clipCounterProcessor->getNumClipEvents (channelNumber) > 0)
        g.setColour(Colours::red);
    else
        g.setColour(Colours::white.withAlpha (0.5f));

    const float fontHeight = static_cast<float> (getHeight()) * 0.8f;
    g.setFont (fontHeight);
    g.drawFittedText ("CLIP", 0, 0, getWidth(), getHeight(), Justification::centredTop, 1);
}
void ClipIndicatorComponent::mouseDown (const MouseEvent&)
{
    auto* parent = dynamic_cast<AnalyserComponent*> (getParentComponent());
    parent->showClipStats();
}
String ClipIndicatorComponent::getTooltip()
{
    return String("Click to show clip stats");
}
