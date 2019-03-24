/*
  ==============================================================================

    MeteringProcessors.cpp
    Created: 18 Jan 2018 4:01:46pm
    Author:  Andrew

  ==============================================================================
*/

#include "MeteringProcessors.h"

float PeakMeterProcessor::getLevel (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
        return envelopeContinuation[channelNum].load();
    else
        return 0.0f;
}
float PeakMeterProcessor::getLevelDb (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
    {
        const auto env = envelopeContinuation[channelNum].load();
        return Decibels::gainToDecibels (env);
    }
    else
        return noSignalDbLevel;
}
size_t PeakMeterProcessor::getNumChannels () const
{
    return numChannels;
}
void PeakMeterProcessor::prepare (const dsp::ProcessSpec& spec)
{
	numChannels = spec.numChannels;

	envelopeContinuation.allocate (numChannels, true);
    
    const auto releaseTime = 0.650f * static_cast<float> (spec.sampleRate); // 650 msec (as per PPM spec)
    releaseTimeConstant =  1.0f - exp (-1.0f / releaseTime);
}
void PeakMeterProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (numChannels == context.getInputBlock().getNumChannels());
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
	{
        const auto* channelBuffer = context.getInputBlock().getChannelPointer(static_cast<int> (ch));
        auto env = envelopeContinuation[ch].load();
        // Calculate envelope over the block, but only keep last envelope sample as the meter refresh rate
        // should be slower than the block processing rate
        for (size_t i=0; i < context.getInputBlock().getNumSamples(); i++)
        {
            const auto x = fabsf (channelBuffer[i]) + antiDenormalFloat;
            if (x > env)
                env = x; // Instant attack
            else
                env += (releaseTimeConstant * (x - env));
	    }
        envelopeContinuation[ch].store (env);
	}
}
void PeakMeterProcessor::reset ()
{
    envelopeContinuation.clear (numChannels);
}

float VUMeterProcessor::getLevel (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
        return sqrtf (envelopeContinuation[channelNum].load());
    else
        return 0.0f;
}
float VUMeterProcessor::getLevelDb (const int channelNum) const
{
    if (channelNum >= 0 && channelNum < static_cast<int> (numChannels))
    {
        const auto level = sqrtf (envelopeContinuation[channelNum].load());
        return Decibels::gainToDecibels (level);
    }
    else
        return noSignalDbLevel;
}
size_t VUMeterProcessor::getNumChannels () const
{
    return numChannels;
}
void VUMeterProcessor::prepare (const dsp::ProcessSpec& spec)
{
	numChannels = spec.numChannels;

	envelopeContinuation.allocate (numChannels, true);
    
    const auto responseTime = 0.600f * static_cast<float> (spec.sampleRate); // 600 msec (similar to K system)
    timeConstant =  1.0f - exp (-1.0f / responseTime);
}
void VUMeterProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (numChannels == context.getInputBlock().getNumChannels());
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
	{
        const auto* channelBuffer = context.getInputBlock().getChannelPointer(static_cast<int> (ch));
        auto env = envelopeContinuation[ch].load();
        // Calculate envelope over the block, but only keep last envelope sample as the meter refresh rate
        // should be slower than the block processing rate
        for (size_t i=0; i < context.getInputBlock().getNumSamples(); i++)
        {
            // Calculate envelope on RMS values
            const auto x = channelBuffer[i] * channelBuffer[i] + antiDenormalFloat * antiDenormalFloat;
            // Attack & release time are the same for this meter
            env += (timeConstant * (x - env));
	    }
        envelopeContinuation[ch].store (env);
	}
}
void VUMeterProcessor::reset ()
{
    envelopeContinuation.clear (numChannels);
}