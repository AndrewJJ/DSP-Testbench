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
    {
        // Define default properties to be used if user settings not already saved
        config.reset(new XmlElement (keyName));
        config->setAttribute ("Active", true);
    }

    lblTitle.setName ("Analyser label");
    lblTitle.setText ("Analyser", dontSendNotification);
    lblTitle.setFont (Font (GUI_SIZE_F(0.7), Font::bold));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    addAndMakeVisible (lblTitle);
    
    btnConfig.setButtonText ("Config");
    btnConfig.setToggleState (!statusActive.get(), dontSendNotification);
    addAndMakeVisible (btnConfig);
    
    btnDisable.setButtonText ("Disable");
    btnDisable.setClickingTogglesState (true);
    btnDisable.setColour(TextButton::buttonOnColourId, Colours::darkred);
    statusActive.set (config->getBoolAttribute ("Active"));
    btnDisable.setToggleState (!statusActive.get(), dontSendNotification);
    btnDisable.onClick = [this] { statusActive = !btnDisable.getToggleState(); };
    addAndMakeVisible (btnDisable);

    addAndMakeVisible (fftScope);
    fftScope.assignFftMult (&fftMult);
    //fftScope.setAggregationMethod (FftScope<12>::AggregationMethod::average);

    addAndMakeVisible (oscilloscope);
    oscilloscope.assignOscProcessor (&oscProcessor);
    oscilloscope.setAggregationMethod (Oscilloscope::AggregationMethod::NearestSample);
    // TODO - set oscilloscope x axis so that performance doesn't choke
    //oscilloscope.setXMin (2000);
    oscilloscope.setXMax (500);

    // Construct config component last so it picks up the correct values
    configComponent.reset(new AnalyserConfigComponent(this));
    btnConfig.onClick = [this] { 
        DialogWindow::showDialog ("Analyser configuration", configComponent.get(), nullptr, Colours::darkgrey, true);
    };
}
AnalyserComponent::~AnalyserComponent()
{
    // Update configuration from class state
    config->setAttribute ("Active", statusActive.get());

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
    Grid grid;
    grid.rowGap = GUI_GAP_PX(2);
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (GUI_SIZE_PX(2.2)), Track (GUI_SIZE_PX(2.3)) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (btnConfig),
                            GridItem (btnDisable),
                            GridItem (fftScope).withArea ({}, GridItem::Span (3)),
                            GridItem (oscilloscope).withArea ({}, GridItem::Span (3))
                        });

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
void AnalyserComponent::prepare (const dsp::ProcessSpec& spec)
{
    if (spec.numChannels > 0)
    {
        fftMult.prepare (spec);
        fftMult.prepare (spec);
        oscProcessor.prepare (spec);
        oscilloscope.prepare();
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
    }
}
void AnalyserComponent::reset ()
{ }
bool AnalyserComponent::isActive () const noexcept
{
    return statusActive.get();
}

AnalyserComponent::AnalyserConfigComponent::AnalyserConfigComponent (AnalyserComponent* analyserToConfigure): analyserComponent(analyserToConfigure)
{
    auto* osc = &analyserComponent->oscilloscope;

    lblAggregation.setText("Oscilloscope aggregation method", dontSendNotification);
    addAndMakeVisible(lblAggregation);

    cmbAggregation.setTooltip ("Defines how to aggregate samples if there are more than one per pixel in the plot (listed in order of computation cost)");
    cmbAggregation.addItem ("Nearest sample", Oscilloscope::AggregationMethod::NearestSample);
    cmbAggregation.addItem ("Maximum", Oscilloscope::AggregationMethod::Maximum);
    cmbAggregation.addItem ("Average", Oscilloscope::AggregationMethod::Average);
    addAndMakeVisible (cmbAggregation);
    cmbAggregation.setSelectedId (osc->getAggregationMethod(), dontSendNotification);
    cmbAggregation.onChange = [this, osc]
    {
        osc->setAggregationMethod (static_cast<const Oscilloscope::AggregationMethod>(cmbAggregation.getSelectedId()));
    };

    lblScaleX.setText ("Oscilloscope X scale", dontSendNotification);
    addAndMakeVisible (lblScaleX);

    sldScaleX.setSliderStyle (Slider::SliderStyle::TwoValueHorizontal);
    sldScaleX.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
    sldScaleX.setNumDecimalPlacesToDisplay (0);
    sldScaleX.setPopupDisplayEnabled (true, true, this);
    sldScaleX.setTooltip ("Select range of samples from each 8192 sample frame for display in oscilloscope");
    sldScaleX.setRange (0.0, 8192.0, 128.0);
    sldScaleX.setMinAndMaxValues (osc->getXMin(), osc->getXMax(), dontSendNotification);
    addAndMakeVisible (sldScaleX);
    sldScaleX.onValueChange = [this, osc]
    {
        osc->setXMin(static_cast<int> (sldScaleX.getMinValue()));
        osc->setXMax(static_cast<int> (sldScaleX.getMaxValue()));
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
        Track(1_fr)
    };

    grid.templateColumns = { Track(2_fr), Track(3_fr) };

    grid.autoColumns = Track(1_fr);
    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({
        GridItem(lblAggregation),
        GridItem(cmbAggregation),
        GridItem(lblScaleX),
        GridItem(sldScaleX)
    });

    grid.performLayout(getLocalBounds().reduced(GUI_GAP_I(2), GUI_GAP_I(2)));
}
