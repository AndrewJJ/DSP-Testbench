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