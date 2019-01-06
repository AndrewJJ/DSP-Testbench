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
    oscilloscope.setAggregationMethod (Oscilloscope::AggregationMethod::Sample);
    // TODO - set oscilloscope x axis so that performance doesn't choke
    //oscilloscope.setXMin (2000);
    oscilloscope.setXMax (500);
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

    grid.templateColumns = { Track (1_fr), Track (GUI_SIZE_PX(3)) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (btnDisable),
                            GridItem (fftScope).withArea ({}, GridItem::Span (2)),
                            GridItem (oscilloscope).withArea ({}, GridItem::Span (2))
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
