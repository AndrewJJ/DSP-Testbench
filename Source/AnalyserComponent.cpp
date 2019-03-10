/*
  ==============================================================================

    AnalyserComponent.cpp
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "AnalyserComponent.h"
#include "Main.h"

AnalyserComponent::AnalyserComponent()
    : keyName ("Analyser")
{
    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config.reset (propertiesFile->getXmlValue (keyName));
    if (!config)
        config.reset(new XmlElement (keyName));

    lblTitle.setName ("Analyser label");
    lblTitle.setText ("Analyser", dontSendNotification);
    lblTitle.setFont (Font (GUI_SIZE_F(0.7), Font::bold));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (lblTitle);

    addAndMakeVisible (btnConfig = new DrawableButton ("Config", DrawableButton::ImageFitted));
    DspTestBenchLnF::setImagesForDrawableButton (btnConfig, BinaryData::configure_svg, BinaryData::configure_svgSize, Colours::black);
    btnConfig->setTooltip ("Configure analyser settings");
    
    addAndMakeVisible (btnPause = new DrawableButton ("Disable", DrawableButton::ImageFitted));
    DspTestBenchLnF::setImagesForDrawableButton (btnPause, BinaryData::pause_svg, BinaryData::pause_svgSize, Colours::black, Colours::red);
    btnPause->setTooltip ("Pause the analyser");
    btnPause->setClickingTogglesState (true);
    statusActive.set (config->getBoolAttribute ("Active", true));
    btnPause->setToggleState (!statusActive.get(), dontSendNotification);
    btnPause->onClick = [this] { statusActive = !btnPause->getToggleState(); };

    addAndMakeVisible (btnExpand = new DrawableButton ("Expand", DrawableButton::ImageFitted));
    DspTestBenchLnF::setImagesForDrawableButton (btnExpand, BinaryData::expand_svg, BinaryData::expand_svgSize, Colours::black, Colours::yellow);
    btnExpand->setTooltip ("Expand analyser");
    btnExpand->setClickingTogglesState (true);
    btnExpand->onClick = [this] 
    {
        auto* parent = dynamic_cast<MainContentComponent*> (this->getParentComponent());
        parent->setAnalyserExpanded (btnExpand->getToggleState());
    };

    addAndMakeVisible (fftScope);
    fftScope.assignFftMult (&fftMult);
    fftScope.setAggregationMethod(static_cast<const FftScope<12>::AggregationMethod> (config->getIntAttribute ("FftAggregationMethod", FftScope<12>::AggregationMethod::Maximum)));

    addAndMakeVisible (oscilloscope);
    oscilloscope.assignOscProcessor (&oscProcessor);
    oscilloscope.setXMin (config->getIntAttribute ("ScopeXMin", 0));
    oscilloscope.setXMax (config->getIntAttribute ("ScopeXMax", 512));
    oscilloscope.setMaxAmplitude (static_cast<float> (config->getDoubleAttribute("ScopeMaxAmplitude", 1.0)));
    oscilloscope.setAggregationMethod (static_cast<const Oscilloscope::AggregationMethod> (config->getIntAttribute ("ScopeAggregationMethod", Oscilloscope::AggregationMethod::NearestSample)));

    addAndMakeVisible (meterBackground);

    // Construct config component last so it picks up the correct values
    configComponent.reset(new AnalyserConfigComponent(this));
    btnConfig->onClick = [this] { 
        DialogWindow::showDialog ("Analyser configuration", configComponent.get(), nullptr, Colours::darkgrey, true);
    };

    startTimerHz(50);
}
AnalyserComponent::~AnalyserComponent()
{
    // Update configuration from class state
    config->setAttribute ("Active", statusActive.get());
    config->setAttribute ("FftAggregationMethod", fftScope.getAggregationMethod());
    config->setAttribute ("ScopeXMin", oscilloscope.getXMin());
    config->setAttribute ("ScopeXMax", oscilloscope.getXMax());
    config->setAttribute ("ScopeMaxAmplitude", oscilloscope.getMaxAmplitude());
    config->setAttribute ("ScopeAggregationMethod", oscilloscope.getAggregationMethod());

    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue(keyName, config.get());
    propertiesFile->saveIfNeeded();
}
void AnalyserComponent::paint (Graphics& g)
{
    g.setColour (Colours::black);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void AnalyserComponent::resized()
{
    using Track = Grid::TrackInfo;

    Grid titleBarGrid;
    titleBarGrid.templateRows = { Track (1_fr) };
    //titleBarGrid.templateColumns = { Track (1_fr), Track (GUI_SIZE_PX(1.2)), Track (GUI_SIZE_PX(1.2)), Track (GUI_SIZE_PX(1.2)) };
    titleBarGrid.templateColumns = { Track (1_fr) };
    titleBarGrid.autoColumns = Track (GUI_SIZE_PX (0.7));
    titleBarGrid.columnGap = GUI_GAP_PX (1);
    titleBarGrid.autoFlow = Grid::AutoFlow::column;
    titleBarGrid.items.addArray({ GridItem (lblTitle), GridItem (btnPause), GridItem (btnExpand), GridItem (btnConfig) });
    titleBarGrid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)).withHeight(GUI_BASE_SIZE_I));

    Grid analyserGrid;
    analyserGrid.rowGap = GUI_GAP_PX(2);
    analyserGrid.columnGap = GUI_GAP_PX(2);
    analyserGrid.templateRows = { Track (1_fr), Track (1_fr) };
    // TODO - calculate desired meter width
    analyserGrid.templateColumns = { Track (1_fr), Track (meterBackground.getDesiredWidth (numChannels)) };
    analyserGrid.items.addArray({
                            GridItem (fftScope).withArea (1, 1),
                            GridItem (oscilloscope).withArea (2, 1),
                            GridItem (meterBackground).withArea (GridItem::Span (2), 2)
                        });
    analyserGrid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)).withTrimmedTop(GUI_BASE_SIZE_I + GUI_GAP_I(2)));

    // Set bounds of meter bars
    for (auto ch = 0; ch < numChannels; ++ch)
        if (meterBars[ch] != nullptr)
            meterBars[ch]->setBounds (meterBackground.getBarBoundsInParent (ch, numChannels));
}
void AnalyserComponent::timerCallback ()
{
    for (auto ch = 0; ch < numChannels; ++ch)
        if (meterBars[ch] != nullptr)
            meterBars[ch]->setLevel (peakMeterProcessor.getLevelDb (ch));
}
void AnalyserComponent::prepare (const dsp::ProcessSpec& spec)
{
    if (spec.numChannels > 0)
    {
        fftMult.prepare (spec);
        fftMult.prepare (spec);
        oscProcessor.prepare (spec);
        oscilloscope.prepare();
        peakMeterProcessor.prepare (spec);
        // If number of channels has changed, then re-initialise the meter bar components
        if (static_cast<int> (spec.numChannels) != numChannels)
        {
            numChannels = static_cast<int> (spec.numChannels);
            meterBars.clear();
            for (auto ch = 0; ch < numChannels; ++ch)
            {
                addAndMakeVisible (meterBars.add (new SimplePeakMeterComponent()));
                meterBars[ch]->setMaxDb(meterBackground.getScaleMax());
                meterBars[ch]->setMinDb(meterBackground.getScaleMin());
                meterBars[ch]->setBackgroundColour(Colours::transparentBlack);
            }
            resized();
        }
    }
    else
    {
        meterBars.clear();
        numChannels = 0;
        resized();
    }
}
void AnalyserComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    auto* inputBlock = &context.getInputBlock();
    for (size_t ch = 0; ch < inputBlock->getNumChannels(); ++ch)
    {
        const auto chNum = static_cast<int> (ch);
        const auto numSamples = static_cast<int> (inputBlock->getNumSamples());
        const auto* audioData = inputBlock->getChannelPointer (ch);
        fftMult.appendData (chNum, numSamples, audioData);
        oscProcessor.appendData (chNum, numSamples, audioData);
        peakMeterProcessor.process (context);
    }
}
void AnalyserComponent::reset ()
{ }
bool AnalyserComponent::isActive () const noexcept
{
    return statusActive.get();
}
void AnalyserComponent::activate()
{
    statusActive.set (true);
    btnPause->setToggleState(false, dontSendNotification);
}
int AnalyserComponent::getOscilloscopeMaximumBlockSize() const
{
    return oscilloscope.getMaximumBlockSize();
}

AnalyserComponent::AnalyserConfigComponent::AnalyserConfigComponent (AnalyserComponent* analyserToConfigure): analyserComponent(analyserToConfigure)
{
    auto* fft = &analyserComponent->fftScope;
    auto* osc = &analyserComponent->oscilloscope;

    lblFftAggregation.setText("FFT scope aggregation method", dontSendNotification);
    lblFftAggregation.setJustificationType (Justification::centredRight);
    addAndMakeVisible(lblFftAggregation);

    cmbFftAggregation.setTooltip ("Defines how to aggregate samples if there are more than one per pixel in the plot.\n\nThe maximum method will better show the peak value of a harmonic, but will make white noise looks like it tails upwards. The average method will make white noise look flat but is more computationally intensive.");
    cmbFftAggregation.addItem ("Maximum", FftScope<12>::AggregationMethod::Maximum);
    cmbFftAggregation.addItem ("Average", FftScope<12>::AggregationMethod::Average);
    addAndMakeVisible (cmbFftAggregation);
    cmbFftAggregation.setSelectedId (fft->getAggregationMethod(), dontSendNotification);
    cmbFftAggregation.onChange = [this, fft]
    {
        fft->setAggregationMethod (static_cast<const FftScope<12>::AggregationMethod>(cmbFftAggregation.getSelectedId()));
    };

    lblScopeAggregation.setText("Oscilloscope aggregation method", dontSendNotification);
    lblScopeAggregation.setJustificationType (Justification::centredRight);
    addAndMakeVisible(lblScopeAggregation);

    cmbScopeAggregation.setTooltip ("Defines how to aggregate samples if there are more than one per pixel in the plot (listed in order of computation cost).\n\nThe nearest sample method offers the best performance, but shows variable amplitude for higher frequency content. The maximum method better represents the envelope of higher frequency content but is more computationally intensive. The average method tends to show even lower amplitudes for higher frequency content than nearest sample and is most intensive.");
    cmbScopeAggregation.addItem ("Nearest sample", Oscilloscope::AggregationMethod::NearestSample);
    cmbScopeAggregation.addItem ("Maximum", Oscilloscope::AggregationMethod::Maximum);
    cmbScopeAggregation.addItem ("Average", Oscilloscope::AggregationMethod::Average);
    addAndMakeVisible (cmbScopeAggregation);
    cmbScopeAggregation.setSelectedId (osc->getAggregationMethod(), dontSendNotification);
    cmbScopeAggregation.onChange = [this, osc]
    {
        osc->setAggregationMethod (static_cast<const Oscilloscope::AggregationMethod>(cmbScopeAggregation.getSelectedId()));
    };

    lblScopeScaleX.setText ("Oscilloscope time scale (samples)", dontSendNotification);
    lblScopeScaleX.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblScopeScaleX);

    sldScopeScaleX.setSliderStyle (Slider::SliderStyle::TwoValueHorizontal);
    sldScopeScaleX.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    sldScopeScaleX.setNumDecimalPlacesToDisplay (0);
    sldScopeScaleX.setPopupDisplayEnabled (true, true, this);
    const auto xMax = analyserToConfigure->getOscilloscopeMaximumBlockSize();
    sldScopeScaleX.setTooltip ("Select range of samples from each " + String(xMax) + " sample frame for oscilloscope");
    sldScopeScaleX.setRange (0.0, static_cast<double> (xMax), 128.0);
    sldScopeScaleX.setMinAndMaxValues (osc->getXMin(), osc->getXMax(), dontSendNotification);
    addAndMakeVisible (sldScopeScaleX);
    sldScopeScaleX.onValueChange = [this, osc]
    {
        osc->setXMin(static_cast<int> (sldScopeScaleX.getMinValue()));
        osc->setXMax(static_cast<int> (sldScopeScaleX.getMaxValue()));
    };

    lblScopeScaleY.setText ("Oscilloscope amplitude scale (dB)", dontSendNotification);
    lblScopeScaleY.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblScopeScaleY);

    sldScopeScaleY.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    sldScopeScaleY.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    sldScopeScaleY.setNumDecimalPlacesToDisplay (0);
    sldScopeScaleY.setTextValueSuffix(" dB");
    sldScopeScaleY.setPopupDisplayEnabled (true, true, this);
    sldScopeScaleY.setTooltip ("Select maximum amplitude for oscilloscope");
    sldScopeScaleY.setRange (-100.0, 0.0, 1.0);
    // minus Infinity dB must be set lower than the slider minimum to prevent divide by zero
    sldScopeScaleY.setValue (Decibels::decibelsToGain (osc->getMaxAmplitude(), -150.0f), dontSendNotification);
    addAndMakeVisible (sldScopeScaleY);
    sldScopeScaleY.onValueChange = [this, osc]
    {
        // minus Infinity dB must be set lower than the slider minimum to prevent divide by zero
        osc->setMaxAmplitude(Decibels::decibelsToGain (static_cast<float> (sldScopeScaleY.getValue()), -150.0f));
    };

    setSize (800, 300);
}
void AnalyserComponent::AnalyserConfigComponent::resized ()
{
    Grid grid;
    grid.rowGap = GUI_GAP_PX(2);
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {
        Track(GUI_BASE_SIZE_PX),
        Track(GUI_BASE_SIZE_PX),
        Track(GUI_BASE_SIZE_PX),
        Track(GUI_BASE_SIZE_PX),
        Track(1_fr)
    };

    grid.templateColumns = { Track(2_fr), Track(3_fr) };

    grid.autoColumns = Track(1_fr);
    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({
        GridItem(lblFftAggregation), GridItem(cmbFftAggregation),
        GridItem(lblScopeAggregation), GridItem(cmbScopeAggregation),
        GridItem(lblScopeScaleX), GridItem(sldScopeScaleX),
        GridItem(lblScopeScaleY), GridItem(sldScopeScaleY)
    });

    grid.performLayout(getLocalBounds().reduced(GUI_GAP_I(2), GUI_GAP_I(2)));
}

AnalyserComponent::MeterBackground::MeterBackground ()
{
    setBufferedToImage (true);
    setOpaque (true);
}
void AnalyserComponent::MeterBackground::paint (Graphics& g)
{
    g.fillAll(Colours::black);
    drawScale(g);
}
void AnalyserComponent::MeterBackground::resized()
{
}
Grid::Px AnalyserComponent::MeterBackground::getDesiredWidth (const int numChannels) const
{
    if (numChannels <1 )
        return  Grid::Px (0);
    // else
    return Grid::Px (dBScaleWidth + numChannels * desiredBarWidth + (numChannels - 1) * gap);
}
Rectangle<int> AnalyserComponent::MeterBackground::getBarBoundsInParent (const int channel, const int numChannels) const
{
    const auto barClientArea = getBarMeterAreaInParent();
    if (numChannels < 1)
        return barClientArea;
    // else
    const auto barWidth = (barClientArea.getWidth() - gap * (numChannels + 1)) / numChannels;
    const auto barLeft = barClientArea.getX() + (barWidth + gap) * channel + gap;
    return barClientArea.withLeft(barLeft).withWidth(barWidth);
}
float AnalyserComponent::MeterBackground::getScaleMax() const
{
    return scaleMax;
}
float AnalyserComponent::MeterBackground::getScaleMin() const
{
    return scaleMin;
}
Rectangle<int> AnalyserComponent::MeterBackground::getBarMeterAreaInParent() const
{
    return this->getBoundsInParent().reduced (GUI_BASE_GAP_I, GUI_GAP_I(1.5)).withTrimmedRight (dBScaleWidth);
}
Rectangle<int> AnalyserComponent::MeterBackground::getBarMeterArea() const
{
    return this->getBounds().reduced (GUI_BASE_GAP_I, GUI_GAP_I(1.5)).withTrimmedRight (dBScaleWidth);
}
void AnalyserComponent::MeterBackground::drawScale (Graphics& g) const
{
    const auto backingWidth = static_cast<float>(getWidth() - static_cast<float> (dBScaleWidth));
    const auto channelHeight = getHeight() - 3 * gap;
	const auto numSteps = static_cast<int> ((scaleMax - scaleMin) / stepSize);
	const auto heightStep = static_cast<float> (channelHeight) / static_cast<float> (numSteps);
    const auto labelHeight = heightStep / 2;
    const auto fontHeight = jmin (static_cast<float> (dBScaleWidth) * 0.5f, static_cast<float> (labelHeight));
    const auto scaleColour = Colours::white;
    const auto tickWidth = backingWidth - static_cast<float> (gap * 2);
    const auto textX = getWidth() - dBScaleWidth + gap;
    const auto textWidth = dBScaleWidth - 2 * gap;

    g.setColour(Colour::fromRGB (30, 30, 30));
    g.fillRoundedRectangle(0.0f, 0.0f, backingWidth, static_cast<float> (getHeight()), static_cast<float> (gap) * 0.5f);

	g.setFont (fontHeight);
	for (auto i = 0; i <= numSteps; ++i)
	{
        g.setColour (scaleColour.withAlpha(0.5f));
        const auto tickY = static_cast<float> (gap) * 1.5f + static_cast<float>(i) * heightStep;
        g.drawRect (static_cast<float> (gap), tickY - 0.5f, tickWidth, 1.0f);
        g.setColour (scaleColour);
        g.drawFittedText (String (scaleMax - i * stepSize),
            textX,
            static_cast<int> (tickY - labelHeight * 0.5f),
            textWidth,
            static_cast<int> (labelHeight),
            Justification::centred,
            1);
	}
}
