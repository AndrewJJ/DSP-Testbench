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

    addAndMakeVisible (sldGain = new Slider ("Output gain slider"));
    sldGain->setTooltip (TRANS("Allows gain adjustment of the output to your audio device"));
    sldGain->setRange (-100, 0, 0.1);
    sldGain->setSliderStyle (Slider::LinearHorizontal);
    sldGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldGain->addListener (this);

    addAndMakeVisible (btnLimiter = new TextButton ("Limiter button"));
    btnLimiter->setTooltip (TRANS("Activate limiter on output"));
    btnLimiter->setButtonText (TRANS("Limiter"));
    btnLimiter->setClickingTogglesState (true);
    btnLimiter->setColour(TextButton::buttonOnColourId, Colours::darkorange);
    //btnLimiter->setToggleState (true, dontSendNotification);
    btnLimiter->onClick = [this] {
        // TODO
    };
    
    addAndMakeVisible (btnMute = new TextButton ("Mute button"));
    btnMute->setButtonText ("Mute");
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnMute->onClick = [this] { 
        // TODO - notify audio engine of change to mute
    };

    //setSize (400, 150);
}
MonitoringComponent::~MonitoringComponent()
{
    lblTitle = nullptr;
    sldGain = nullptr;
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
                            GridItem (sldGain),
                            GridItem (btnLimiter).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f)),
                            GridItem (btnMute).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f))
                        });

    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void MonitoringComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldGain)
    {
        // TODO - notify audio engine of change to gain
    }
}
double MonitoringComponent::getGain () const
{
    return sldGain->getValue();
}
bool MonitoringComponent::isLimited () const
{
    return btnLimiter->getToggleState();
}
bool MonitoringComponent::isMuted () const
{
    return btnMute->getToggleState();
}