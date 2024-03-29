/*
  ==============================================================================

    FftProcessor.h
    Created: 29 Jan 2018 10:20:18pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "AudioDataTransfer.h"

/**
	This class inherits from FixedBlockProcessor so that it can run on the audio processing thread and allow a FFT to
	be computed on a fixed block size, regardless of the block sized used by the audio device or host. An AudioProbe object
	is then used to make the processed data available for use on other threads.
*/
template <int Order>
class FftProcessor final : public FixedBlockProcessor
{
public:

	// This FftFrame is necessary for us to use the AudioProbe class and is why this class (& therefore FftScope) is templated.
    struct FftFrame final
    {
		alignas(16) float f [1 << Order];
	};

    explicit FftProcessor();
    ~FftProcessor () override = default;

    /** Note that this clears then sets AudioProbes per channel - so it must be called before any attached classes attempt to add listeners to the AudioProbes. */
    void prepare (const dsp::ProcessSpec& spec) override;

    void performProcessing (const int channel) override;
    
    /** Copy frame of FFT frequency data */
    void copyFrequencyFrame (float* dest, const int channel) const;

    /** Copy frame of FFT phase data */
	void copyPhaseFrame (float* dest, const int channel) const;

    /** Call this to choose a different windowing method (class is initialised with Hann) */
    void setWindowingMethod (dsp::WindowingFunction<float>::WindowingMethod);

    /** Sets whether or not an envelope will be applied to the amplitude output. */
    void setAmplitudeEnvelopeEnabled (const bool shouldBeEnabled);

    /** Returns true if an envelope us being applied to the amplitude output. */
    bool isAmplitudeEnvelopeEnabled() const;

    /** Sets the release constant for the amplitude envelope. Note that this is calculated per FFT frame, which is a small
     *  fraction of the sample rate - so a reasonable value is between 0.2f to 0.6f. */
    void setAmplitudeEnvelopeReleaseConstant (const float releaseConstant);

    /** Gets the release constant for the amplitude envelope. */
    float getAmplitudeEnvelopeReleaseConstant() const;

    /** Allows a listener to add a lambda function as a callback to the AudioProbe assigned to the phase of the last channel.
     *  Listener callbacks are cleared each time prepare() is called on this class, so they must be added after this.
     *  
     *  Returns a function which allows the listener to de-register it's callback. The listener must remove any references
     *  to de-register functions that have become invalid.
     */
    ListenerRemovalCallback addListenerCallback (ListenerCallback&& listenerCallback) const;

private:

    dsp::FFT fft;
    const int size;
	AudioSampleBuffer temp;
	AudioSampleBuffer window;
    AudioSampleBuffer amplitudeEnvelope;
    float amplitudeCorrectionFactor = 0.0f;
    Atomic<bool> amplitudeEnvelopeEnabled = false;
    Atomic<float> amplitudeReleaseConstant = 0.0f;

    OwnedArray <AudioProbe <FftFrame>> freqProbes;;
    OwnedArray <AudioProbe <FftFrame>> phaseProbes;
};


// ===========================================================================================
//  Template implementations
// ===========================================================================================

template <int Order>
FftProcessor<Order>::FftProcessor(): FixedBlockProcessor (1 << Order),
                                      fft (Order),
                                      size (1 << Order)
{
    temp.setSize (1, size * 2, false, true);
    window.setSize (1, size);

    setWindowingMethod(dsp::WindowingFunction<float>::hann);
}

template <int Order>
void FftProcessor<Order>::prepare (const dsp::ProcessSpec& spec)
{
    FixedBlockProcessor::prepare (spec);

    amplitudeEnvelope.clear();
    amplitudeEnvelope.setSize (spec.numChannels, size);

    freqProbes.clear();
    phaseProbes.clear();

    // Add probes for each channel to transfer audio data to the GUI
    for (auto ch = 0; ch < static_cast<int> (spec.numChannels); ++ch)
    {
        freqProbes.add(new AudioProbe<FftFrame>());
        phaseProbes.add (new AudioProbe<FftFrame>());
    }
}

template <int Order>
void FftProcessor<Order>::performProcessing (const int channel)
{
    // Apply window to audio input
    temp.copyFrom (0, 0, buffer.getReadPointer (channel), size);
    FloatVectorOperations::multiply (temp.getWritePointer (0), window.getWritePointer (0), size);

    // Perform FFT
    fft.performFrequencyOnlyForwardTransform (temp.getWritePointer (0));

    // Correct amplitude
    FloatVectorOperations::multiply (temp.getWritePointer (0), amplitudeCorrectionFactor, size);

    if (amplitudeEnvelopeEnabled.get())
    {
        // Compute envelope on amplitude
        FloatVectorOperations::addWithMultiply (temp.getWritePointer (0), amplitudeEnvelope.getWritePointer (channel), amplitudeReleaseConstant.get(), size);
    
        // Store last audio frame in envelope buffer
        amplitudeEnvelope.copyFrom (channel, 0, temp.getReadPointer (0), size);
    }

    // Write output frames
    freqProbes[channel]->writeFrame (reinterpret_cast<const FftFrame*> (temp.getReadPointer (0)));
    phaseProbes[channel]->writeFrame (reinterpret_cast<const FftFrame*> (temp.getReadPointer (0) + size));
}

template <int Order>
void FftProcessor<Order>::copyFrequencyFrame (float* dest, const int channel) const
{
    freqProbes[channel]->copyFrame(reinterpret_cast<FftFrame*>(dest));
}

template <int Order>
void FftProcessor<Order>::copyPhaseFrame (float* dest, const int channel) const
{
	phaseProbes[channel]->copyFrame (reinterpret_cast <FftFrame*> (dest));
}

template <int Order>
void FftProcessor<Order>::setWindowingMethod (dsp::WindowingFunction<float>::WindowingMethod)
{
    const auto windowType = dsp::WindowingFunction<float>::hann;
    dsp::WindowingFunction<float>::fillWindowingTables (window.getWritePointer (0), static_cast<size_t> (size), windowType);

    auto windowIntegral = 0.0f;
    for (auto i = 0; i < size; ++i)
        windowIntegral += window.getWritePointer(0)[i];

    amplitudeCorrectionFactor = 2.0f / windowIntegral;
}

template<int Order>
inline void FftProcessor<Order>::setAmplitudeEnvelopeEnabled(const bool shouldBeEnabled)
{
    amplitudeEnvelopeEnabled.set(shouldBeEnabled);
}

template<int Order>
inline bool FftProcessor<Order>::isAmplitudeEnvelopeEnabled() const
{
    return amplitudeEnvelopeEnabled.get();
}

template<int Order>
inline void FftProcessor<Order>::setAmplitudeEnvelopeReleaseConstant(const float releaseConstant)
{
    amplitudeReleaseConstant.set (releaseConstant);
}

template<int Order>
inline float FftProcessor<Order>::getAmplitudeEnvelopeReleaseConstant() const
{
    return amplitudeReleaseConstant.get();
}

template <int Order>
ListenerRemovalCallback FftProcessor<Order>::addListenerCallback (ListenerCallback&& listenerCallback) const
{
    // If this asserts then you're trying to add the listener before the AudioProbes are set up
    jassert (getNumChannels()>0);

    if (phaseProbes.size() == getNumChannels() && phaseProbes[getNumChannels() - 1])
        return phaseProbes[getNumChannels() - 1]->addListenerCallback(std::forward<ListenerCallback>(listenerCallback));

    return {};
}
