/*
  ==============================================================================

    MonitoringComponent.cpp
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MonitoringComponent.h"

MonitoringComponent::MonitoringComponent ()
{
    addAndMakeVisible (lblTitle = new Label ("Monitoring label", TRANS("Monitoring")));
    lblTitle->setFont (Font (15.00f, Font::bold));
    lblTitle->setJustificationType (Justification::topLeft);
    lblTitle->setEditable (false, false, false);
    lblTitle->setColour (TextEditor::textColourId, Colours::black);
    lblTitle->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sldOutputGain = new Slider ("Output gain slider"));
    sldOutputGain->setTooltip (TRANS("Allows gain adjustment of the output to your audio device"));
    sldOutputGain->setRange (-100, 0, 0.1);
    sldOutputGain->setSliderStyle (Slider::LinearHorizontal);
    sldOutputGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldOutputGain->addListener (this);

    addAndMakeVisible (btnLimiter = new TextButton ("Limiter button"));
    btnLimiter->setTooltip (TRANS("Activate limiter on output"));
    btnLimiter->setButtonText (TRANS("Limiter"));
    btnLimiter->setClickingTogglesState (true);
    btnLimiter->setColour(TextButton::buttonOnColourId, Colours::darkorange);
    //btnLimiter->setToggleState (true, dontSendNotification);
    btnLimiter->onClick = [this] { toggleLimiter(); };
    
    addAndMakeVisible (btnMute = new TextButton ("Mute button"));
    btnMute->setButtonText ("Mute");
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnMute->onClick = [this] { toggleMute(); };

    //setSize (400, 150);
}

MonitoringComponent::~MonitoringComponent()
{
    lblTitle = nullptr;
    sldOutputGain = nullptr;
    btnLimiter = nullptr;
    btnMute = nullptr;
}

void MonitoringComponent::paint (Graphics& g)
{
    g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), 10.000f);
}

void MonitoringComponent::resized()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (6_fr), Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (sldOutputGain),
                            GridItem (btnLimiter).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f)),
                            GridItem (btnMute).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f))
                        });

    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}

void MonitoringComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldOutputGain)
    {
    }
}

void MonitoringComponent::toggleLimiter ()
{
    if (btnLimiter->getToggleState())
    {
        // TODO - notify audio engine that this source component should have it's limiter turned on
    }
    else
    {
        // TODO - notify audio engine that this source component should have it's limiter turned off
    }
}

void MonitoringComponent::toggleMute ()
{
    if (btnMute->getToggleState())
    {
        // TODO - notify audio engine that this source component should be muted
    }
    else
    {
        // TODO - notify audio engine that this source component should be unmuted
    }
}
