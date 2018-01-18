/*
  ==============================================================================

    MeteringProcessors.cpp
    Created: 18 Jan 2018 4:01:46pm
    Author:  Andrew

  ==============================================================================
*/

#include "MeteringProcessors.h"

float SimpleLevelMeterProcessor::getLevel (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
	    return envelopeContinuation[channelNum].get();
    else
        return 0.0f;
}
float SimpleLevelMeterProcessor::getLeveldB (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
	    return Decibels::gainToDecibels (envelopeContinuation[channelNum].get());
    else
        return 0.0f;
}
size_t SimpleLevelMeterProcessor::getNumChannels () const
{
    return numChannels;
}
void SimpleLevelMeterProcessor::prepare (const dsp::ProcessSpec& spec)
{
	numChannels = spec.numChannels;

	envelopeContinuation.clear();
	for (auto ch=0; ch<numChannels; ++ch)
		envelopeContinuation.add (0.0f);
    
    const auto attackTime = 1.0f; // 1 sample
    const auto releaseTime = 0.1f * static_cast<float> (spec.sampleRate); // 100 msec in samples
    attackTimeConstant =  1.0f - exp (-1.0f / attackTime);
    releaseTimeConstant =  1.0f - exp (-1.0f / releaseTime);
}
void SimpleLevelMeterProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (numChannels == context.getInputBlock().getNumChannels());
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
	{
        ScopedNoDenormals noDenormals;
        const auto* channelBuffer = context.getInputBlock().getChannelPointer(ch);
        auto x = 0.0f;
        auto env = envelopeContinuation[ch].get();
        // Calculate envelope over the block, but only keep last envelope sample as the meter refresh rate
        // should be slower than the block processing rate
        for (size_t i=0; i < context.getInputBlock().getNumSamples(); i++)
        {
            x = fabsf (channelBuffer[i]);
            if (x > env)
                env += (attackTimeConstant * (x - env));
            else
                env += (releaseTimeConstant * (x - env));
	    }
        envelopeContinuation[ch].set (env);
	}
}
void SimpleLevelMeterProcessor::reset ()
{
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
        envelopeContinuation[ch].set (0.0f);
}
