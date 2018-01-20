/*
  ==============================================================================

    AnalyserComponent.h
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Analysis.h"

class SimpleFftScope : public Component, public AudioProcessorProbe <FftProcessor <12>::FftFrame>::Listener
{
public:

    SimpleFftScope ();
    ~SimpleFftScope ();

    void paint (Graphics& g) override;
    void assignFftMult (FftProcessor<12>* fftMultPtr);
    void asyncProbeUpdated (AudioProcessorProbe<FftProcessor<12>::FftFrame>* probe) override;
    void prepare (const dsp::ProcessSpec& spec);

private:
    
    float todBVoltsFromLinear (const float x) const;

	FftProcessor<12>* fftMult;
	float f [1 << 12]; // pre-allocated array for use by paint routine
	double samplingFreq = 48000;
};

class AnalyserComponent  :  public Component, public dsp::ProcessorBase
{
public:

    AnalyserComponent();
    ~AnalyserComponent();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    bool isActive() const;

    // TODO - improve FFT display
    // TODO - add waveform display
    // TODO - add detailed metering (peak, RMS, VU)
    // TODO - add phase scope

private:

    Label lblTitle;
    TextButton btnDisable;

    FftProcessor<12> fftMult;
    SimpleFftScope fftScope;

    bool statusActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};