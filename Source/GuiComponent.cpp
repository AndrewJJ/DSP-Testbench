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

    addAndMakeVisible (srcComponentA = new SourceComponent ("A"));
    addAndMakeVisible (srcComponentB = new SourceComponent ("B"));
    addAndMakeVisible (procComponentA = new ProcessorComponent ("A"));
    addAndMakeVisible (procComponentB = new ProcessorComponent ("B"));
    addAndMakeVisible (monitoringComponent = new MonitoringComponent());

    //[UserPreSize]
    //[/UserPreSize]

    setSize (1024, 768);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

GuiComponent::~GuiComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    srcComponentA = nullptr;
    srcComponentB = nullptr;
    procComponentA = nullptr;
    procComponentB = nullptr;
    monitoringComponent = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void GuiComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void GuiComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    srcComponentA->setBounds (proportionOfWidth (0.0000f), proportionOfHeight (0.0000f), proportionOfWidth (0.3301f), proportionOfHeight (0.3294f));
    srcComponentB->setBounds (proportionOfWidth (0.0000f), proportionOfHeight (0.3346f), proportionOfWidth (0.3301f), proportionOfHeight (0.3294f));
    procComponentA->setBounds (proportionOfWidth (0.3350f), proportionOfHeight (0.0000f), proportionOfWidth (0.3301f), proportionOfHeight (0.3294f));
    procComponentB->setBounds (proportionOfWidth (0.3350f), proportionOfHeight (0.3346f), proportionOfWidth (0.3301f), proportionOfHeight (0.3294f));
    monitoringComponent->setBounds (proportionOfWidth (0.6699f), proportionOfHeight (0.0000f), proportionOfWidth (0.3301f), proportionOfHeight (0.1771f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
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
                 fixedSize="1" initialWidth="1024" initialHeight="768">
  <BACKGROUND backgroundColour="ff323e44"/>
  <JUCERCOMP name="Source A" id="efb7a0dc57eb4c34" memberName="srcComponentA"
             virtualName="SourceComponent" explicitFocusOrder="0" pos="0% 0% 33.008% 32.943%"
             sourceFile="SourceComponent.cpp" constructorParams="&quot;A&quot;"/>
  <JUCERCOMP name="Source B" id="89f150fb6fb15f93" memberName="srcComponentB"
             virtualName="SourceComponent" explicitFocusOrder="0" pos="0% 33.464% 33.008% 32.943%"
             sourceFile="SourceComponent.cpp" constructorParams="&quot;B&quot;"/>
  <JUCERCOMP name="Processor A" id="6031eb5fa84b4b75" memberName="procComponentA"
             virtualName="ProcessorComponent" explicitFocusOrder="0" pos="33.496% 0% 33.008% 32.943%"
             sourceFile="ProcessorComponent.cpp" constructorParams="&quot;A&quot;"/>
  <JUCERCOMP name="Processor B" id="cbfe16dd113e17e9" memberName="procComponentB"
             virtualName="ProcessorComponent" explicitFocusOrder="0" pos="33.496% 33.464% 33.008% 32.943%"
             sourceFile="ProcessorComponent.cpp" constructorParams="&quot;B&quot;"/>
  <JUCERCOMP name="Monitoring" id="97272d590bec0349" memberName="monitoringComponent"
             virtualName="MonitoringComponent" explicitFocusOrder="0" pos="66.992% 0% 33.008% 17.708%"
             sourceFile="MonitoringComponent.cpp" constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
