/*
  ==============================================================================

    OscilloscopeProcessor.h
    Created: 4 Feb 2018 1:40:18pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "AudioDataTransfer.h"

/**
	This class inherits from FixedBlockProcessor so that it can run on the audio processing thread and allow an oscilloscope
	be plotted at a fixed block size, regardless of the block sized used by the audio device or host. An AudioProbe object
	is then used to make the processed data available for use on other threads.
*/
class OscilloscopeProcessor final : public FixedBlockProcessor
{
private:
    static const int frame_size = 4096;

public:
    struct OscilloscopeFrame final
    {
	    alignas(16) float f[frame_size];
    };

    explicit OscilloscopeProcessor();
    ~OscilloscopeProcessor() = default;

    void prepare (const dsp::ProcessSpec& spec) override;
    void performProcessing (const int channel) override;

    /** Copy frame of audio data */
    void copyFrame (float* dest, const int channel) const;

    /** Allows a listener to add a lambda function as a callback to the AudioProbe assigned to the last channel.
     *  Listener callbacks are cleared each time prepare() is called on this class, so they must be added after this.
     *  
     *  Returns a function which allows the listener to de-register it's callback. The listener must remove any references
     *  to de-register functions that have become invalid.
     */
    ListenerRemovalCallback addListenerCallback (ListenerCallback&& listenerCallback) const;


private:
    OwnedArray <AudioProbe <OscilloscopeFrame>> audioProbes;

public:
    // Declare non-copyable, non-movable
    OscilloscopeProcessor (const OscilloscopeProcessor&) = delete;
    OscilloscopeProcessor& operator= (const OscilloscopeProcessor&) = delete;
    OscilloscopeProcessor (OscilloscopeProcessor&& other) = delete;
    OscilloscopeProcessor& operator=(OscilloscopeProcessor&& other) = delete;
};


// ===========================================================================================
//  Implementation
// ===========================================================================================

inline OscilloscopeProcessor::OscilloscopeProcessor (): FixedBlockProcessor(frame_size)
{
}

inline void OscilloscopeProcessor::prepare (const dsp::ProcessSpec& spec)
{
    FixedBlockProcessor::prepare(spec);

    audioProbes.clear();

    // Add probes for each channel to transfer audio data to the GUI
    for (auto ch = 0; ch < static_cast<int>(spec.numChannels); ++ch)
        audioProbes.add(new AudioProbe<OscilloscopeFrame>());
}

inline void OscilloscopeProcessor::performProcessing (const int channel)
{
    audioProbes[channel]->writeFrame(reinterpret_cast<const OscilloscopeFrame*>(buffer.getReadPointer(channel)));
}

inline void OscilloscopeProcessor::copyFrame (float* dest, const int channel) const
{
    audioProbes[channel]->copyFrame(reinterpret_cast<OscilloscopeFrame*>(dest));
}

inline ListenerRemovalCallback OscilloscopeProcessor::addListenerCallback (ListenerCallback&& listenerCallback) const
{
    // If this asserts then you're trying to add the listener before the AudioProbes are set up
    jassert (getNumChannels()>0);

    if (audioProbes.size() == getNumChannels() && audioProbes[getNumChannels() - 1])
        return audioProbes[getNumChannels() - 1]->addListenerCallback (std::forward<ListenerCallback>(listenerCallback));

    return {};
}
