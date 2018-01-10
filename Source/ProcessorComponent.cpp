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

#include "ProcessorComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ProcessorComponent::ProcessorComponent (String processorId)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (btnSourceA = new ToggleButton ("new toggle button"));
    btnSourceA->setTooltip (TRANS("Process input from source A"));
    btnSourceA->setButtonText (TRANS("Source A"));
    btnSourceA->addListener (this);

    addAndMakeVisible (btnSourceB = new ToggleButton ("new toggle button"));
    btnSourceB->setTooltip (TRANS("Process input from source B"));
    btnSourceB->setButtonText (TRANS("Source B"));
    btnSourceB->addListener (this);

    addAndMakeVisible (slider1 = new Slider ("new slider"));
    slider1->setRange (0, 10, 0);
    slider1->setSliderStyle (Slider::LinearHorizontal);
    slider1->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider1->addListener (this);

    addAndMakeVisible (lblParameter1 = new Label ("new label",
                                                  TRANS("Parameter 1")));
    lblParameter1->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblParameter1->setJustificationType (Justification::centredLeft);
    lblParameter1->setEditable (false, false, false);
    lblParameter1->setColour (TextEditor::textColourId, Colours::black);
    lblParameter1->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider2 = new Slider ("new slider"));
    slider2->setRange (0, 10, 0);
    slider2->setSliderStyle (Slider::LinearHorizontal);
    slider2->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider2->addListener (this);

    addAndMakeVisible (lblParameter2 = new Label ("new label",
                                                  TRANS("Parameter 2")));
    lblParameter2->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblParameter2->setJustificationType (Justification::centredLeft);
    lblParameter2->setEditable (false, false, false);
    lblParameter2->setColour (TextEditor::textColourId, Colours::black);
    lblParameter2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider3 = new Slider ("new slider"));
    slider3->setRange (0, 10, 0);
    slider3->setSliderStyle (Slider::LinearHorizontal);
    slider3->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider3->addListener (this);

    addAndMakeVisible (lblParameter3 = new Label ("new label",
                                                  TRANS("Parameter 3")));
    lblParameter3->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblParameter3->setJustificationType (Justification::centredLeft);
    lblParameter3->setEditable (false, false, false);
    lblParameter3->setColour (TextEditor::textColourId, Colours::black);
    lblParameter3->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider4 = new Slider ("new slider"));
    slider4->setRange (0, 10, 0);
    slider4->setSliderStyle (Slider::LinearHorizontal);
    slider4->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider4->addListener (this);

    addAndMakeVisible (lblParameter4 = new Label ("new label",
                                                  TRANS("Parameter 4\n")));
    lblParameter4->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblParameter4->setJustificationType (Justification::centredLeft);
    lblParameter4->setEditable (false, false, false);
    lblParameter4->setColour (TextEditor::textColourId, Colours::black);
    lblParameter4->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (lblProcessor = new Label ("new label",
                                                 TRANS("Processor")));
    lblProcessor->setFont (Font (25.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblProcessor->setJustificationType (Justification::centredLeft);
    lblProcessor->setEditable (false, false, false);
    lblProcessor->setColour (TextEditor::textColourId, Colours::black);
    lblProcessor->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btnMute = new TextButton ("new button"));
    btnMute->setButtonText (TRANS("Mute"));
    btnMute->addListener (this);

    addAndMakeVisible (btnDisable = new TextButton ("new button"));
    btnDisable->setButtonText (TRANS("Disable"));
    btnDisable->addListener (this);

    addAndMakeVisible (lblParameter5 = new Label ("new label",
                                                  TRANS("Parameter 5\n")));
    lblParameter5->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblParameter5->setJustificationType (Justification::centredLeft);
    lblParameter5->setEditable (false, false, false);
    lblParameter5->setColour (TextEditor::textColourId, Colours::black);
    lblParameter5->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider5 = new Slider ("new slider"));
    slider5->setRange (0, 10, 0);
    slider5->setSliderStyle (Slider::LinearHorizontal);
    slider5->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider5->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (400, 300);


    //[Constructor] You can add your own custom stuff here..
    lblProcessor->setText ("Processor " + processorId, dontSendNotification);
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnDisable->setClickingTogglesState (true);
    btnDisable->setColour(TextButton::buttonOnColourId, Colours::darkred);
    //[/Constructor]
}

ProcessorComponent::~ProcessorComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    btnSourceA = nullptr;
    btnSourceB = nullptr;
    slider1 = nullptr;
    lblParameter1 = nullptr;
    slider2 = nullptr;
    lblParameter2 = nullptr;
    slider3 = nullptr;
    lblParameter3 = nullptr;
    slider4 = nullptr;
    lblParameter4 = nullptr;
    lblProcessor = nullptr;
    btnMute = nullptr;
    btnDisable = nullptr;
    lblParameter5 = nullptr;
    slider5 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ProcessorComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    {
        float x = static_cast<float> (proportionOfWidth (0.0000f)), y = static_cast<float> (proportionOfHeight (0.0000f)), width = static_cast<float> (proportionOfWidth (1.0000f)), height = static_cast<float> (proportionOfHeight (1.0000f));
        Colour fillColour = Colour (0x300081ff);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRoundedRectangle (x, y, width, height, 10.000f);
    }

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ProcessorComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    btnSourceA->setBounds (proportionOfWidth (0.0385f), proportionOfHeight (0.2134f), proportionOfWidth (0.2604f), proportionOfHeight (0.0791f));
    btnSourceB->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.2134f), proportionOfWidth (0.2604f), proportionOfHeight (0.0791f));
    slider1->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.3202f), proportionOfWidth (0.6834f), proportionOfHeight (0.1067f));
    lblParameter1->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.3202f), proportionOfWidth (0.2811f), proportionOfHeight (0.1067f));
    slider2->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.4546f), proportionOfWidth (0.6834f), proportionOfHeight (0.1067f));
    lblParameter2->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.4585f), proportionOfWidth (0.2811f), proportionOfHeight (0.1028f));
    slider3->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.5850f), proportionOfWidth (0.6834f), proportionOfHeight (0.1067f));
    lblParameter3->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.5850f), proportionOfWidth (0.2811f), proportionOfHeight (0.1067f));
    slider4->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.7194f), proportionOfWidth (0.6834f), proportionOfHeight (0.1067f));
    lblParameter4->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.7194f), proportionOfWidth (0.2811f), proportionOfHeight (0.1067f));
    lblProcessor->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.0277f), proportionOfWidth (0.3343f), proportionOfHeight (0.1423f));
    btnMute->setBounds (proportionOfWidth (0.8402f), proportionOfHeight (0.0514f), proportionOfWidth (0.1213f), proportionOfHeight (0.0791f));
    btnDisable->setBounds (proportionOfWidth (0.7012f), proportionOfHeight (0.0514f), proportionOfWidth (0.1213f), proportionOfHeight (0.0791f));
    lblParameter5->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.8498f), proportionOfWidth (0.2811f), proportionOfHeight (0.1107f));
    slider5->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.8538f), proportionOfWidth (0.6834f), proportionOfHeight (0.1067f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ProcessorComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btnSourceA)
    {
        //[UserButtonCode_btnSourceA] -- add your button handler code here..
        //[/UserButtonCode_btnSourceA]
    }
    else if (buttonThatWasClicked == btnSourceB)
    {
        //[UserButtonCode_btnSourceB] -- add your button handler code here..
        //[/UserButtonCode_btnSourceB]
    }
    else if (buttonThatWasClicked == btnMute)
    {
        //[UserButtonCode_btnMute] -- add your button handler code here..
        //[/UserButtonCode_btnMute]
    }
    else if (buttonThatWasClicked == btnDisable)
    {
        //[UserButtonCode_btnDisable] -- add your button handler code here..
        //[/UserButtonCode_btnDisable]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void ProcessorComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider1)
    {
        //[UserSliderCode_slider1] -- add your slider handling code here..
        //[/UserSliderCode_slider1]
    }
    else if (sliderThatWasMoved == slider2)
    {
        //[UserSliderCode_slider2] -- add your slider handling code here..
        //[/UserSliderCode_slider2]
    }
    else if (sliderThatWasMoved == slider3)
    {
        //[UserSliderCode_slider3] -- add your slider handling code here..
        //[/UserSliderCode_slider3]
    }
    else if (sliderThatWasMoved == slider4)
    {
        //[UserSliderCode_slider4] -- add your slider handling code here..
        //[/UserSliderCode_slider4]
    }
    else if (sliderThatWasMoved == slider5)
    {
        //[UserSliderCode_slider5] -- add your slider handling code here..
        //[/UserSliderCode_slider5]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ProcessorComponent" componentName=""
                 parentClasses="public Component" constructorParams="String processorId"
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="400" initialHeight="300">
  <BACKGROUND backgroundColour="ff323e44">
    <ROUNDRECT pos="0% 0% 100% 100%" cornerSize="10" fill="solid: 300081ff"
               hasStroke="0"/>
  </BACKGROUND>
  <TOGGLEBUTTON name="new toggle button" id="b80b45e08613b7f8" memberName="btnSourceA"
                virtualName="" explicitFocusOrder="0" pos="3.846% 21.344% 26.036% 7.905%"
                tooltip="Process input from source A" buttonText="Source A" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="fadba663517228d3" memberName="btnSourceB"
                virtualName="" explicitFocusOrder="0" pos="29.882% 21.344% 26.036% 7.905%"
                tooltip="Process input from source B" buttonText="Source B" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <SLIDER name="new slider" id="20564e593a3e7cc8" memberName="slider1"
          virtualName="" explicitFocusOrder="0" pos="29.882% 32.016% 68.343% 10.672%"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="5187a7e9c6403df2" memberName="lblParameter1"
         virtualName="" explicitFocusOrder="0" pos="2.071% 32.016% 28.107% 10.672%"
         edTextCol="ff000000" edBkgCol="0" labelText="Parameter 1" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" kerning="0" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="f8ff1ff84b1e0fec" memberName="slider2"
          virtualName="" explicitFocusOrder="0" pos="29.882% 45.455% 68.343% 10.672%"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="cfc87b5a32b7788" memberName="lblParameter2"
         virtualName="" explicitFocusOrder="0" pos="2.071% 45.85% 28.107% 10.277%"
         edTextCol="ff000000" edBkgCol="0" labelText="Parameter 2" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" kerning="0" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="ce460427107df6d2" memberName="slider3"
          virtualName="" explicitFocusOrder="0" pos="29.882% 58.498% 68.343% 10.672%"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="8207e89a27b5461c" memberName="lblParameter3"
         virtualName="" explicitFocusOrder="0" pos="2.071% 58.498% 28.107% 10.672%"
         edTextCol="ff000000" edBkgCol="0" labelText="Parameter 3" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" kerning="0" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="8ef85c2be972a4d0" memberName="slider4"
          virtualName="" explicitFocusOrder="0" pos="29.882% 71.937% 68.343% 10.672%"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="4986af1b4314ec61" memberName="lblParameter4"
         virtualName="" explicitFocusOrder="0" pos="2.071% 71.937% 28.107% 10.672%"
         edTextCol="ff000000" edBkgCol="0" labelText="Parameter 4&#10;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" kerning="0" bold="0" italic="0"
         justification="33"/>
  <LABEL name="new label" id="ab275a705d086288" memberName="lblProcessor"
         virtualName="" explicitFocusOrder="0" pos="2.071% 2.767% 33.432% 14.229%"
         edTextCol="ff000000" edBkgCol="0" labelText="Processor" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="25" kerning="0" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="new button" id="76c48ad71d5b3e37" memberName="btnMute"
              virtualName="" explicitFocusOrder="0" pos="84.024% 5.138% 12.13% 7.905%"
              buttonText="Mute" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="new button" id="1075646b928b7871" memberName="btnDisable"
              virtualName="" explicitFocusOrder="0" pos="70.118% 5.138% 12.13% 7.905%"
              buttonText="Disable" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="65778ce086a78765" memberName="lblParameter5"
         virtualName="" explicitFocusOrder="0" pos="2.071% 84.98% 28.107% 11.067%"
         edTextCol="ff000000" edBkgCol="0" labelText="Parameter 5&#10;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" kerning="0" bold="0" italic="0"
         justification="33"/>
  <SLIDER name="new slider" id="3368bb4a3217cdc4" memberName="slider5"
          virtualName="" explicitFocusOrder="0" pos="29.882% 85.375% 68.343% 10.672%"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
