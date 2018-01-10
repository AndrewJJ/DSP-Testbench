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

// TODO - consider putting this enum into a separate file "Definitions.h" so it can also be used by the audio engine
enum Waveforms
{
    sine = 1,
    saw,
    square,
    impulse,
    step,
    whiteNoise,
    pinkNoise
};

class SynthesisTab : public Component, public Slider::Listener
{
public:
    SynthesisTab();
    ~SynthesisTab();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:
    ScopedPointer<ComboBox> cmbWaveform;
    ScopedPointer<Slider> sldStartFrequency;
    ScopedPointer<Slider> sldEndFrequency;
    ScopedPointer<Slider> sldSweepDuration;
    ScopedPointer<TextButton> btnSweepEnabled;
    ScopedPointer<TextButton> btnSweepReset;

    void updateWaveform();
    void updateSweepEnablement();
    void resetSweep();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesisTab)
};

class SampleTab : public Component, public Button::Listener, public::ComboBox::Listener
{
public:
    SampleTab();
    ~SampleTab();

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

private:
    ScopedPointer<ComboBox> cmbSample;
    ScopedPointer<TextButton> btnLoopEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTab)
};

class WaveTab : public Component, public Button::Listener
{
public:
    WaveTab();
    ~WaveTab();

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;

private:
    ScopedPointer<TextButton> btnPlay;
    ScopedPointer<TextButton> btnPause;
    ScopedPointer<TextButton> btnLoopEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTab)
};

class AudioTab : public Component
{
public:
    AudioTab();
    ~AudioTab();

    void paint (Graphics& g) override;
    void resized() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTab)
};

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SourceComponent  : public Component,
                         public Button::Listener,
                         public Slider::Listener
{
public:
    //==============================================================================
    SourceComponent (String sourceId);
    ~SourceComponent();

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
    ScopedPointer<TabbedComponent> tabbedComponent;
    ScopedPointer<TextButton> btnMuteSource;
    ScopedPointer<Slider> sldInputGain;
    ScopedPointer<Label> lblSource;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
