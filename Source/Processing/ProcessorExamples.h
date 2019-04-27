/*
  ==============================================================================

    ProcessorExamples.h
    Created: 23 Apr 2019 7:50:32pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once
#include "ProcessorHarness.h"

/** 
 * Example processor implementing a low pass filter using a biquad.
 */
class LpfExample : public ProcessorHarness
{
public:
    LpfExample();
    ~LpfExample() = default;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    String getControlName (const int index) override;
    double getDefaultControlValue (const int index);

private:
    void init();
    void calculateCoefficients();

    int numChannels = 0;
    double freqConversionFactor = 0.0;
    double a0, a1, b1, b2;
    HeapBlock<double> z1, z2;
};