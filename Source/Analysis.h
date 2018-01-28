/*
  ==============================================================================

    Analysis.h
    Created: 11 Jan 2018 9:48:07am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioDataTransfer.h"

/**
	This class inherits from FixedBlockProcessor so that it can run on the audio processing thread and allow a FFT to
	be computed on a fixed block size, regardless of the block sized used by the audio device or host. An AudioProbe object
	is then used to make the processed data available for use on other threads.
*/
template <int Order>
class FftProcessor : public FixedBlockProcessor
{
public:

	struct FftFrame
    {
		alignas(16) float f[1 << Order];
	};

    explicit FftProcessor();
    ~FftProcessor ();

    void prepare (const dsp::ProcessSpec& spec) override;
    void performProcessing (const int channel) override;
    void copyFrequencyData (float* dest, const int channel);
	//void copyPhaseData (float* dest, const int channel);

    void addListener (typename AudioProbe<FftFrame>::Listener* listener);
    void removeListener (typename AudioProbe<FftFrame>::Listener* listener);

private:

    // Generic 2-term trigonometric window (from ICST_DSP BlkDsp)
    void trigwin2 (float* d, const int windowSize, const double c0, const double c1) const;

    // TODO - use juce::dsp WindowingFunctions instead
    // Hann window (from ICST_DSP BlkDsp)
    void hann (float* d, const int windowSize) const;

    dsp::FFT fft;
    const int size;
	AudioSampleBuffer temp;
	AudioSampleBuffer window;
	float atime;
	float rtime;
	int type;

    OwnedArray <AudioProbe <FftFrame>> freqProbes;;
    //OwnedArray <AudioProbe <FftFrame>> phaseProbes;

    ListenerList<typename AudioProbe<FftFrame>::Listener> listeners;
};


// ===========================================================================================
// Template implementations
// ===========================================================================================

template <int Order>
FftProcessor<Order>::FftProcessor (): FixedBlockProcessor (1 << Order),
                                      fft (Order),
                                      size (1 << Order)
{
    temp.setSize(1, size * 2, false, true);
    window.setSize(1, size);
    hann(window.getWritePointer(0), size);
}

template <int Order>
FftProcessor<Order>::~FftProcessor ()
{
}

template <int Order>
void FftProcessor<Order>::prepare (const dsp::ProcessSpec& spec)
{
    FixedBlockProcessor::prepare (spec);

    freqProbes.clear();
    //phaseProbes.clear();

    // Add probes for each channel to transfer audio data to the GUI
    for (auto ch = 0; ch < static_cast<int> (spec.numChannels); ++ch)
    {
        freqProbes.add(new AudioProbe<FftFrame>());
        //phaseProbes.add (new AsyncDataTransfer <FftFrame> ());
    }

    // But add listeners to the last channel only (prevents excessive paint calls)
    const auto lastChannel = static_cast<int>(spec.numChannels) - 1;
    for (auto i = 0; i < listeners.size(); ++i)
    {
        freqProbes[lastChannel]->addListener(listeners.getListeners()[i]);
        //phaseProbes[lastChannel]->addListener (listeners.getListeners()[i]);
    }
}

template <int Order>
void FftProcessor<Order>::performProcessing (const int channel)
{
    temp.copyFrom(0, 0, buffer.getReadPointer(channel), size);
    FloatVectorOperations::multiply(temp.getWritePointer(0), window.getWritePointer(0), size);
    fft.performFrequencyOnlyForwardTransform(temp.getWritePointer(0));
    freqProbes[channel]->writeFrame(reinterpret_cast<FftFrame*>(temp.getWritePointer(0)));
    //phaseProbes[channel]->writeFrame(reinterpret_cast<FftFrame*> (temp.getWritePointer (0) + size));
}

template <int Order>
void FftProcessor<Order>::copyFrequencyData (float* dest, const int channel)
{
    freqProbes[channel]->copyFrame(reinterpret_cast<FftFrame*>(dest));
}

//template <int Order>
//void FftProcessor<Order>::copyPhaseData (float* dest, const int channel)
//{
//	phaseProbes[channel]->copyFrame (reinterpret_cast <FftFrame*> (dest));
//}

template <int Order>
void FftProcessor<Order>::addListener (typename AudioProbe<FftFrame>::Listener* listener)
{
    listeners.add(listener);
}

template <int Order>
void FftProcessor<Order>::removeListener (typename AudioProbe<FftFrame>::Listener* listener)
{
    listeners.remove(listener);
}

template <int Order>
void FftProcessor<Order>::trigwin2 (float* d, const int windowSize, const double c0, const double c1) const
{
    if (windowSize == 1)
    {
        d[0] = static_cast<float>(c0 + c1);
        return;
    }
    auto tmp = MathConstants<double>::twoPi / static_cast<double>(windowSize - 1);
    const auto wre = cos(tmp);
    const auto wim = sin(tmp);
    auto re = -1.0;
    auto im = 0.0;
    for (auto i = 0; i <= (windowSize >> 1); i++)
    {
        d[windowSize - i - 1] = d[i] = static_cast<float>(c0 + c1 * re);
        tmp = re;
        re = wre * re - wim * im;
        im = wre * im + wim * tmp;
    }
}

template <int Order>
void FftProcessor<Order>::hann (float* d, const int windowSize) const
{
    trigwin2(d, windowSize, 0.5, 0.5);
}
