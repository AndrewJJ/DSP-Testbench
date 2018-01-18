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
	    // TODO - delete
        //return envelopeContinuation[channelNum].get();
        return envelopeContinuation[channelNum].load();
    else
        return 0.0f;
}
float SimpleLevelMeterProcessor::getLeveldB (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
    {
        // TODO - delete
        //const auto env = envelopeContinuation[channelNum].get();
        const auto env = envelopeContinuation[channelNum].load();
        return Decibels::gainToDecibels (env);
    }
    else
        return -100.0f;
}
size_t SimpleLevelMeterProcessor::getNumChannels () const
{
    return numChannels;
}
void SimpleLevelMeterProcessor::prepare (const dsp::ProcessSpec& spec)
{
	numChannels = spec.numChannels;

	envelopeContinuation.allocate (numChannels, true);
	// TODO - delete
    //for (size_t ch=0; ch < numChannels; ++ch)
		//envelopeContinuation.add (0.0f);
    
    const auto releaseTime = 0.100f * static_cast<float> (spec.sampleRate); // 100 msec in samples
    releaseTimeConstant =  1.0f - exp (-1.0f / releaseTime);
}
void SimpleLevelMeterProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (numChannels == context.getInputBlock().getNumChannels());
    //jassert (numChannels == static_cast<size_t> (envelopeContinuation.size()));
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
	{
        const auto* channelBuffer = context.getInputBlock().getChannelPointer(static_cast<int> (ch));
        auto x = 0.0f;
        // TODO - delete
	    //auto env = envelopeContinuation[ch].get();
        auto env = envelopeContinuation[ch].load();
        // Calculate envelope over the block, but only keep last envelope sample as the meter refresh rate
        // should be slower than the block processing rate
        for (size_t i=0; i < context.getInputBlock().getNumSamples(); i++)
        {
            x = fabsf (channelBuffer[i]) + antiDenormalFloat;
            if (x > env)
                env = x; // Instant attack
            else
                env += (releaseTimeConstant * (x - env));
	    }
        // TODO - delete
        //envelopeContinuation[ch].set (env);
        envelopeContinuation[ch].store (env);
	}
}
void SimpleLevelMeterProcessor::reset ()
{
    // TODO - delete
    envelopeContinuation.clear (numChannels);
	//for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
        //envelopeContinuation[ch].set (0.0f);
}
