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

#include "GuiComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
GuiComponent::GuiComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (btnMuteProcessorB = new ToggleButton ("new toggle button"));
    btnMuteProcessorB->setTooltip (TRANS("Mute processor B"));
    btnMuteProcessorB->setButtonText (TRANS("Mute"));
    btnMuteProcessorB->addListener (this);

    btnMuteProcessorB->setBounds (1240, 32, 72, 24);

    addAndMakeVisible (cmbInputSelection = new ComboBox ("new combo box"));
    cmbInputSelection->setTooltip (TRANS("Select the type of signal to use as input to the processor(s) being tested."));
    cmbInputSelection->setEditableText (false);
    cmbInputSelection->setJustificationType (Justification::centredLeft);
    cmbInputSelection->setTextWhenNothingSelected (String());
    cmbInputSelection->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    cmbInputSelection->addItem (TRANS("Synthesis"), 1);
    cmbInputSelection->addItem (TRANS("Sample"), 2);
    cmbInputSelection->addItem (TRANS("Wave File"), 3);
    cmbInputSelection->addItem (TRANS("Audio Device"), 4);
    cmbInputSelection->addListener (this);

    cmbInputSelection->setBounds (120, 24, 200, 24);

    addAndMakeVisible (lblInput = new Label ("new label",
                                             TRANS("Input source")));
    lblInput->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblInput->setJustificationType (Justification::centredLeft);
    lblInput->setEditable (false, false, false);
    lblInput->setColour (TextEditor::textColourId, Colours::black);
    lblInput->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    lblInput->setBounds (16, 24, 96, 24);

    addAndMakeVisible (inputControls = new Component());
    inputControls->setName ("new component");

    inputControls->setBounds (24, 64, 296, 192);

    addAndMakeVisible (sldInputGain = new Slider ("new slider"));
    sldInputGain->setTooltip (TRANS("Allows gain adjustment of the selected input source"));
    sldInputGain->setRange (-100, 50, 0);
    sldInputGain->setSliderStyle (Slider::LinearHorizontal);
    sldInputGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldInputGain->addListener (this);

    sldInputGain->setBounds (104, 272, 216, 24);

    addAndMakeVisible (lblInputGain = new Label ("new label",
                                                 TRANS("Input gain")));
    lblInputGain->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblInputGain->setJustificationType (Justification::centredLeft);
    lblInputGain->setEditable (false, false, false);
    lblInputGain->setColour (TextEditor::textColourId, Colours::black);
    lblInputGain->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    lblInputGain->setBounds (16, 272, 80, 24);

    addAndMakeVisible (routingPanel = new Component());
    routingPanel->setName ("new component");

    routingPanel->setBounds (352, 64, 296, 192);

    addAndMakeVisible (processorControlsA = new Component());
    processorControlsA->setName ("new component");

    processorControlsA->setBounds (680, 64, 296, 192);

    addAndMakeVisible (processorControlsB = new Component());
    processorControlsB->setName ("new component");

    processorControlsB->setBounds (1008, 64, 296, 192);

    addAndMakeVisible (loggingPanel = new Component());
    loggingPanel->setName ("new component");

    loggingPanel->setBounds (24, 992, 1288, 192);

    addAndMakeVisible (fftScope = new Component());
    fftScope->setName ("new component");

    fftScope->setBounds (24, 328, 1288, 640);

    addAndMakeVisible (levelMeters = new Component());
    levelMeters->setName ("new component");

    levelMeters->setBounds (1336, 336, 296, 416);

    addAndMakeVisible (phaseScope = new Component());
    phaseScope->setName ("new component");

    phaseScope->setBounds (1336, 776, 296, 192);

    addAndMakeVisible (sldOutputGain = new Slider ("new slider"));
    sldOutputGain->setTooltip (TRANS("Allows gain adjustment of the output to your audio device"));
    sldOutputGain->setRange (-100, 0, 0);
    sldOutputGain->setSliderStyle (Slider::LinearHorizontal);
    sldOutputGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldOutputGain->addListener (this);

    sldOutputGain->setBounds (1419, 73, 216, 24);

    addAndMakeVisible (lblInputGain2 = new Label ("new label",
                                                  TRANS("Output gain")));
    lblInputGain2->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblInputGain2->setJustificationType (Justification::centredLeft);
    lblInputGain2->setEditable (false, false, false);
    lblInputGain2->setColour (TextEditor::textColourId, Colours::black);
    lblInputGain2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    lblInputGain2->setBounds (1331, 73, 80, 24);

    addAndMakeVisible (btnMuteProcessorA = new ToggleButton ("new toggle button"));
    btnMuteProcessorA->setTooltip (TRANS("Mute processor A"));
    btnMuteProcessorA->setButtonText (TRANS("Mute"));
    btnMuteProcessorA->addListener (this);

    btnMuteProcessorA->setBounds (912, 32, 72, 24);

    addAndMakeVisible (btnMuteLeft = new ToggleButton ("new toggle button"));
    btnMuteLeft->setTooltip (TRANS("Mute left audio channel"));
    btnMuteLeft->setButtonText (TRANS("Mute left"));
    btnMuteLeft->addListener (this);

    btnMuteLeft->setBounds (1336, 104, 104, 24);

    addAndMakeVisible (btnMuteLeft2 = new ToggleButton ("new toggle button"));
    btnMuteLeft2->setTooltip (TRANS("Mute right audio channel"));
    btnMuteLeft2->setButtonText (TRANS("Mute right"));
    btnMuteLeft2->addListener (this);

    btnMuteLeft2->setBounds (1432, 104, 104, 24);

    addAndMakeVisible (btnOutputLimiter = new ToggleButton ("new toggle button"));
    btnOutputLimiter->setTooltip (TRANS("Activate limiter on output"));
    btnOutputLimiter->setButtonText (TRANS("Limiter"));
    btnOutputLimiter->addListener (this);
    btnOutputLimiter->setToggleState (true, dontSendNotification);

    btnOutputLimiter->setBounds (1552, 104, 80, 24);

    addAndMakeVisible (btnOpenLog = new TextButton ("new button"));
    btnOpenLog->setTooltip (TRANS("Opens the log file in a text editor"));
    btnOpenLog->setButtonText (TRANS("Open log file"));
    btnOpenLog->addListener (this);

    btnOpenLog->setBounds (1336, 1000, 296, 24);

    addAndMakeVisible (btnEmptyLog = new TextButton ("new button"));
    btnEmptyLog->setTooltip (TRANS("Delete all entries in log file"));
    btnEmptyLog->setButtonText (TRANS("Clear log file"));
    btnEmptyLog->addListener (this);

    btnEmptyLog->setBounds (1336, 1040, 296, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

GuiComponent::~GuiComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    btnMuteProcessorB = nullptr;
    cmbInputSelection = nullptr;
    lblInput = nullptr;
    inputControls = nullptr;
    sldInputGain = nullptr;
    lblInputGain = nullptr;
    routingPanel = nullptr;
    processorControlsA = nullptr;
    processorControlsB = nullptr;
    loggingPanel = nullptr;
    fftScope = nullptr;
    levelMeters = nullptr;
    phaseScope = nullptr;
    sldOutputGain = nullptr;
    lblInputGain2 = nullptr;
    btnMuteProcessorA = nullptr;
    btnMuteLeft = nullptr;
    btnMuteLeft2 = nullptr;
    btnOutputLimiter = nullptr;
    btnOpenLog = nullptr;
    btnEmptyLog = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void GuiComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    {
        int x = 684, y = 28, width = 292, height = 30;
        String text (TRANS("Processor A"));
        Colour fillColour = Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    Justification::centred, true);
    }

    {
        int x = 1012, y = 28, width = 292, height = 30;
        String text (TRANS("Processor B"));
        Colour fillColour = Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    Justification::centred, true);
    }

    {
        int x = 356, y = 28, width = 292, height = 30;
        String text (TRANS("Routing"));
        Colour fillColour = Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    Justification::centred, true);
    }

    {
        int x = 1340, y = 28, width = 292, height = 30;
        String text (TRANS("Output"));
        Colour fillColour = Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    Justification::centred, true);
    }

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void GuiComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void GuiComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btnMuteProcessorB)
    {
        //[UserButtonCode_btnMuteProcessorB] -- add your button handler code here..
        //[/UserButtonCode_btnMuteProcessorB]
    }
    else if (buttonThatWasClicked == btnMuteProcessorA)
    {
        //[UserButtonCode_btnMuteProcessorA] -- add your button handler code here..
        //[/UserButtonCode_btnMuteProcessorA]
    }
    else if (buttonThatWasClicked == btnMuteLeft)
    {
        //[UserButtonCode_btnMuteLeft] -- add your button handler code here..
        //[/UserButtonCode_btnMuteLeft]
    }
    else if (buttonThatWasClicked == btnMuteLeft2)
    {
        //[UserButtonCode_btnMuteLeft2] -- add your button handler code here..
        //[/UserButtonCode_btnMuteLeft2]
    }
    else if (buttonThatWasClicked == btnOutputLimiter)
    {
        //[UserButtonCode_btnOutputLimiter] -- add your button handler code here..
        //[/UserButtonCode_btnOutputLimiter]
    }
    else if (buttonThatWasClicked == btnOpenLog)
    {
        //[UserButtonCode_btnOpenLog] -- add your button handler code here..
        //[/UserButtonCode_btnOpenLog]
    }
    else if (buttonThatWasClicked == btnEmptyLog)
    {
        //[UserButtonCode_btnEmptyLog] -- add your button handler code here..
        //[/UserButtonCode_btnEmptyLog]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void GuiComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == cmbInputSelection)
    {
        //[UserComboBoxCode_cmbInputSelection] -- add your combo box handling code here..
        //[/UserComboBoxCode_cmbInputSelection]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void GuiComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sldInputGain)
    {
        //[UserSliderCode_sldInputGain] -- add your slider handling code here..
        //[/UserSliderCode_sldInputGain]
    }
    else if (sliderThatWasMoved == sldOutputGain)
    {
        //[UserSliderCode_sldOutputGain] -- add your slider handling code here..
        //[/UserSliderCode_sldOutputGain]
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

<JUCER_COMPONENT documentType="Component" className="GuiComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff323e44">
    <TEXT pos="684 28 292 30" fill="solid: ffffffff" hasStroke="0" text="Processor A"
          fontname="Default font" fontsize="15" kerning="0" bold="0" italic="0"
          justification="36"/>
    <TEXT pos="1012 28 292 30" fill="solid: ffffffff" hasStroke="0" text="Processor B"
          fontname="Default font" fontsize="15" kerning="0" bold="0" italic="0"
          justification="36"/>
    <TEXT pos="356 28 292 30" fill="solid: ffffffff" hasStroke="0" text="Routing"
          fontname="Default font" fontsize="15" kerning="0" bold="0" italic="0"
          justification="36"/>
    <TEXT pos="1340 28 292 30" fill="solid: ffffffff" hasStroke="0" text="Output"
          fontname="Default font" fontsize="15" kerning="0" bold="0" italic="0"
          justification="36"/>
  </BACKGROUND>
  <TOGGLEBUTTON name="new toggle button" id="235769b59cdbee66" memberName="btnMuteProcessorB"
                virtualName="" explicitFocusOrder="0" pos="1240 32 72 24" tooltip="Mute processor B"
                buttonText="Mute" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <COMBOBOX name="new combo box" id="a8c0c0337d853382" memberName="cmbInputSelection"
            virtualName="" explicitFocusOrder="0" pos="120 24 200 24" tooltip="Select the type of signal to use as input to the processor(s) being tested."
            editable="0" layout="33" items="Synthesis&#10;Sample&#10;Wave File&#10;Audio Device"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="b4dbf7255f7bbfe2" memberName="lblInput"
         virtualName="" explicitFocusOrder="0" pos="16 24 96 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Input source" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" kerning="0" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="new component" id="d15bc16956729317" memberName="inputControls"
                    virtualName="" explicitFocusOrder="0" pos="24 64 296 192" class="Component"
                    params=""/>
  <SLIDER name="new slider" id="c31f97dab0d58c74" memberName="sldInputGain"
          virtualName="" explicitFocusOrder="0" pos="104 272 216 24" tooltip="Allows gain adjustment of the selected input source"
          min="-100" max="50" int="0" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="c33bfabdc740b3d7" memberName="lblInputGain"
         virtualName="" explicitFocusOrder="0" pos="16 272 80 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Input gain" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         kerning="0" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="new component" id="85406f8fe1982b01" memberName="routingPanel"
                    virtualName="" explicitFocusOrder="0" pos="352 64 296 192" class="Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="286f2223c09bcfc6" memberName="processorControlsA"
                    virtualName="" explicitFocusOrder="0" pos="680 64 296 192" class="Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="96b59d53f112a558" memberName="processorControlsB"
                    virtualName="" explicitFocusOrder="0" pos="1008 64 296 192" class="Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="785a151f392792f7" memberName="loggingPanel"
                    virtualName="" explicitFocusOrder="0" pos="24 992 1288 192" class="Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="67c398dc6f45a9db" memberName="fftScope"
                    virtualName="" explicitFocusOrder="0" pos="24 328 1288 640" class="Component"
                    params=""/>
  <GENERICCOMPONENT name="new component" id="4edad9c47c277f2c" memberName="levelMeters"
                    virtualName="" explicitFocusOrder="0" pos="1336 336 296 416"
                    class="Component" params=""/>
  <GENERICCOMPONENT name="new component" id="6c84e79db6fa306f" memberName="phaseScope"
                    virtualName="" explicitFocusOrder="0" pos="1336 776 296 192"
                    class="Component" params=""/>
  <SLIDER name="new slider" id="714beded01ec88dd" memberName="sldOutputGain"
          virtualName="" explicitFocusOrder="0" pos="1419 73 216 24" tooltip="Allows gain adjustment of the output to your audio device"
          min="-100" max="0" int="0" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="687970d9a109fd25" memberName="lblInputGain2"
         virtualName="" explicitFocusOrder="0" pos="1331 73 80 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Output gain" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" kerning="0" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="new toggle button" id="e2b97afd014b460a" memberName="btnMuteProcessorA"
                virtualName="" explicitFocusOrder="0" pos="912 32 72 24" tooltip="Mute processor A"
                buttonText="Mute" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="a4d32aa31f99cd6a" memberName="btnMuteLeft"
                virtualName="" explicitFocusOrder="0" pos="1336 104 104 24" tooltip="Mute left audio channel"
                buttonText="Mute left" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="a314323bddb1578d" memberName="btnMuteLeft2"
                virtualName="" explicitFocusOrder="0" pos="1432 104 104 24" tooltip="Mute right audio channel"
                buttonText="Mute right" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="bc1c74cee641025c" memberName="btnOutputLimiter"
                virtualName="" explicitFocusOrder="0" pos="1552 104 80 24" tooltip="Activate limiter on output"
                buttonText="Limiter" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="1"/>
  <TEXTBUTTON name="new button" id="d0ffb019bda180ed" memberName="btnOpenLog"
              virtualName="" explicitFocusOrder="0" pos="1336 1000 296 24"
              tooltip="Opens the log file in a text editor" buttonText="Open log file"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="new button" id="842708023e158b6b" memberName="btnEmptyLog"
              virtualName="" explicitFocusOrder="0" pos="1336 1040 296 24"
              tooltip="Delete all entries in log file" buttonText="Clear log file"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
