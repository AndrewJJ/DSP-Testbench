/*
  ==============================================================================

    PulseFunctions.h
    Created: 2 Feb 2019 3:12:17pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace juce {
namespace dsp {

template <typename SampleType>
class PulseFunctionBase : public ProcessorBase
{
public:
    
    PulseFunctionBase()
    = default;

    ~PulseFunctionBase()
    = default;
    
    void prepare (const dsp::ProcessSpec&) override
    { sampleIndex = 0; }

    virtual void process (const dsp::ProcessContextReplacing<SampleType>& context) override;

    virtual void reset () override
    { sampleIndex = 0; }

    /**
     * Set pre delay in samples
     */
    virtual void setPreDelay (const size_t numSamples)
    { preDelay = numSamples; }
    
    size_t getPreDelay() const
    { return preDelay; }

    /**
     * Set pulse width in samples
     */
    virtual void setPulseWidth (const size_t numSamples)
    { pulseWidth = numSamples; }
    
    size_t getPulseWidth() const
    { return pulseWidth; }

    /**
     * Set leading edge of pulse to transition from zero to either full scale positive or negative
     */
    void setPositivePolarity (const bool positive)
    { positivePolarity = positive; }
    
    bool getPositivePolarity() const
    { return positivePolarity; }

protected:
    size_t sampleIndex = 0;
    size_t preDelay = 100;
    size_t pulseWidth = 1;
    bool positivePolarity = true;
};

template <typename SampleType>
void PulseFunctionBase<SampleType>::process (const dsp::ProcessContextReplacing<SampleType>& context)
{
    auto&& outBlock = context.getOutputBlock();
    const auto pulseValue = positivePolarity ? static_cast<SampleType>(1.0) : static_cast<SampleType>(-1.0);
    const auto zeroValue = static_cast<SampleType>(0.0);
    const auto endOfImpulse = preDelay + pulseWidth;

    // this is an output-only processor
    jassert (context.getInputBlock().getNumChannels() == 0 || (!context.usesSeparateInputAndOutputBlocks()));

    if (outBlock.getNumChannels() > 0)
    {
        auto* out = outBlock.getChannelPointer(0);
        for (size_t i = 0; i < outBlock.getNumSamples(); ++i)
        {
            if (sampleIndex >= preDelay && sampleIndex < endOfImpulse)
                out[i] = pulseValue;
            else
                out[i] = zeroValue;
            sampleIndex++;
        }
        // Copy to other channels
        for (size_t ch = 1; ch < outBlock.getNumChannels(); ++ch)
        {
            float* dest = outBlock.getChannelPointer(static_cast<size_t>(ch));
            FloatVectorOperations::copy(dest, out, static_cast<int>(outBlock.getNumSamples()));
        }
    }
    else
        sampleIndex += context.getInputBlock().getNumSamples();
}

// =================================================================
// =================================================================

template <typename SampleType>
class ImpulseFunction : public PulseFunctionBase<SampleType>
{
public:
    ImpulseFunction() = default;
    ~ImpulseFunction() = default;
};

// =================================================================
// =================================================================

template <typename SampleType>
class StepFunction : public PulseFunctionBase<SampleType>
{
public:
    StepFunction()
    {
        PulseFunctionBase<SampleType> ();
        PulseFunctionBase<SampleType>::pulseWidth = std::numeric_limits<size_t>::max() - PulseFunctionBase<SampleType>::preDelay;
    }
    ~StepFunction() = default;

    void setPulseWidth (const size_t) override
    { }

    void setPreDelay (const size_t numSamples) override
    {
        PulseFunctionBase<SampleType>::preDelay = numSamples;
        PulseFunctionBase<SampleType>::pulseWidth = std::numeric_limits<size_t>::max() - PulseFunctionBase<SampleType>::preDelay;
    }
};


}   // namespace dsp
}   // namespace juce
