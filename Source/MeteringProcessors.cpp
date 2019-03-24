/*
  ==============================================================================

    MeteringProcessors.cpp
    Created: 18 Jan 2018 4:01:46pm
    Author:  Andrew

  ==============================================================================
*/

#include "MeteringProcessors.h"

float PeakMeterProcessor::getLevel (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
        return envelopeContinuation[channelNumber].load();
    else
        return 0.0f;
}
float PeakMeterProcessor::getLevelDb (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
    {
        const auto env = envelopeContinuation[channelNumber].load();
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

float VUMeterProcessor::getLevel (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
        return sqrtf (envelopeContinuation[channelNumber].load());
    else
        return 0.0f;
}
float VUMeterProcessor::getLevelDb (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
    {
        const auto level = sqrtf (envelopeContinuation[channelNumber].load());
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

long ClipCounterProcessor::getNumClipEvents(const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
        return numClipEvents[channelNumber].load();
    else
        return 0;
}
double ClipCounterProcessor::getAvgClipLength (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels) && numClipEvents[channelNumber].load() > 0)
        return static_cast<double>(numClippedSamples[channelNumber].load()) / static_cast<double>(numClipEvents[channelNumber].load());
    else
        return 0.0;
}
long ClipCounterProcessor::getMaxClipLength (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
        return maxClipLength[channelNumber].load();
    else
        return 0;
}
long ClipCounterProcessor::getNumClippedSamples (const int channelNumber) const
{
    if (channelNumber >= 0 && channelNumber < static_cast<int> (numChannels))
        return numClippedSamples[channelNumber].load();
    else
        return 0;
}
size_t ClipCounterProcessor::getNumChannels () const
{
   return numChannels;
}
void ClipCounterProcessor::prepare (const dsp::ProcessSpec& spec)
{
    numChannels = spec.numChannels;

	numClipEvents.allocate (numChannels, true);
    maxClipLength.allocate (numChannels, true);
    numClippedSamples.allocate (numChannels, true);
    clipLengthContinuation.allocate (numChannels, true);
}
void ClipCounterProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    jassert (numChannels == context.getInputBlock().getNumChannels());
    
    const auto numSamples = context.getInputBlock().getNumSamples();
	for (auto ch = 0; ch < static_cast<int> (numChannels); ++ch)
	{
        // Pointer to samples for this channel
        const auto* x = context.getInputBlock().getChannelPointer(static_cast<int> (ch));

        for (size_t i=0; i < numSamples; i++)
        {
            // Check if this sample is clipped
            if (isClipped (x[i]))
            {
                numClippedSamples[ch] += 1;
                if (clipLengthContinuation[ch] == 0)
                {
                    // We just started a new clip event
	                numClipEvents[ch] += 1;
                }
                else
                {
                    // Clip had previously started and is still continuing
                    if (clipLengthContinuation[ch].load() > maxClipLength[ch].load())
                        maxClipLength[ch] = clipLengthContinuation[ch].load();                    
                }
                clipLengthContinuation[ch] += 1;
            }
            else
            {
                // Reset continuation
                clipLengthContinuation[ch].store (0);
            }
        }

        // TODO - delete

        //{
        //    // If this sample is clipped, then increment the continuation counter
        //    if (isClipped (x[i]))
        //    {
        //        clipLengthContinuation[ch].store (clipLengthContinuation[ch] + 1);
        //    }
        //    else
        //    {
        //        // Sample wasn't clipped so need to check if we just finished a clip event
        //        if (clipLengthContinuation[ch] > 0)
        //        {
        //            // We just finished a clip event, so update all the stats
	       //         numClipEvents[ch] += 1;
        //            sumClipLength[ch] += clipLengthContinuation[ch].load();
        //            if (clipLengthContinuation[ch].load() > maxClipLength[ch].load())
        //                maxClipLength[ch] = clipLengthContinuation[ch].load();
        //            numClippedSamples[ch] += clipLengthContinuation[ch].load();
        //            // Reset continuation
        //            clipLengthContinuation[ch].store (0);
        //        }
        //    }
        //}
        //int i = 0;
        //while (i < numSamples)
        //{
        //    if (clipLengthContinuation[ch] == 0)
        //    {
        //        // We weren't clipping at the end of the last block, so let's advance to next clip event
        //        while (i < numSamples && !isClipped(x[i]))
        //            i++;
        //    }
        //    if (i < numSamples)
        //    {
        //        // We're not at the end of the block, so we must have found a clip event
        //        // So let's advance to the end of the clip event
        //        while (i < numSamples && !isClipped(x[i]))
        //            i++;
        //    }
	}
}
void ClipCounterProcessor::reset()
{
    for (auto ch = 0; ch < static_cast<int>(numChannels); ++ch)
    {
	    numClipEvents[ch].store (0);
        maxClipLength[ch].store (0);
        numClippedSamples[ch].store (0);
        clipLengthContinuation[ch].store (0);
    }
}
inline bool ClipCounterProcessor::isClipped (const float amplitude)
{
    return (amplitude > 1.0f || amplitude < -1.0f);
}
