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

// TODO - triangle aliases during sweeping
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
            case triangle: 
                generator = [] (float x)    {
                                                auto one = static_cast<NumericType> (1.0);
                                                auto pi = MathConstants<NumericType>::pi;
                                                return x < pi ? one : -one;
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
        auto normIncrement = frequency.getNextValue() / sampleRate;
        auto increment = normIncrement * MathConstants<NumericType>::twoPi;
        auto ph = phase.advance (increment);
        auto value = generator (ph);
        auto t = ph / MathConstants<NumericType>::twoPi;
        if (waveform == saw)
            value -= poly_blep (t, normIncrement);
        else if (waveform == square || waveform == triangle)
        {
            value += poly_blep (t, normIncrement);
            value -= poly_blep (fmod (t + half, one), normIncrement);
        }
        if (waveform == triangle)
            value = increment * value + (1 - increment) * lastValue; // leaky integrator
        lastValue = value;
        return value;
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& outBlock = context.getOutputBlock();

        // this is an output-only processory
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));
        jassert (outBlock.getNumSamples() <= static_cast<size_t> (phaseBuffer.size()));

        auto len           = outBlock.getNumSamples();
        auto one           = static_cast<NumericType> (1.0);
        auto half          = static_cast<NumericType> (0.5);
        auto zero          = static_cast<NumericType> (0.0);
        auto oneOnTwoPi    = static_cast<NumericType> (1.0) / MathConstants<NumericType>::twoPi;
        auto oneOnSr       = one / sampleRate;

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
                    ch0[i] -= poly_blep (t, incBuffer[i]);
                else if (waveform == square || waveform == triangle)
                {
                    ch0[i] += poly_blep (t, incBuffer[i]);
                    ch0[i] -= poly_blep (fmod (t + half, one), incBuffer[i]);
                }
                if (waveform == triangle)
                    //ch0[i] = increment * ch0[i] + (one - increment) * lastValue; // leaky integrator
                    // TODO - this can be unstable if sweeping near Nyquist, so apply tanh to keep it under control (still aliases while sweeping)
                    ch0[i] = tanh (increment * ch0[i] + (one - increment) * lastValue); // leaky integrator
                lastValue = ch0[i];
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
                    ch0[i] -= poly_blep (t, normIncrement);
                else if (waveform == square || waveform == triangle)
                {
                    ch0[i] += poly_blep (t, normIncrement);
                    ch0[i] -= poly_blep (fmod (t + half, one), normIncrement);
                }
                if (waveform == triangle)
                    ch0[i] = increment * ch0[i] + (one - increment) * lastValue; // leaky integrator
                lastValue = ch0[i];
            }
           
            duplicateOtherChannelsFromFirst (outBlock);
        }
    }
    
private:

    // PolyBLEP by Tale (http://www.kvraudio.com/forum/viewtopic.php?t=375517)
    // Slightly modified by Martin Finke (http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/)
    // Then adapted by Andrew Jerrim for use in juce::dsp
    //
    // Normalised phase = phase / (2 * Pi)
    //
    NumericType poly_blep(NumericType normalisedPhase, NumericType normalisedPhaseIncrement)
    {
        auto one = static_cast<NumericType> (1.0);
        // 0 <= t < 1
        if (normalisedPhase < normalisedPhaseIncrement)
        {
            normalisedPhase /= normalisedPhaseIncrement;
            return normalisedPhase+normalisedPhase - normalisedPhase*normalisedPhase - one;
        }
        // -1 < t < 0
        else if (normalisedPhase > one - normalisedPhaseIncrement)
        {
            normalisedPhase = (normalisedPhase - one) / normalisedPhaseIncrement;
            return normalisedPhase*normalisedPhase + normalisedPhase+normalisedPhase + one;
        }
        // 0 otherwise
        else return static_cast<NumericType> (0.0);
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
    NumericType lastValue = 0.0;
};

}   // namespace dsp
}   // namespace juce
