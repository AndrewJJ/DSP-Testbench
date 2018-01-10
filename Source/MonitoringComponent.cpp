/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.2.0

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MonitoringComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MonitoringComponent::MonitoringComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (sldOutputGain = new Slider ("new slider"));
    sldOutputGain->setTooltip (TRANS("Allows gain adjustment of the output to your audio device"));
    sldOutputGain->setRange (-100, 0, 0.1);
    sldOutputGain->setSliderStyle (Slider::LinearHorizontal);
    sldOutputGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldOutputGain->addListener (this);

    addAndMakeVisible (lblGain = new Label ("new label",
                                            TRANS("Gain")));
    lblGain->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblGain->setJustificationType (Justification::centredLeft);
    lblGain->setEditable (false, false, false);
    lblGain->setColour (TextEditor::textColourId, Colours::black);
    lblGain->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btnLimiter = new ToggleButton ("new toggle button"));
    btnLimiter->setTooltip (TRANS("Activate limiter on output"));
    btnLimiter->setButtonText (TRANS("Limiter"));
    btnLimiter->addListener (this);
    btnLimiter->setToggleState (true, dontSendNotification);

    addAndMakeVisible (btnMuteLeft = new TextButton ("new button"));
    btnMuteLeft->setButtonText (TRANS("Mute L"));
    btnMuteLeft->addListener (this);

    addAndMakeVisible (btnMuteRight = new TextButton ("new button"));
    btnMuteRight->setButtonText (TRANS("Mute R"));
    btnMuteRight->addListener (this);

    addAndMakeVisible (lblMonitoring = new Label ("new label",
                                                  TRANS("Monitoring")));
    lblMonitoring->setFont (Font (25.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblMonitoring->setJustificationType (Justification::centredLeft);
    lblMonitoring->setEditable (false, false, false);
    lblMonitoring->setColour (TextEditor::textColourId, Colours::black);
    lblMonitoring->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 150);


    //[Constructor] You can add your own custom stuff here..
    btnMuteLeft->setClickingTogglesState (true);
    btnMuteLeft->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnMuteRight->setClickingTogglesState (true);
    btnMuteRight->setColour(TextButton::buttonOnColourId, Colours::darkred);
    //[/Constructor]
}

MonitoringComponent::~MonitoringComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    sldOutputGain = nullptr;
    lblGain = nullptr;
    btnLimiter = nullptr;
    btnMuteLeft = nullptr;
    btnMuteRight = nullptr;
    lblMonitoring = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MonitoringComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    {
        float x = static_cast<float> (proportionOfWidth (0.0000f)), y = static_cast<float> (proportionOfHeight (0.0000f)), width = static_cast<float> (proportionOfWidth (1.0000f)), height = static_cast<float> (proportionOfHeight (1.0000f));
        Colour fillColour = Colour (0x304b0092);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRoundedRectangle (x, y, width, height, 10.000f);
    }

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MonitoringComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    sldOutputGain->setBounds (proportionOfWidth (0.2337f), proportionOfHeight (0.4071f), proportionOfWidth (0.7396f), proportionOfHeight (0.1818f));
    lblGain->setBounds (proportionOfWidth (0.0621f), proportionOfHeight (0.4071f), proportionOfWidth (0.1538f), proportionOfHeight (0.1818f));
    btnLimiter->setBounds (proportionOfWidth (0.6391f), proportionOfHeight (0.6917f), proportionOfWidth (0.2012f), proportionOfHeight (0.1581f));
    btnMuteLeft->setBounds (proportionOfWidth (0.2396f), proportionOfHeight (0.6917f), proportionOfWidth (0.1391f), proportionOfHeight (0.1581f));
    btnMuteRight->setBounds (proportionOfWidth (0.4201f), proportionOfHeight (0.6917f), proportionOfWidth (0.1391f), proportionOfHeight (0.1581f));
    lblMonitoring->setBounds (proportionOfWidth (0.0444f), proportionOfHeight (0.0672f), proportionOfWidth (0.3314f), proportionOfHeight (0.2016f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MonitoringComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sldOutputGain)
    {
        //[UserSliderCode_sldOutputGain] -- add your slider handling code here..
        //[/UserSliderCode_sldOutputGain]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void MonitoringComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btnLimiter)
    {
        //[UserButtonCode_btnLimiter] -- add your button handler code here..
        //[/UserButtonCode_btnLimiter]
    }
    else if (buttonThatWasClicked == btnMuteLeft)
    {
        //[UserButtonCode_btnMuteLeft] -- add your button handler code here..
        //[/UserButtonCode_btnMuteLeft]
    }
    else if (buttonThatWasClicked == btnMuteRight)
    {
        //[UserButtonCode_btnMuteRight] -- add your button handler code here..
        //[/UserButtonCode_btnMuteRight]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MonitoringComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="400" initialHeight="150">
  <BACKGROUND backgroundColour="ff323e44">
    <ROUNDRECT pos="0% 0% 100% 100%" cornerSize="10" fill="solid: 304b0092"
               hasStroke="0"/>
  </BACKGROUND>
  <SLIDER name="new slider" id="714beded01ec88dd" memberName="sldOutputGain"
          virtualName="" explicitFocusOrder="0" pos="23.373% 40.441% 73.964% 18.382%"
          tooltip="Allows gain adjustment of the output to your audio device"
          min="-100" max="0" int="0.10000000000000000555" style="LinearHorizontal"
          textBoxPos="TextBoxRight" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1" needsCallback="1"/>
  <LABEL name="new label" id="687970d9a109fd25" memberName="lblGain" virtualName=""
         explicitFocusOrder="0" pos="6.213% 40.441% 15.385% 18.382%" edTextCol="ff000000"
         edBkgCol="0" labelText="Gain" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         kerning="0" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="bc1c74cee641025c" memberName="btnLimiter"
                virtualName="" explicitFocusOrder="0" pos="63.905% 69.118% 20.118% 16.176%"
                tooltip="Activate limiter on output" buttonText="Limiter" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="1"/>
  <TEXTBUTTON name="new button" id="b4fe3327a8c0e1b1" memberName="btnMuteLeft"
              virtualName="" explicitFocusOrder="0" pos="23.964% 69.118% 13.905% 16.176%"
              buttonText="Mute L" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="new button" id="d93e22679e4d5533" memberName="btnMuteRight"
              virtualName="" explicitFocusOrder="0" pos="42.012% 69.118% 13.905% 16.176%"
              buttonText="Mute R" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="ab275a705d086288" memberName="lblMonitoring"
         virtualName="" explicitFocusOrder="0" pos="4.438% 6.618% 33.136% 19.853%"
         edTextCol="ff000000" edBkgCol="0" labelText="Monitoring" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="25" kerning="0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
