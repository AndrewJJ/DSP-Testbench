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

template <typename SampleType>
class PolyBlepOscillator final
{
public:

    /** List of waveforms which can be generated by this oscillator. */
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

    /** Creates an oscillator. If lookup table is non-zero, then the sine function will be
        approximated with a lookup table. The other waveforms will not use the lookup table.
    */
    PolyBlepOscillator (const PolyBlepWaveform waveformToGenerate, const size_t lookupTableNumPoints = 0)
        : waveform (waveformToGenerate)
    {
        // Note that these PolyBLEP oscillators implement phase from 0 to 2*PI (rather than -Pi to Pi)
       
        switch (waveform) {

            case sine:
                generator = [] (NumericType x)  { return std::sin (x); };
                break;

            case saw:
                generator = [=] (NumericType x) { return (x * oneOnPi) - one; };
                break;

            case square:
                generator = [=] (NumericType x) { return x < MathConstants<NumericType>::pi ? one : -one; };
                break;

            case triangle:
                generator = [=] (NumericType x) { return two * (fabs ((x * oneOnPi) - one) - half); };
                break;

            default: ; // Do nothing
        }        

        initialise (lookupTableNumPoints);
    }

    // Note that we do not define a default constructor as that means copy-assignment operator won't be generated
    // by some compilers and therefore list initialisation won't work.

    /** Sets the frequency of the oscillator. */
    void setFrequency (const NumericType newFrequency, const bool force = false) noexcept
    {
        if (force)
                frequency.setCurrentAndTargetValue (newFrequency);
        else
            frequency.setTargetValue (newFrequency);
    }

    /** Returns the current frequency of the oscillator. */
    [[nodiscard]] NumericType getFrequency() const noexcept
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

        if (waveform == sine)
            return value;

        // Add PolyBLEP/BLAMP corrections
        auto t = ph * oneOnTwoPi;
        if (waveform == saw)
            value -= PolyBLEPSaw (t, normIncrement);
        else if (waveform == square)
        {
            value += PolyBLEPSquare (t, normIncrement);
            value -= PolyBLEPSquare (fmod (t + half, one), normIncrement);
        }
        else // if (waveform == triangle) // avoid the unnecessary comparison
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

        // this is an output-only processor
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));
        jassert (outBlock.getNumSamples() <= static_cast<size_t> (phaseBuffer.size()));

        auto len = outBlock.getNumSamples();

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

                if (waveform == sine)
                    continue;

                // Add PolyBLEP/BLAMP corrections
                auto t = phBuffer[i] * oneOnTwoPi;
                if (waveform == saw)
                    ch0[i] -= PolyBLEPSaw (t, incBuffer[i]);
                else if (waveform == square)
                {
                    ch0[i] += PolyBLEPSquare (t, incBuffer[i]);
                    ch0[i] -= PolyBLEPSquare (fmod (t + half, one), incBuffer[i]);
                }
                else //if (waveform == triangle) // avoid the unnecessary comparison
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

                if (waveform == sine)
                    continue;

                // Add PolyBLEP/BLAMP corrections
                auto t = ph * oneOnTwoPi;
                if (waveform == saw)
                    ch0[i] -= PolyBLEPSaw (t, normIncrement);
                else if (waveform == square)
                {
                    ch0[i] += PolyBLEPSquare (t, normIncrement);
                    ch0[i] -= PolyBLEPSquare (fmod (t + half, one), normIncrement);
                }
                else //if (waveform == triangle) // avoid the unnecessary comparison
                {
                    ch0[i] -= PolyBLAMP (t, normIncrement);
                    ch0[i] += PolyBLAMP (fmod (t + half, one), normIncrement);
                }
            }
           
            duplicateOtherChannelsFromFirst (outBlock);
        }
    }
    
private:

    /** Initialises the oscillator with a waveform. */
    void initialise (size_t lookupTableNumPoints = 0)
    {
        // Only implement lookup table for sine wave (the other waveforms are cheap to
        // calculate anyway). Note that the lookup table approach works fine for triangle and
        // sawtooth, but produces unpleasant artefacts for square waves (perhaps because 
        // the transition doesn't exactly match where the PolyBLEP is applied).
        //
        // Draft code to modify generator for square wave to apply PolyBLEP corrections to lookup table.
        // Note that the following code doesn't work (and if you try it you need to comment out the corrections in the process methods
        //if (waveform == square)
        //{
        //    std::function<NumericType (NumericType)> naive = generator;
        //    auto dt = one / (lookupTableNumPoints);
        //    generator = [=] (NumericType x) {
        //        auto value = naive (x);
        //        auto t = x * oneOnTwoPi;
        //        // Add PolyBLEP corrections to generator
        //        value += PolyBLEPSquare (t, dt);
        //        value -= PolyBLEPSquare (fmod (t + half, one), dt);
        //        return value;
        //    };
        //}
        //
        if (waveform == sine && lookupTableNumPoints != 0)
        {
            // Note that the period is shifted Pi ahead of what is used by the original juce::dsp::Oscillator class
            lookupTable = std::make_unique<LookupTableTransform<NumericType>> (generator, zero, MathConstants<NumericType>::twoPi, lookupTableNumPoints);
            auto* table = lookupTable.get();
            generator = [table] (NumericType x) { return (*table) (x); };
        }
    }

    // The following are adapted from - https://github.com/tebjan/VVVV.Audio/blob/master/Source/VVVV.Audio.Signals/Sources/OscSignal.cs
    // See here for a simple explanation of the technique - http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/)
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
        // Adapted to scale the output by 4 * dt
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
    std::unique_ptr<LookupTableTransform<NumericType>> lookupTable;
    Array<NumericType> phaseBuffer, incrementBuffer;
    LinearSmoothedValue<NumericType> frequency { static_cast<NumericType> (440.0) };
    NumericType sampleRate = 48000.0;
    Phase<NumericType> phase;

    // These are defined to reduce code warnings and/or to avoid repetitive divide operations
    NumericType zero       = static_cast<NumericType> (0.0);
    NumericType one        = static_cast<NumericType> (1.0);
    NumericType two        = static_cast<NumericType> (2.0);
    NumericType four       = static_cast<NumericType> (4.0);
    NumericType half       = static_cast<NumericType> (0.5);
    NumericType oneThird   = one / static_cast<NumericType> (3.0);
    NumericType oneOnPi    = one / MathConstants<NumericType>::pi;
    NumericType oneOnTwoPi = one / MathConstants<NumericType>::twoPi;
    NumericType oneOnSr;
};

}   // namespace dsp
}   // namespace juce
