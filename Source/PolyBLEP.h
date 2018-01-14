/*
  ==============================================================================

    PolyBLEP.h
    Created: 14 Jan 2018 11:00:48am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace juce {
namespace dsp {

// TODO - consider wavetable implementation

/* 
 * Note that these PolyBLEP oscillators assume phase from 0 to 2*PI
 */
template <typename SampleType>        
class PolyBlepOscillator
{
public:

    enum PolyBlepWaveform
    {
        sine = 1,
        saw,
        square,
        triangle
    };
    
    /** The NumericType is the underlying primitive type used by the SampleType (which
        could be either a primitive or vector)
    */
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    PolyBlepOscillator(const PolyBlepWaveform waveformToGenerate)
        : waveform (waveformToGenerate)
    {
        switch (waveform) {
            case sine:
                generator = [] (float x) { return std::sin (x); };
                break;

            case saw:
                generator = [] (float x)    {
                                                auto one = static_cast<NumericType> (1.0);
                                                auto pi = MathConstants<NumericType>::pi;
                                                auto oneOnPi = one / pi;
                                                return (x * oneOnPi) - one;
                                            };
                break;

            case square:
                generator = [] (float x)    {
                                                auto one = static_cast<NumericType> (1.0);
                                                auto pi = MathConstants<NumericType>::pi;
                                                return x < pi ? one : -one;
                                            };
                break;

            case triangle:
                generator = [] (float x)    {
                                                auto one = static_cast<NumericType> (1.0);
                                                auto two = static_cast<NumericType> (2.0);
                                                auto oneOnPi = one / MathConstants<NumericType>::pi;
                                                auto half = static_cast<NumericType> (0.5);
                                                auto y = (x * oneOnPi) - one;
                                                return two * (fabs (y) - half);
                                            };
                break;

            default: ; // Do nothing
        }        
    }
    ~PolyBlepOscillator() 
    { }

    /** Sets the frequency of the oscillator. */
    void setFrequency (NumericType newFrequency, bool force = false) noexcept
    {
        frequency.setValue (newFrequency, force);
    }

    /** Returns the current frequency of the oscillator. */
    NumericType getFrequency() const noexcept
    {
        return frequency.getTargetValue();
    }

    /** Called before processing starts. */
    void prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        phaseBuffer.resize (static_cast<int> (spec.maximumBlockSize));
        incrementBuffer.resize (static_cast<int> (spec.maximumBlockSize));

        oneOnSr = one / sampleRate;

        reset();
    }

    /** Resets the internal state of the oscillator */
    void reset() noexcept
    {
        phase.reset();

        if (sampleRate > 0)
            frequency.reset (sampleRate, 0.05);
    }

    /** Returns the result of processing a single sample. */
    SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType) noexcept
    {
        auto normIncrement  = frequency.getNextValue() / sampleRate;
        auto increment      = normIncrement * MathConstants<NumericType>::twoPi;
        auto ph             = phase.advance (increment);
        auto value          = generator (ph);
        auto t              = ph * oneOnTwoPi;

        if (waveform == saw)
            value -= PolyBLEPSaw (t, normIncrement);
        else if (waveform == square)
        {
            value += PolyBLEPSquare (t, normIncrement);
            value -= PolyBLEPSquare (fmod (t + half, one), normIncrement);
        }
        else if (waveform == triangle)
        {
            value -= PolyBLAMP (t, normIncrement);
            value += PolyBLAMP (fmod (t + half, one), normIncrement);
        }
        return value;
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& outBlock = context.getOutputBlock();
        auto   len      = outBlock.getNumSamples();

        // this is an output-only processory
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));
        jassert (outBlock.getNumSamples() <= static_cast<size_t> (phaseBuffer.size()));

        if (frequency.isSmoothing())
        {
            auto* phBuffer = phaseBuffer.getRawDataPointer();
            auto* incBuffer = incrementBuffer.getRawDataPointer();

            auto* ch0 = outBlock.getChannelPointer (0);
            for (size_t i = 0; i < len; ++i)
            {
                incBuffer[i] = oneOnSr * frequency.getNextValue();
                auto increment = incBuffer[i] * MathConstants<NumericType>::twoPi;
                phBuffer[i] = phase.advance (increment);
                ch0[i] = generator (phBuffer[i]);
                auto t = phBuffer[i] * oneOnTwoPi;
                if (waveform == saw)
                    ch0[i] -= PolyBLEPSaw (t, incBuffer[i]);
                else if (waveform == square)
                {
                    ch0[i] += PolyBLEPSquare (t, incBuffer[i]);
                    ch0[i] -= PolyBLEPSquare (fmod (t + half, one), incBuffer[i]);
                }
                else if (waveform == triangle)
                {
                    ch0[i] -= PolyBLAMP (t, incBuffer[i]);
                    ch0[i] += PolyBLAMP (fmod (t + half, one), incBuffer[i]);
                }
            }

            duplicateOtherChannelsFromFirst (outBlock);
        }
        else
        {
            auto normIncrement = oneOnSr * frequency.getNextValue();
            auto increment = normIncrement * MathConstants<NumericType>::twoPi;
            auto* ch0 = outBlock.getChannelPointer (0);
            for (size_t i = 0; i < len; ++i)
            {
                auto ph = phase.advance (increment);
                ch0[i] = generator (ph);
                auto t = ph * oneOnTwoPi;
                if (waveform == saw)
                    ch0[i] -= PolyBLEPSaw (t, normIncrement);
                else if (waveform == square)
                {
                    ch0[i] += PolyBLEPSquare (t, normIncrement);
                    ch0[i] -= PolyBLEPSquare (fmod (t + half, one), normIncrement);
                }
                else if (waveform == triangle)
                {
                    ch0[i] -= PolyBLAMP (t, normIncrement);
                    ch0[i] += PolyBLAMP (fmod (t + half, one), normIncrement);
                }
            }
           
            duplicateOtherChannelsFromFirst (outBlock);
        }
    }
    
private:

    //// PolyBLEP by Tale (http://www.kvraudio.com/forum/viewtopic.php?t=375517)
    //// Slightly modified by Martin Finke (http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/)
    //// Then adapted by Andrew Jerrim for use in juce::dsp
    ////
    //// Normalised phase = phase / (2 * Pi)
    ////
    //NumericType polyBlep (NumericType t, NumericType dt)
    //{
    //    // 0 <= t < 1
    //    if (t < dt)
    //    {
    //        t /= dt;
    //        return t+t - t*t - one;
    //    }
    //    // -1 < t < 0
    //    else if (t > one - dt)
    //    {
    //        t = (t - one) / dt;
    //        return t*t + t+t + one;
    //    }
    //    // 0 otherwise
    //    else return zero;
    //}

    // The following are adapted from https://github.com/tebjan/VVVV.Audio/blob/master/Source/VVVV.Audio.Signals/Sources/OscSignal.cs
    NumericType PolyBLEPSquare (NumericType t, NumericType dt)
    {
        // 0 <= t < 1
        if (t < dt)
        {
            t = t/dt - one;
            return -t*t;
        }
        // -1 < t < 0
        else if (t > one - dt)
        {
            t = (t - one)/dt + one;
            return t*t;
        }
        // 0 otherwise
        else
        {
            return zero;
        }
    }        
    NumericType PolyBLEPSaw (NumericType t, NumericType dt)
    {
        // 0 <= t < 1
        if (t < dt)
        {
            t /= dt;
            // 2 * (t - t^2/2 - 0.5)
            return t+t - t*t - one;
        }
        // -1 < t < 0
        else if (t > one - dt)
        {
            t = (t - one) / dt;
            // 2 * (t^2/2 + t + 0.5)
            return t*t + t+t + one;
        }
        // 0 otherwise
        else
        {
            return zero;
        }
    }
    NumericType PolyBLAMP (NumericType t, NumericType dt)
    {
        // Adapated to scale the output by 4 * dt
        if (t < dt)
        {
            t = t/dt - one;
            return t * t * t * -oneThird * four * dt;
        }
        else if (t > one - dt)
        {
            t = (t - one)/dt + one;
            return oneThird * t * t * t * four * dt;
        }
        else
        {
            return zero;
        }
    }

    static void duplicateOtherChannelsFromFirst (AudioBlock<NumericType> &block)
    {
        auto src = block.getChannelPointer (0);
        auto numSamples = block.getNumSamples();
        for (auto ch = 1; ch < block.getNumChannels(); ++ch)
        {
            auto dest = block.getChannelPointer (static_cast<size_t> (ch));
            if (typeid(NumericType) == typeid(float))
                FloatVectorOperations::copy (dest, src, static_cast<int> (numSamples));
            else
            {
                for (size_t i = 0; i < numSamples; ++i)
                    dest[i] = src[i];
            }
        }
    }

    PolyBlepWaveform waveform;
    std::function<NumericType (NumericType)> generator;
    Array<NumericType> phaseBuffer, incrementBuffer;
    LinearSmoothedValue<NumericType> frequency { static_cast<NumericType> (440.0) };
    NumericType sampleRate = 48000.0;
    Phase<NumericType> phase;

    NumericType one        = static_cast<NumericType> (1.0);
    NumericType four       = static_cast<NumericType> (4.0);
    NumericType half       = static_cast<NumericType> (0.5);
    NumericType oneThird   = static_cast<NumericType> (1.0/3.0);
    NumericType zero       = static_cast<NumericType> (0.0);
    NumericType oneOnTwoPi = static_cast<NumericType> (1.0) / MathConstants<NumericType>::twoPi;
    NumericType oneOnSr;
};

}   // namespace dsp
}   // namespace juce
