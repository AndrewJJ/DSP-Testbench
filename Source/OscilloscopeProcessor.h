/*
  ==============================================================================

    OscilloscopeProcessor.h
    Created: 4 Feb 2018 1:40:18pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioDataTransfer.h"

/**
	This class inherits from FixedBlockProcessor so that it can run on the audio processing thread and allow am oscilloscope
	be plotted at a fixed block size, regardless of the block sized used by the audio device or host. An AudioProbe object
	is then used to make the processed data available for use on other threads.
*/
class OscilloscopeProcessor : public FixedBlockProcessor
{
public:

    // This is necessary for us to use the AudioProbe class and is why this class (& therefore FftScope is templated).
    struct OscilloscopeFrame
    {
	    alignas(16) float f[8192];
    };

    explicit OscilloscopeProcessor()
        :   FixedBlockProcessor (8192)
    { }

    ~OscilloscopeProcessor() = default;

    void prepare (const dsp::ProcessSpec& spec) override
    {
        FixedBlockProcessor::prepare (spec);

        audioProbes.clear();

        // Add probes for each channel to transfer audio data to the GUI
        for (auto ch = 0; ch < static_cast<int> (spec.numChannels); ++ch)
            audioProbes.add(new AudioProbe<OscilloscopeFrame>());

        // But add listeners to the last channel only (prevents excessive paint calls)
        const auto lastChannel = static_cast<int>(spec.numChannels) - 1;
        for (auto i = 0; i < listeners.size(); ++i)
            audioProbes[lastChannel]->addListener(listeners.getListeners()[i]);        
    }

    void performProcessing (const int channel) override
    {
        audioProbes[channel]->writeFrame (reinterpret_cast<const OscilloscopeFrame*>(buffer.getReadPointer(channel)));
    }
    
    void addListener (typename AudioProbe<OscilloscopeFrame>::Listener* listener)
    {
        listeners.add(listener);
    }
    
    void removeListener (typename AudioProbe<OscilloscopeFrame>::Listener* listener)
    {
        listeners.remove(listener);
    }

    /** Copy frame of audio data */
    void copyFrame (float* dest, const int channel) const
    {
        audioProbes[channel]->copyFrame(reinterpret_cast<OscilloscopeFrame*>(dest));
    }

    /** Returns true if the referenced probe is owned by this object */
    bool ownsProbe (AudioProbe<OscilloscopeFrame>* audioProbe) const
    {
        auto owned = false;
        for (auto p : audioProbes)
            if (audioProbe == p)
            {
                owned = true;
                break;
            }        
        return owned;
    }

private:

    OwnedArray <AudioProbe <OscilloscopeFrame>> audioProbes;;
    ListenerList<typename AudioProbe<OscilloscopeFrame>::Listener> listeners;
};