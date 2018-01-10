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

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ProcessorComponent  : public Component,
                            public Button::Listener,
                            public Slider::Listener
{
public:
    //==============================================================================
    ProcessorComponent (String processorId);
    ~ProcessorComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<ToggleButton> btnSourceA;
    ScopedPointer<ToggleButton> btnSourceB;
    ScopedPointer<Slider> slider1;
    ScopedPointer<Label> lblParameter1;
    ScopedPointer<Slider> slider2;
    ScopedPointer<Label> lblParameter2;
    ScopedPointer<Slider> slider3;
    ScopedPointer<Label> lblParameter3;
    ScopedPointer<Slider> slider4;
    ScopedPointer<Label> lblParameter4;
    ScopedPointer<Label> lblProcessor;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TextButton> btnDisable;
    ScopedPointer<Label> lblParameter5;
    ScopedPointer<Slider> slider5;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
