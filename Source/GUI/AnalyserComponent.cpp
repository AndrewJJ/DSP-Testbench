/*
  ==============================================================================

    AnalyserComponent.cpp
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "AnalyserComponent.h"
#include "../Main.h"

AnalyserComponent::AnalyserComponent()
    : keyName ("Analyser")
{
    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config = propertiesFile->getXmlValue (keyName);
    if (!config)
        config = std::make_unique<XmlElement> (keyName);

    lblTitle.setName ("Analyser label");
    lblTitle.setText ("Analyser", dontSendNotification);
    lblTitle.setFont (Font (FontOptions (GUI_SIZE_F(0.7), Font::bold)));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (lblTitle);

    btnConfig = std::make_unique<DrawableButton> ("Config", DrawableButton::ImageFitted);
    addAndMakeVisible (btnConfig.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnConfig.get(), BinaryData::configure_svg, BinaryData::configure_svgSize, Colours::black);
    btnConfig->setTooltip ("Configure analyser settings");
    
    btnPause = std::make_unique<DrawableButton> ("Disable", DrawableButton::ImageFitted);
    addAndMakeVisible (btnPause.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnPause.get(), BinaryData::pause_svg, BinaryData::pause_svgSize, Colours::black, Colours::red);
    btnPause->setTooltip ("Pause the analyser");
    btnPause->setClickingTogglesState (true);
    statusActive.set (true);
    btnPause->setToggleState (!statusActive.get(), dontSendNotification);
    btnPause->onClick = [this]
    {
        statusActive.set (!btnPause->getToggleState());
        fftScope.setMouseMoveRepaintEnablement (!statusActive.get());
        oscilloscope.setMouseMoveRepaintEnablement (!statusActive.get());
    };

    btnExpand = std::make_unique<DrawableButton> ("Expand", DrawableButton::ImageFitted);
    addAndMakeVisible (btnExpand.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnExpand.get(), BinaryData::expand_svg, BinaryData::expand_svgSize, Colours::black, Colours::yellow);
    btnExpand->setTooltip ("Expand analyser");
    btnExpand->setClickingTogglesState (true);
    btnExpand->onClick = [this] 
    {
        auto* parent = dynamic_cast<MainContentComponent*> (this->getParentComponent());
        parent->setAnalyserExpanded (btnExpand->getToggleState());
    };

    addAndMakeVisible (fftScope);
    fftScope.assignFftProcessor (&fftProcessor);
    fftScope.setAggregationMethod (static_cast<const FftScope<12>::AggregationMethod> (config->getIntAttribute ("FftAggregationMethod", static_cast<int> (FftScope<12>::AggregationMethod::Maximum))));
    fftScope.setReleaseCharacteristic (static_cast<const FftScope<12>::ReleaseCharacteristic> (config->getIntAttribute ("FftReleaseCharacteristic", static_cast<int> (FftScope<12>::ReleaseCharacteristic::Off))));

    addAndMakeVisible (oscilloscope);
    oscilloscope.assignAudioScopeProcessor (&audioScopeProcessor);
    oscilloscope.setXMin (config->getIntAttribute ("ScopeXMin", 0));
    oscilloscope.setXMax (config->getIntAttribute ("ScopeXMax", oscilloscope.getDefaultXMaximum()));
    oscilloscope.setMaxAmplitude (static_cast<float> (config->getDoubleAttribute("ScopeMaxAmplitude", 1.0)));
    oscilloscope.setAggregationMethod (static_cast<const Oscilloscope::AggregationMethod> (config->getIntAttribute ("ScopeAggregationMethod", static_cast<int> (Oscilloscope::AggregationMethod::NearestSample))));

    addAndMakeVisible (goniometer);
    goniometer.assignAudioScopeProcessor (&audioScopeProcessor);

    addAndMakeVisible (mainMeterBackground);
    
    clipStatsComponent.assignProcessor (&clipCounterProcessor);
    clipStatsViewport.setViewedComponent (&clipStatsComponent, false);
    clipStatsViewport.setScrollBarsShown (false, true);

    // Construct config component last so it picks up the correct values
    configComponent = std::make_unique<AnalyserConfigComponent> (this);
    btnConfig->onClick = [this] { 
        DialogWindow::showDialog ("Analyser configuration", configComponent.get(), nullptr, Colours::darkgrey, true);
    };

    startTimerHz(50);
}
AnalyserComponent::~AnalyserComponent()
{
    // Update configuration from class state
    config->setAttribute ("FftAggregationMethod", static_cast<int> (fftScope.getAggregationMethod()));
    config->setAttribute ("FftReleaseCharacteristic", static_cast<int> (fftScope.getReleaseCharacteristic()));
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
    if (getHeight() < GUI_SIZE_I(2))
        return; // Prevents an issue with performGridLayout during construction

    using Track = Grid::TrackInfo;

    Grid titleBarGrid;
    titleBarGrid.templateRows = { Track (1_fr) };
    titleBarGrid.templateColumns = { Track (1_fr) };
    titleBarGrid.autoColumns = Track (GUI_SIZE_PX (0.7));
    titleBarGrid.columnGap = GUI_GAP_PX (1);
    titleBarGrid.autoFlow = Grid::AutoFlow::column;
    titleBarGrid.items.addArray({ GridItem (lblTitle), GridItem (btnPause.get()), GridItem (btnExpand.get()), GridItem (btnConfig.get()) });
    Rectangle<int> titleGridBounds = getLocalBounds().withTrimmedLeft (GUI_GAP_I(1)).withTrimmedRight (GUI_GAP_I(2)).withTrimmedTop (GUI_GAP_I(1)).withHeight (GUI_BASE_SIZE_I);
    titleBarGrid.performLayout (titleGridBounds);

    Rectangle<int> analyserGridBounds = getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)).withTrimmedTop (titleGridBounds.getHeight());
    Grid::Px phaseScopeWidth (jmin (250, analyserGridBounds.getHeight()));
    Grid analyserGrid;
    analyserGrid.rowGap = GUI_GAP_PX(2);
    analyserGrid.columnGap = GUI_GAP_PX(2);
    analyserGrid.templateRows = { Track (1_fr), Track (1_fr) };
    analyserGrid.templateColumns = {
        Track (1_fr), // FFT & oscilloscope
        Track (phaseScopeWidth),
        Track (mainMeterBackground.getDesiredWidth())
    };
    analyserGrid.items.addArray({
        GridItem (fftScope).withArea (1, 1),
        GridItem (oscilloscope).withArea (2, 1),
        GridItem (goniometer).withArea (GridItem::Span (2), 2),
        GridItem (mainMeterBackground).withArea (GridItem::Span (2), 3)
    });
    analyserGrid.performLayout (analyserGridBounds);

    // Force a resize of meters so positions of related components need to be updated even if the actual meter background didn't change size.
    mainMeterBackground.resized();

    // Set bounds of meter bars & clip indicators
    for (auto ch = 0; ch < numChannels; ++ch)
    {
        if (peakMeterBars[ch] != nullptr)
            peakMeterBars[ch]->setBounds (mainMeterBackground.getMeterBarBoundsInParent (ch, true));

        if (vuMeterBars[ch] != nullptr)
            vuMeterBars[ch]->setBounds (mainMeterBackground.getMeterBarBoundsInParent (ch, false));

        if (clipIndicators[ch] != nullptr)
            clipIndicators[ch]->setBounds (mainMeterBackground.getClipIndicatorBoundsInParent (ch));
    }
}
void AnalyserComponent::timerCallback()
{
    for (auto ch = 0; ch < numChannels; ++ch)
    {
        if (peakMeterBars[ch])
            peakMeterBars[ch]->setLevel (peakMeterProcessor.getLevelDb (ch));

        if (vuMeterBars[ch])
            vuMeterBars[ch]->setLevel (vuMeterProcessor.getLevelDb (ch));

        if (clipIndicators[ch] != nullptr)
            clipIndicators[ch]->repaint();
    }
    clipStatsComponent.updateStats();
}
void AnalyserComponent::prepare (const dsp::ProcessSpec& spec)
{
    if (spec.numChannels > 0)
    {
        fftProcessor.prepare (spec);
        fftScope.prepare (spec);
        audioScopeProcessor.prepare (spec);
        oscilloscope.prepare();
        goniometer.prepare();
        peakMeterProcessor.prepare (spec);
        vuMeterProcessor.prepare (spec);
        clipCounterProcessor.prepare (spec);
        // If number of channels has changed, then re-initialise the meter bar components
        if (static_cast<int> (spec.numChannels) != numChannels)
        {
            numChannels = static_cast<int> (spec.numChannels);
            peakMeterBars.clear();
            vuMeterBars.clear();
            clipIndicators.clear();
            for (auto ch = 0; ch < numChannels; ++ch)
            {
                // Add VU meters
                addAndMakeVisible (vuMeterBars.add (new MeterBar()));
                vuMeterBars[ch]->setMaxDb(mainMeterBackground.getScaleMax());
                vuMeterBars[ch]->setMinDb(mainMeterBackground.getScaleMin());
                vuMeterBars[ch]->setBackgroundColour (Colours::transparentBlack);

                // Add peak meters
                addAndMakeVisible (peakMeterBars.add (new MeterBar()));
                peakMeterBars[ch]->setMaxDb(mainMeterBackground.getScaleMax());
                peakMeterBars[ch]->setMinDb(mainMeterBackground.getScaleMin());
                peakMeterBars[ch]->setBackgroundColour (Colours::transparentBlack);

                // Add clip indicators
                addAndMakeVisible (clipIndicators.add (new ClipIndicatorComponent (ch, &clipCounterProcessor)));
            }
            mainMeterBackground.setNumChannels (numChannels);
            clipStatsComponent.setNumChannels (numChannels);
            resized();
        }
    }
    else
    {
        peakMeterBars.clear();
        vuMeterBars.clear();
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
        fftProcessor.appendData (chNum, numSamples, audioData);
        audioScopeProcessor.appendData (chNum, numSamples, audioData);
    }
    peakMeterProcessor.process (context);
    vuMeterProcessor.process (context);
    clipCounterProcessor.process (context);
}
void AnalyserComponent::reset()
{
    clipCounterProcessor.reset();
}
bool AnalyserComponent::isProcessing() const noexcept
{
    return statusActive.get();
}
void AnalyserComponent::activateProcessing()
{
    statusActive.set (true);
    const MessageManagerLock mmLock;
    btnPause->setEnabled (true);
    btnPause->setToggleState(false, dontSendNotification);
    fftScope.setMouseMoveRepaintEnablement (false);
    oscilloscope.setMouseMoveRepaintEnablement (false);
}
void AnalyserComponent::suspendProcessing()
{
    statusActive.set (false);
    const MessageManagerLock mmLock;
    btnPause->setToggleState(true, dontSendNotification);
    btnPause->setEnabled (false);
    fftScope.setMouseMoveRepaintEnablement (true);
    oscilloscope.setMouseMoveRepaintEnablement (true);
}
void AnalyserComponent::showClipStats()
{
    auto maxWidth = DSPTestbenchApplication::getApp().getMainWindow().getConstrainer()->getMinimumWidth();
    if (const auto* primaryDisplay = Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        const auto primaryDisplayWidth = primaryDisplay->userArea.getWidth() - 100;
        maxWidth = jmax ( maxWidth, primaryDisplayWidth);
    }
    const auto viewPortWidth = jmin (clipStatsComponent.getDesiredWidth(), maxWidth);
    clipStatsViewport.setSize (viewPortWidth, clipStatsComponent.getDesiredHeight());

    DialogWindow::LaunchOptions launchOptions;
    launchOptions.dialogTitle = "Clip Stats";
    launchOptions.useNativeTitleBar = false;
    launchOptions.dialogBackgroundColour = DspTestBenchLnF::ApplicationColours::componentBackground();
    launchOptions.componentToCentreAround = &mainMeterBackground;
    launchOptions.resizable = false;
    launchOptions.content.set (&clipStatsViewport, false);

    if (!clipStatsWindow)
        clipStatsWindow.reset (launchOptions.create());
    if (clipStatsWindow)
    {
        clipStatsWindow->setVisible (true);
        clipStatsWindow->setAlwaysOnTop (true);
        clipStatsWindow->addToDesktop();
    }
}
int AnalyserComponent::getOscilloscopeMaximumBlockSize() const
{
    return oscilloscope.getMaximumBlockSize();
}

AnalyserComponent::AnalyserConfigComponent::AnalyserConfigComponent (AnalyserComponent* analyserToConfigure): analyserComponent(analyserToConfigure)
{
    auto* fftScopePtr = &analyserComponent->fftScope;
    auto* osc = &analyserComponent->oscilloscope;

    lblFftAggregation.setText("FFT scope aggregation method", dontSendNotification);
    lblFftAggregation.setJustificationType (Justification::centredRight);
    addAndMakeVisible(lblFftAggregation);

    cmbFftAggregation.setTooltip ("Defines how to aggregate samples if there are more than one per pixel in the plot.\n\nThe maximum method will better show the peak value of a harmonic, but will make white noise looks like it tails upwards. The average method will make white noise look flat but is more computationally intensive.");
    cmbFftAggregation.addItem ("Maximum", static_cast<int> (FftScope<12>::AggregationMethod::Maximum));
    cmbFftAggregation.addItem ("Average", static_cast<int> (FftScope<12>::AggregationMethod::Average));
    addAndMakeVisible (cmbFftAggregation);
    cmbFftAggregation.setSelectedId (static_cast<int> (fftScopePtr->getAggregationMethod()), dontSendNotification);
    cmbFftAggregation.onChange = [this, fftScopePtr]
    {
        fftScopePtr->setAggregationMethod (static_cast<const FftScope<12>::AggregationMethod>(cmbFftAggregation.getSelectedId()));
    };
    
    lblFftRelease.setText("FFT scope release characteristic", dontSendNotification);
    lblFftRelease.setJustificationType (Justification::centredRight);
    addAndMakeVisible(lblFftRelease);

    cmbFftRelease.setTooltip ("Set the release characteristic for the envelope applied to each FFT amplitude bin.");
    cmbFftRelease.addItem ("Off", FftScope<12>::ReleaseCharacteristic::Off);
    cmbFftRelease.addItem ("Quick", FftScope<12>::ReleaseCharacteristic::Quick);
    cmbFftRelease.addItem ("Medium", FftScope<12>::ReleaseCharacteristic::Medium);
    cmbFftRelease.addItem ("Slow", FftScope<12>::ReleaseCharacteristic::Slow);
    addAndMakeVisible (cmbFftRelease);
    cmbFftRelease.setSelectedId (fftScopePtr->getReleaseCharacteristic(), dontSendNotification);
    cmbFftRelease.onChange = [this, fftScopePtr]
    {
        fftScopePtr->setReleaseCharacteristic (static_cast<const FftScope<12>::ReleaseCharacteristic>(cmbFftRelease.getSelectedId()));
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

    const String helpText = "You can pan the waveform in the oscilloscope by dragging it to the left or right. "
        "You can zoom in on the time axis with the mouse wheel. If you hold down the shift key, you can zoom "
        "the amplitude axis. You can reset the default zoom by double-clicking.";
    txtHelp.setText(helpText, false);
    txtHelp.setMultiLine (true);
    txtHelp.setReadOnly (true);
    txtHelp.setCaretVisible (false);
    txtHelp.setPopupMenuEnabled (false);
    txtHelp.setScrollbarsShown (false);
    txtHelp.setColour (TextEditor::ColourIds::backgroundColourId, Colours::transparentBlack);
    txtHelp.setColour (TextEditor::ColourIds::outlineColourId, Colours::transparentBlack);
    addAndMakeVisible (txtHelp);

    setSize (800, 300);
}
void AnalyserComponent::AnalyserConfigComponent::resized ()
{
    Grid grid;
    grid.rowGap = GUI_GAP_PX(2);
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {
        Track(GUI_SIZE_PX(1.5)),
        Track(GUI_BASE_SIZE_PX),
        Track(GUI_BASE_SIZE_PX),
        Track(GUI_BASE_SIZE_PX),
        Track(1_fr)
    };

    grid.templateColumns = { Track(2_fr), Track(3_fr) };

    grid.autoColumns = Track(1_fr);
    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({
        GridItem(txtHelp).withArea( { }, GridItem::Span (2)),
        GridItem(lblFftAggregation), GridItem(cmbFftAggregation),
        GridItem(lblFftRelease), GridItem(cmbFftRelease),
        GridItem(lblScopeAggregation), GridItem(cmbScopeAggregation),
    });

    grid.performLayout(getLocalBounds().reduced(GUI_GAP_I(2), GUI_GAP_I(2)));
}
