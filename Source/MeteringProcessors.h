/*
  ==============================================================================

    MeteringProcessors.h
    Created: 18 Jan 2018 4:01:46pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class SimpleLevelMeterProcessor : public dsp::ProcessorBase
{
public:

	SimpleLevelMeterProcessor()
    {};

	~SimpleLevelMeterProcessor()
    = default;

	float getLevel (const int channelNum) const;
	float getLeveldB (const int channelNum) const;
	size_t getNumChannels () const;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleLevelMeterProcessor)

private:

    size_t numChannels = 0;
	Array<Atomic<float>> envelopeContinuation;
    float attackTimeConstant = 0.0f;
	float releaseTimeConstant = 0.0f;
};