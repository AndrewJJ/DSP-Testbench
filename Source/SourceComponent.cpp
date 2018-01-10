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

#include "SourceComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
SynthesisTab::SynthesisTab ()
{
    addAndMakeVisible (cmbWaveform = new ComboBox ("Select Waveform"));
    cmbWaveform->addItem ("Sine", 1);
    cmbWaveform->addItem ("Saw", 2);
    cmbWaveform->addItem ("Square", 3);
    cmbWaveform->addItem ("Impulse", 4);
    cmbWaveform->addItem ("Step", 5);
    cmbWaveform->addItem ("White Noise", 6);
    cmbWaveform->addItem ("Pink Noise", 7);
    cmbWaveform->onChange = [this] { waveformSelectionChange(); };

    addAndMakeVisible (sldStartFrequency = new Slider ("Start Frequency"));
    sldStartFrequency->addListener(this);

    addAndMakeVisible (sldEndFrequency = new Slider ("End Frequency"));
    sldEndFrequency->addListener(this);

    addAndMakeVisible (sldSweepDuration = new Slider ("Sweep Duration"));
    sldSweepDuration->addListener(this);

    addAndMakeVisible (btnSweepEnabled = new TextButton ("Sweep Enabled"));
    btnSweepEnabled->setClickingTogglesState(true);
    btnSweepEnabled->setToggleState(true, dontSendNotification);
    btnSweepEnabled->setColour(TextButton::buttonOnColourId, Colours::green);

    // TODO - set size?
}
SynthesisTab::~SynthesisTab ()
{
    cmbWaveform = nullptr;
    sldStartFrequency = nullptr;
    sldEndFrequency = nullptr;
    sldSweepDuration = nullptr;
    btnSweepEnabled = nullptr;
}
void SynthesisTab::paint (Graphics& g)
{
    // TODO - paint some graphics?
}
void SynthesisTab::resized ()
{
    // TODO - size all components appropriately
    cmbWaveform->setBoundsRelative(0.1f, 0.0f, 0.8f, 0.2f);
    sldStartFrequency->setBoundsRelative(0.1f, 0.25f, 0.8f, .2f);
    sldEndFrequency->setBoundsRelative(0.1f, 0.5f, 0.8f, .2f);
    sldSweepDuration->setBoundsRelative(0.1f, 0.75f, 0.6f, .2f);
    btnSweepEnabled->setBoundsRelative(0.7f, 0.75f, 0.2f, .2f);
}
void SynthesisTab::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == btnSweepEnabled)
    {
    }
}
void SynthesisTab::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldStartFrequency)
    {
    }
    else if (sliderThatWasMoved == sldEndFrequency)
    {
    }
    else if (sliderThatWasMoved == sldSweepDuration)
    {
    }
}
void SynthesisTab::waveformSelectionChange ()
{
    // TODO - finish this implementation
    if (cmbWaveform->getSelectedId() == 1)
        DBG("Sine");
    else
        DBG("TODO :)");

}

SampleTab::SampleTab ()
{
    addAndMakeVisible (cmbSample = new ComboBox ("Select Sample"));
    cmbSample->addItem ("None", 1);
    cmbSample->addListener(this);

    addAndMakeVisible (btnLoopEnabled = new TextButton ("Loop Enabled"));
    btnLoopEnabled->setClickingTogglesState(true);
    btnLoopEnabled->setToggleState(true, dontSendNotification);
    btnLoopEnabled->setColour(TextButton::buttonOnColourId, Colours::green);
}
SampleTab::~SampleTab ()
{
    cmbSample = nullptr;
    btnLoopEnabled = nullptr;
}
void SampleTab::paint (Graphics& g)
{
}
void SampleTab::resized ()
{
    cmbSample->setBoundsRelative(0.1f, 0.2f, 0.8f, 0.2f);
    btnLoopEnabled->setBoundsRelative(0.1f, 0.5f, 0.8f, 0.2f);
}
void SampleTab::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == btnLoopEnabled)
    {
    }
}
void SampleTab::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == cmbSample)
    {
    }
}

WaveTab::WaveTab ()
{
}
WaveTab::~WaveTab ()
{
}
void WaveTab::paint (Graphics& g)
{
    g.setFont(25.0f);
    g.drawFittedText("Wave playing not yet implemented", 0, 0, getWidth(), getHeight(), Justification::Flags::centred, 2);
}
void WaveTab::resized ()
{
}
void WaveTab::buttonClicked (Button* buttonThatWasClicked)
{
}

AudioTab::AudioTab ()
{
}
AudioTab::~AudioTab ()
{
}
void AudioTab::paint (Graphics& g)
{
    g.setFont(25.0f);
    g.drawFittedText("Audio input not yet implemented", 0, 0, getWidth(), getHeight(), Justification::Flags::centred, 2);
}
void AudioTab::resized ()
{
}
//[/MiscUserDefs]

//==============================================================================
SourceComponent::SourceComponent (String sourceId)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (30);
    tabbedComponent->addTab (TRANS("Synthesis"), Colours::lightgrey, new SynthesisTab(), true);
    tabbedComponent->addTab (TRANS("Sample"), Colours::lightgrey, new SampleTab(), true);
    tabbedComponent->addTab (TRANS("Wave File"), Colours::lightgrey, new WaveTab(), true);
    tabbedComponent->addTab (TRANS("Audio In"), Colours::lightgrey, new AudioTab(), true);
    tabbedComponent->setCurrentTabIndex (0);

    addAndMakeVisible (btnMuteSource = new TextButton ("new button"));
    btnMuteSource->setButtonText (TRANS("Mute"));
    btnMuteSource->addListener (this);

    addAndMakeVisible (sldInputGain = new Slider ("new slider"));
    sldInputGain->setTooltip (TRANS("Adjusts the gain of this source"));
    sldInputGain->setRange (-100, 50, 0.1);
    sldInputGain->setSliderStyle (Slider::LinearHorizontal);
    sldInputGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldInputGain->addListener (this);

    addAndMakeVisible (lblSource = new Label ("new label",
                                              TRANS("Source")));
    lblSource->setFont (Font (25.00f, Font::plain).withTypefaceStyle ("Regular"));
    lblSource->setJustificationType (Justification::centredLeft);
    lblSource->setEditable (false, false, false);
    lblSource->setColour (TextEditor::textColourId, Colours::black);
    lblSource->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    lblSource->setText ("Source " + sourceId, dontSendNotification);
    btnMuteSource->setClickingTogglesState (true);
    btnMuteSource->setColour(TextButton::buttonOnColourId, Colours::darkred);
    //[/Constructor]
}

SourceComponent::~SourceComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    tabbedComponent = nullptr;
    btnMuteSource = nullptr;
    sldInputGain = nullptr;
    lblSource = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SourceComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    {
        float x = static_cast<float> (proportionOfWidth (0.0000f)), y = static_cast<float> (proportionOfHeight (0.0000f)), width = static_cast<float> (proportionOfWidth (1.0000f)), height = static_cast<float> (proportionOfHeight (1.0000f));
        Colour fillColour = Colour (0x30a80000);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRoundedRectangle (x, y, width, height, 10.000f);
    }

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SourceComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    tabbedComponent->setBounds (proportionOfWidth (0.0325f), proportionOfHeight (0.9526f) - proportionOfHeight (0.7747f), proportionOfWidth (0.9320f), proportionOfHeight (0.7747f));
    btnMuteSource->setBounds (proportionOfWidth (0.8935f), proportionOfHeight (0.0593f), proportionOfWidth (0.0681f), proportionOfHeight (0.0593f));
    sldInputGain->setBounds (proportionOfWidth (0.3047f), proportionOfHeight (0.0593f), proportionOfWidth (0.5710f), proportionOfHeight (0.0593f));
    lblSource->setBounds (proportionOfWidth (0.0266f), proportionOfHeight (0.0435f), proportionOfWidth (0.2633f), proportionOfHeight (0.0988f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SourceComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btnMuteSource)
    {
        //[UserButtonCode_btnMuteSource] -- add your button handler code here..
        //[/UserButtonCode_btnMuteSource]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void SourceComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sldInputGain)
    {
        //[UserSliderCode_sldInputGain] -- add your slider handling code here..
        //[/UserSliderCode_sldInputGain]
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

<JUCER_COMPONENT documentType="Component" className="SourceComponent" componentName=""
                 parentClasses="public Component" constructorParams="String sourceId"
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff323e44">
    <ROUNDRECT pos="0% 0% 100% 100%" cornerSize="10" fill="solid: 30a80000"
               hasStroke="0"/>
  </BACKGROUND>
  <TABBEDCOMPONENT name="new tabbed component" id="68af494af88f97b5" memberName="tabbedComponent"
                   virtualName="" explicitFocusOrder="0" pos="3.333% 95.25%r 93.167% 77.5%"
                   orientation="top" tabBarDepth="30" initialTab="0">
    <TAB name="Synthesis" colour="ffd3d3d3" useJucerComp="0" contentClassName="SynthesisTab"
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Sample" colour="ffd3d3d3" useJucerComp="0" contentClassName="SampleTab"
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Wave File" colour="ffd3d3d3" useJucerComp="0" contentClassName="WaveTab"
         constructorParams="" jucerComponentFile=""/>
    <TAB name="Audio In" colour="ffd3d3d3" useJucerComp="0" contentClassName="AudioTab"
         constructorParams="" jucerComponentFile=""/>
  </TABBEDCOMPONENT>
  <TEXTBUTTON name="new button" id="f2d5eb87d2ae5bb9" memberName="btnMuteSource"
              virtualName="" explicitFocusOrder="0" pos="89.333% 6% 6.833% 6%"
              buttonText="Mute" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <SLIDER name="new slider" id="c31f97dab0d58c74" memberName="sldInputGain"
          virtualName="" explicitFocusOrder="0" pos="30.5% 6% 57.167% 6%"
          tooltip="Adjusts the gain of this source" min="-100" max="50"
          int="0.10000000000000000555" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="88cdc6c6ed2aeaa7" memberName="lblSource"
         virtualName="" explicitFocusOrder="0" pos="2.667% 4.25% 26.333% 10%"
         edTextCol="ff000000" edBkgCol="0" labelText="Source" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="25" kerning="0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
