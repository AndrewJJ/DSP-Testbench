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
class GuiComponent  : public Component,
                      public Button::Listener,
                      public ComboBox::Listener,
                      public Slider::Listener
{
public:
    //==============================================================================
    GuiComponent ();
    ~GuiComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<ToggleButton> btnMuteProcessorB;
    ScopedPointer<ComboBox> cmbInputSelection;
    ScopedPointer<Label> lblInput;
    ScopedPointer<Component> inputControls;
    ScopedPointer<Slider> sldInputGain;
    ScopedPointer<Label> lblInputGain;
    ScopedPointer<Component> routingPanel;
    ScopedPointer<Component> processorControlsA;
    ScopedPointer<Component> processorControlsB;
    ScopedPointer<Component> loggingPanel;
    ScopedPointer<Component> fftScope;
    ScopedPointer<Component> levelMeters;
    ScopedPointer<Component> phaseScope;
    ScopedPointer<Slider> sldOutputGain;
    ScopedPointer<Label> lblInputGain2;
    ScopedPointer<ToggleButton> btnMuteProcessorA;
    ScopedPointer<ToggleButton> btnMuteLeft;
    ScopedPointer<ToggleButton> btnMuteLeft2;
    ScopedPointer<ToggleButton> btnOutputLimiter;
    ScopedPointer<TextButton> btnOpenLog;
    ScopedPointer<TextButton> btnEmptyLog;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuiComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
