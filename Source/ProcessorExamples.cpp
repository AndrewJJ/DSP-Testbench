/*
  ==============================================================================

    ProcessorExamples.cpp
    Created: 23 Apr 2019 7:50:32pm
    Author:  Andrew

  ==============================================================================
*/

#include "ProcessorExamples.h"

LpfExample::LpfExample()
: ProcessorHarness (2)
{
    init();
}

void LpfExample::prepare (const dsp::ProcessSpec & spec)
{
    numChannels = spec.numChannels;
    freqConversionFactor = MathConstants<double>::pi / spec.sampleRate;
    z1.allocate (numChannels, true);
    z2.allocate (numChannels, true);
}

void LpfExample::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (context.getInputBlock().getNumChannels() == context.getOutputBlock().getNumChannels());

    calculateCoefficients();

    for (size_t ch = 0; ch <context.getOutputBlock().getNumChannels(); ++ch)
    {
        auto* in = context.getInputBlock().getChannelPointer (ch);
        auto* out = context.getOutputBlock().getChannelPointer (ch);

        for (size_t i = 0; i < context.getOutputBlock().getNumSamples(); i++)
        {
            const auto sample = in[i] * a0 + z1[ch];
            z1[ch] = in[i] * a1 + z2[ch] - b1 * sample;
            z2[ch] = in[i] * a0 - b2 * sample;
            out[i] = static_cast<float> (sample);
        }
    }
}

void LpfExample::reset()
{
   init();
}

String LpfExample::getControlName (const int index)
{
    switch (index)
    {
        case 0: return String ("Frequency");
    }
    // If you hit this assert then you haven't defined enough control names!
    jassertfalse;
    return String();
}

float LpfExample::getDefaultControlValue (const int index)
{
    switch (index)
    {
        case 0: return 0.75f;
    }
    // If you hit this assert then you haven't defined enough control names!
    jassertfalse;
    return 0.0f;
}

void LpfExample::init()
{
    a0 = 1.0;
    a1 = 0.0;
    b1 = 0.0;
    b2 = 0.0;
    for (auto ch = 0; ch < numChannels; ++ch)
    {
        z1[ch] = 0.0;
        z2[ch] = 0.0;
    }
}

void LpfExample::calculateCoefficients()
{
    // We're logarithmically mapping the 0..1 range of the control to 10Hz..20kHz
    const auto freq = pow (10.0, getControlValue (0) * 3.30103 + 1.0) * freqConversionFactor;
    const auto k = tan (freq);
    const auto kk = k * k;
    const auto norm = 1.0 / (1.0 + k + kk);
    a0 = kk * norm;
    a1 = 2.0 * a0;
    b1 = 2.0 * (kk - 1.0) * norm;
    b2 = (1.0 - k + kk) * norm;
}
