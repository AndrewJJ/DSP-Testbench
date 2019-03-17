/*
  ==============================================================================

    MeteringProcessors.cpp
    Created: 18 Jan 2018 4:01:46pm
    Author:  Andrew

  ==============================================================================
*/

#include "MeteringProcessors.h"

float SimplePeakMeterProcessor::getLevel (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
        return envelopeContinuation[channelNum].load();
    else
        return 0.0f;
}
float SimplePeakMeterProcessor::getLevelDb (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
    {
        const auto env = envelopeContinuation[channelNum].load();
        return Decibels::gainToDecibels (env);
    }
    else
        return -100.0f;
}
size_t SimplePeakMeterProcessor::getNumChannels () const
{
    return numChannels;
}
void SimplePeakMeterProcessor::prepare (const dsp::ProcessSpec& spec)
{
	numChannels = spec.numChannels;

	envelopeContinuation.allocate (numChannels, true);
    
    const auto releaseTime = 0.300f * static_cast<float> (spec.sampleRate); // 300 msec in samples
    releaseTimeConstant =  1.0f - exp (-1.0f / releaseTime);
}
void SimplePeakMeterProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (numChannels == context.getInputBlock().getNumChannels());
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
	{
        const auto* channelBuffer = context.getInputBlock().getChannelPointer(static_cast<int> (ch));
        float x;
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
        envelopeContinuation[ch].store (env);
	}
}
void SimplePeakMeterProcessor::reset ()
{
    envelopeContinuation.clear (numChannels);
}
