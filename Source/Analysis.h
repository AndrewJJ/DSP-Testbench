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
	be computed on a fixed block size, regardless of the block sized used by the audio device or host. An AudioProcessorProbe object
	is then used to make the processed data available for use on other threads.
*/
template <int order>
class FftProcessor : public FixedBlockProcessor
{
public:

	struct FftFrame
    {
		alignas(16) float f[1 << order];
	};

	explicit FftProcessor (const int numChannels)
		: fft (order),
		  size (1 << order)
	{
        setFixedBlockSize (size);
		temp.setSize (1, size * 2, false, true);
		window.setSize (1, size);
		hann (window.getWritePointer (0), size);
	}

	~FftProcessor () {}

    void prepare (const dsp::ProcessSpec& spec) override
	{
        FixedBlockProcessor::prepare (spec);

        freqProbes.clear();
        //phaseProbes.clear();
	    for (auto ch = 0; ch < spec.numChannels; ++ch)
        {
            freqProbes.add (new AudioProcessorProbe <FftFrame> ());
            //phaseProbes.add (new AsyncDataTransfer <FftFrame> ());
            for (auto i = 0; i < listeners.size(); ++i)
            {
	            freqProbes[ch]->addListener (listeners.getListeners()[i]);
                //phaseProbes[ch]->addListener (listeners.getListeners()[i]);
            }
        }
	}

	void performProcessing (const int channel) override
	{
		temp.copyFrom (0, 0, buffer.getReadPointer (channel), size);
		FloatVectorOperations::multiply (temp.getWritePointer (0), window.getWritePointer (0), size);
	    fft.performFrequencyOnlyForwardTransform (temp.getWritePointer (0));
		freqProbes[channel]->writeFrame(reinterpret_cast<FftFrame*> (temp.getWritePointer (0)));
		//phaseProbes[channel]->writeFrame(reinterpret_cast<FftFrame*> (temp.getWritePointer (0) + size));
	}

	void copyFrequencyData (float* dest, const int channel)
	{
		freqProbes[channel]->copyFrame (reinterpret_cast <FftFrame*> (dest));
	}

	//void copyPhaseData (float* dest, const int channel)
	//{
	//	phaseProbes[channel]->copyFrame (reinterpret_cast <FftFrame*> (dest));
	//}
	
    void addListener (typename AudioProcessorProbe<FftFrame>::Listener* listener)
	{
	    listeners.add (listener);
	}

    void removeListener (typename AudioProcessorProbe<FftFrame>::Listener* listener)
	{
	    listeners.remove (listener);
	}

private:

    // Generic 2-term trigonometric window (from ICST_DSP BlkDsp)
    void trigwin2 (float* d, const int windowSize, const double c0, const double c1) const
    {
	    if (windowSize == 1)
        {
	        d[0] = static_cast<float> (c0 + c1);
            return;
        }
	    auto tmp = MathConstants<double>::twoPi / static_cast<double> (windowSize - 1);
	    const auto wre = cos(tmp);
        const auto wim = sin(tmp);
	    auto re=-1.0;
        auto im=0.0;
	    for (auto i=0; i<=(windowSize>>1); i++) {
		    d[windowSize-i-1] = d[i] = static_cast<float> (c0 + c1 * re);
		    tmp = re;
            re = wre*re - wim*im;
            im = wre*im + wim*tmp;
	    }
    }

    // Hann window (from ICST_DSP BlkDsp)
    void hann (float* d, const int windowSize) const
    {
        trigwin2 (d, windowSize, 0.5, 0.5);
    }

    dsp::FFT fft;
	const int size;
	AudioSampleBuffer temp;
	AudioSampleBuffer window;
	float atime;
	float rtime;
	int type;

    OwnedArray <AudioProcessorProbe <FftFrame>> freqProbes;;
    //OwnedArray <AudioProcessorProbe <FftFrame>> phaseProbes;

    ListenerList<typename AudioProcessorProbe<FftFrame>::Listener> listeners;
};
