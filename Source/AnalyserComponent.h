/*
  ==============================================================================

    AnalyserComponent.h
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "FftProcessor.h"
#include "FftScope.h"

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

    bool isActive() const noexcept;

    // TODO - add waveform display
    // TODO - add detailed metering (peak, RMS, VU)
    // TODO - add phase scope

private:

    Label lblTitle;
    TextButton btnDisable;

    FftProcessor<12> fftMult;
    FftScope<12> fftScope;

    Atomic<bool> statusActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};