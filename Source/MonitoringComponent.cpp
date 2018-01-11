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
    addAndMakeVisible (lblMonitoring = new Label ("Monitoring label", TRANS("Monitoring")));
    lblMonitoring->setFont (Font (15.00f, Font::bold));
    lblMonitoring->setJustificationType (Justification::topLeft);
    lblMonitoring->setEditable (false, false, false);
    lblMonitoring->setColour (TextEditor::textColourId, Colours::black);
    lblMonitoring->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (lblGain = new Label ("Gain label", TRANS("Gain")));
    lblGain->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblGain->setJustificationType (Justification::centredLeft);
    lblGain->setEditable (false, false, false);
    lblGain->setColour (TextEditor::textColourId, Colours::black);
    lblGain->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sldOutputGain = new Slider ("Output gain slider"));
    sldOutputGain->setTooltip (TRANS("Allows gain adjustment of the output to your audio device"));
    sldOutputGain->setRange (-100, 0, 0.1);
    sldOutputGain->setSliderStyle (Slider::LinearHorizontal);
    sldOutputGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldOutputGain->addListener (this);

    addAndMakeVisible (btnLimiter = new ToggleButton ("Limiter toggle button"));
    btnLimiter->setTooltip (TRANS("Activate limiter on output"));
    btnLimiter->setButtonText (TRANS("Limiter"));
    btnLimiter->addListener (this);
    btnLimiter->setToggleState (true, dontSendNotification);

    addAndMakeVisible (btnMuteLeft = new TextButton ("Mute left button"));
    btnMuteLeft->setButtonText ("Mute L");
    btnMuteLeft->addListener (this);
    btnMuteLeft->setClickingTogglesState (true);
    btnMuteLeft->setColour(TextButton::buttonOnColourId, Colours::darkred);

    addAndMakeVisible (btnMuteRight = new TextButton ("Mute right button"));
    btnMuteRight->setButtonText ("Mute R");
    btnMuteRight->addListener (this);
    btnMuteRight->setClickingTogglesState (true);
    btnMuteRight->setColour(TextButton::buttonOnColourId, Colours::darkred);

    //setSize (400, 150);
}

MonitoringComponent::~MonitoringComponent()
{
    lblMonitoring = nullptr;
    lblGain = nullptr;
    sldOutputGain = nullptr;
    btnLimiter = nullptr;
    btnMuteLeft = nullptr;
    btnMuteRight = nullptr;
}

void MonitoringComponent::paint (Graphics& g)
{
    g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), 10.000f);
}

void MonitoringComponent::resized()
{
    lblMonitoring->setBounds (proportionOfWidth (0.0444f), proportionOfHeight (0.0672f), proportionOfWidth (0.3314f), proportionOfHeight (0.2016f));
    lblGain->setBounds (proportionOfWidth (0.0621f), proportionOfHeight (0.4071f), proportionOfWidth (0.1538f), proportionOfHeight (0.1818f));
    sldOutputGain->setBounds (proportionOfWidth (0.2337f), proportionOfHeight (0.4071f), proportionOfWidth (0.7396f), proportionOfHeight (0.1818f));
    btnLimiter->setBounds (proportionOfWidth (0.6391f), proportionOfHeight (0.6917f), proportionOfWidth (0.2012f), proportionOfHeight (0.1581f));
    btnMuteLeft->setBounds (proportionOfWidth (0.2396f), proportionOfHeight (0.6917f), proportionOfWidth (0.1391f), proportionOfHeight (0.1581f));
    btnMuteRight->setBounds (proportionOfWidth (0.4201f), proportionOfHeight (0.6917f), proportionOfWidth (0.1391f), proportionOfHeight (0.1581f));
}

void MonitoringComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldOutputGain)
    {
    }
}

void MonitoringComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == btnLimiter)
    {
    }
    else if (buttonThatWasClicked == btnMuteLeft)
    {
    }
    else if (buttonThatWasClicked == btnMuteRight)
    {
    }
}