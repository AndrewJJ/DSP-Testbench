/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

// Forward declaration
class SourceComponent;

enum Waveform
{
    sine = 1,
    saw,
    square,
    impulse,
    step,
    whiteNoise,
    pinkNoise
};

//String WaveformToString (const Waveform waveForm)
//{
//    switch (waveForm)
//    {
//        case sine: return "Sine"; break;
//        case saw: return "Saw"; break;
//        case Waveform::square: return "Square"; break;
//        case impulse: return "Impulse"; break;
//        case step: return "Step"; break;
//        case whiteNoise: return "White Noise"; break;
//        case pinkNoise: return "Pink Noise"; break;
//    }
//}

enum SweepMode
{
    Wrap= 1,
    Reverse
};

class SynthesisTab : public Component, public dsp::ProcessorBase, public Timer, public Slider::Listener
{
public:
    SynthesisTab();
    ~SynthesisTab();

    void paint (Graphics& g) override;
    void resized() override;

    void performSynch();
    void setOtherSource (SourceComponent* otherSourceComponent);
    void syncAndResetOscillator (const Waveform waveform, const double freq,
                                 const double sweepStart, const double sweepEnd,                                 
                                 const double sweepDuration, SweepMode sweepMode, const bool sweepEnabled);

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset() override;
    
    void timerCallback() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:
    ScopedPointer<ComboBox> cmbWaveform;
    ScopedPointer<Slider> sldFrequency;
    ScopedPointer<Slider> sldSweepDuration;
    ScopedPointer<ComboBox> cmbSweepMode;
    ScopedPointer<TextButton> btnSweepEnabled;
    ScopedPointer<TextButton> btnSweepReset;
    ScopedPointer<TextButton> btnSynchWithOther;

    SourceComponent* otherSource;
    CriticalSection synthesiserCriticalSection;
    double sampleRate;
    uint32 maxBlockSize;
    long numSweepSteps;
    long sweepStepIndex;
    int sweepStepDelta;

    Waveform getSelectedWaveform() const;
    int getSelectedWaveformIndex() const;
    bool isSelectedWaveformOscillatorBased() const;
    SweepMode getSelectedSweepMode() const;
    void waveformUpdated();
    void updateSweepEnablement();
    void resetSweep();
    double getSweepFrequency();
    void calculateNumSweepSteps();

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

    // TODO - do these need to be public?
    double getGain() const;
    bool isInverted() const;
    bool isMuted() const;
    Mode getMode() const; // TODO - may no longer be needed
    
    void setOtherSource (SourceComponent* otherSourceComponent);
    SynthesisTab* getSynthesisTab();

    void prepare (const dsp::ProcessSpec&) override;
    void process (const dsp::ProcessContextReplacing<float>&) override;
    void reset () override;

    private:

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldGain;
    ScopedPointer<TextButton> btnInvert;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TabbedComponent> tabbedComponent;
    ScopedPointer<SynthesisTab> synthesisTab;
    ScopedPointer<SampleTab> sampleTab;
    ScopedPointer<WaveTab> waveTab;
    ScopedPointer<AudioTab> audioTab;

    SourceComponent* otherSource;

    // TODO - document that oscillators aren't synchronised across sources
    // TODO - consider synch to other for whole source
    // TODO - add buttons to invert source signals and output of processors
    // TODO - remove aliasing of oscillators (or remove saw & square option)

    // TODO - implement source gain
    dsp::Gain<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};