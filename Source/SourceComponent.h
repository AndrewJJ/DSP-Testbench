/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Common.h"

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
    ScopedPointer<Slider> sldFrequency;
    ScopedPointer<Slider> sldSweepDuration;
    ScopedPointer<TextButton> btnSweepEnabled;
    ScopedPointer<TextButton> btnSweepReset;

    void updateWaveform();
    void updateSweepEnablement();
    void resetSweep();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesisTab)
};

class SampleTab : public Component
{
public:
    SampleTab();
    ~SampleTab();

    void paint (Graphics& g) override;
    void resized() override;

private:
    ScopedPointer<ComboBox> cmbSample;
    ScopedPointer<TextButton> btnLoopEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTab)
};

class WaveTab : public Component
{
public:
    WaveTab();
    ~WaveTab();

    void paint (Graphics& g) override;
    void resized() override;

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

//==============================================================================

class SourceComponent  : public Component,
                         public Slider::Listener
{
public:

    SourceComponent (String sourceId);
    ~SourceComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:

    void toggleMute();

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldGain;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TabbedComponent> tabbedComponent;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};