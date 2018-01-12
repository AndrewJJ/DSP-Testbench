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

class SynthesisTab : public Component, public Slider::Listener, public dsp::ProcessorBase
{
public:
    SynthesisTab();
    ~SynthesisTab();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset () override;

private:
    ScopedPointer<ComboBox> cmbWaveform;
    ScopedPointer<Slider> sldFrequency;
    ScopedPointer<Slider> sldSweepDuration;
    ScopedPointer<TextButton> btnSweepEnabled;
    ScopedPointer<TextButton> btnSweepReset;

    // TODO - remove unused methods
    void updateWaveform();
    void updateSweepEnablement();
    void resetSweep();

    dsp::Oscillator<float> oscillators[3] =
    {
        // No Approximation
        {[] (float x) { return std::sin (x); }},                   // sine
        {[] (float x) { return x / MathConstants<float>::pi; }},   // saw
        {[] (float x) { return x < 0.0f ? -1.0f : 1.0f; }},        // square

        // Approximated by a wave-table
        //{[] (float x) { return std::sin (x); }, 100},                 // sine
        //{[] (float x) { return x / MathConstants<float>::pi; }, 100}, // saw
        //{[] (float x) { return x < 0.0f ? -1.0f : 1.0f; }, 100}       // square
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesisTab)
};

class SampleTab : public Component, public dsp::ProcessorBase
{
public:
    SampleTab();
    ~SampleTab();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset () override;

private:

    void selectedSampleChanged();
    void loopEnablementToggled();

    ScopedPointer<ComboBox> cmbSample;
    ScopedPointer<TextButton> btnLoopEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTab)
};

class WaveTab : public Component, public dsp::ProcessorBase
{
public:
    WaveTab();
    ~WaveTab();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset () override;

private:
    ScopedPointer<TextButton> btnPlay;
    ScopedPointer<TextButton> btnPause;
    ScopedPointer<TextButton> btnLoopEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTab)
};

class AudioTab : public Component, public dsp::ProcessorBase
{
public:
    AudioTab();
    ~AudioTab();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset () override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTab)
};

//==============================================================================

class SourceComponent  : public Component, public Slider::Listener, public dsp::ProcessorBase
{
public:

    enum Mode
    {
        Synthesis = 0,
        Sample,
        WaveFile,
        AudioIn
    };

    SourceComponent (String sourceId);
    ~SourceComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    double getGain() const;
    bool isMuted() const;
    Mode getMode() const; // TODO - may no longer be needed

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset () override;

    private:

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldGain;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TabbedComponent> tabbedComponent;
    ScopedPointer<SynthesisTab> synthesisTab;
    ScopedPointer<SampleTab> sampleTab;
    ScopedPointer<WaveTab> waveTab;
    ScopedPointer<AudioTab> audioTab;

    // TODO - consider using a combo box instead of a tabbed component so it's obvious only one source type can be used at a time?

    // TODO - implement source gain
    dsp::Gain<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};