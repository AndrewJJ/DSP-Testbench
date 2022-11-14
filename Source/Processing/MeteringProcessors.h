/*
  ==============================================================================

    MeteringProcessors.h
    Created: 18 Jan 2018 4:01:46pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <atomic>
#ifdef JUCE_CLANG
#include <mm_malloc.h>
#endif

class PeakMeterProcessor final : public dsp::ProcessorBase
{
public:
	PeakMeterProcessor() = default;
    ~PeakMeterProcessor() override = default;

    [[nodiscard]] float getLevel (const int channelNumber) const;
    [[nodiscard]] float getLevelDb (const int channelNumber) const;
    [[nodiscard]] size_t getNumChannels() const;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

private:
    size_t numChannels = 0;
    HeapBlock <std::atomic <float>> envelopeContinuation{};
	float releaseTimeConstant = 0.0f;
    const float antiDenormalFloat = 1e-15f;
    const float noSignalDbLevel = -150.0f;

public:
    // Declare non-copyable, non-movable
    PeakMeterProcessor (const PeakMeterProcessor&) = delete;
    PeakMeterProcessor& operator= (const PeakMeterProcessor&) = delete;
    PeakMeterProcessor (PeakMeterProcessor&& other) = delete;
    PeakMeterProcessor& operator=(PeakMeterProcessor&& other) = delete;
};

class VUMeterProcessor final : public dsp::ProcessorBase
{
public:
	VUMeterProcessor() = default;
    ~VUMeterProcessor() override = default;

    [[nodiscard]] float getLevel (const int channelNumber) const;
    [[nodiscard]] float getLevelDb (const int channelNumber) const;
    [[nodiscard]] size_t getNumChannels() const;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

private:
    size_t numChannels = 0;
    HeapBlock <std::atomic <float>> envelopeContinuation{};
	float timeConstant = 0.0f;
    const float antiDenormalFloat = 1e-15f;
    const float noSignalDbLevel = -150.0f;

public:
    // Declare non-copyable, non-movable
    VUMeterProcessor (const VUMeterProcessor&) = delete;
    VUMeterProcessor& operator= (const VUMeterProcessor&) = delete;
    VUMeterProcessor (VUMeterProcessor&& other) = delete;
    VUMeterProcessor& operator=(VUMeterProcessor&& other) = delete;
};

class ClipCounterProcessor final : public dsp::ProcessorBase
{
public:

	ClipCounterProcessor() = default;;
    ~ClipCounterProcessor() override = default;

    //Number of clip events
    [[nodiscard]] long getNumClipEvents (const int channelNumber) const;

    //Average clip event length
    [[nodiscard]] double getAvgClipLength (const int channelNumber) const;

    //Largest clip event length
    [[nodiscard]] long getMaxClipLength (const int channelNumber) const;

    //Total number of clipped samples
    [[nodiscard]] long getNumClippedSamples (const int channelNumber) const;

    [[nodiscard]] size_t getNumChannels() const;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

private:

    static inline bool isClipped (const float amplitude);

    size_t numChannels = 0;
    // Values for output
    HeapBlock <std::atomic <long>> numClipEvents{};
    HeapBlock <std::atomic <long>> maxClipLength{};
    HeapBlock <std::atomic <long>> numClippedSamples{};
    // State variable so we can detect runs across multiple blocks
    HeapBlock <std::atomic <long>> clipLengthContinuation{};

public:
    // Declare non-copyable, non-movable
    ClipCounterProcessor (const ClipCounterProcessor&) = delete;
    ClipCounterProcessor& operator= (const ClipCounterProcessor&) = delete;
    ClipCounterProcessor (VUMeterProcessor&& other) = delete;
    ClipCounterProcessor& operator=(ClipCounterProcessor&& other) = delete;
};