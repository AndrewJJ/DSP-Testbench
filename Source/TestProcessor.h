/*
  ==============================================================================

    TestProcessor.h
    Created: 11 Jan 2018 9:47:24am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/** Inherit from this and implement the virtual methods in order to create a customised TestProcessor.
*/
class TestProcessor : public dsp::ProcessorBase
{
public:
	
    TestProcessor();
	~TestProcessor() = default;

    /** Wrapper method which surrounds the code you want to test */
    void prepare (const dsp::ProcessSpec& spec) override;
    /** Wrapper method which surrounds the code you want to test */
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    /** Wrapper method which surrounds the code you want to test */
    void reset() override;

    dsp::ProcessSpec getCurrentProcessSpec() const;

    /** Returns the average time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryPrepareDurationAverage() const;
    /** Returns the maximum time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryPrepareDurationMax() const;
    /** Returns the minimum time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryPrepareDurationMin() const;
    /** Returns the number of times processToBeTested() has been called since statistics were reset. */
    double queryPrepareDurationNumSamples() const;

    /** Returns the average time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryProcessingDurationAverage() const;
    /** Returns the maximum time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryProcessingDurationMax() const;
    /** Returns the minimum time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryProcessingDurationMin() const;
    /** Returns the number of times processToBeTested() has been called since statistics were reset. */
    double queryProcessingDurationNumSamples() const;

    /** Returns the average time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryResetDurationAverage() const;
    /** Returns the maximum time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryResetDurationMax() const;
    /** Returns the minimum time it takes to run processToBeTested() at the current ProcessSpec. */
    double queryResetDurationMin() const;
    /** Returns the number of times processToBeTested() has been called since statistics were reset. */
    double queryResetDurationNumSamples() const;

    /** Reset statistics */
    void resetStatistics();

protected:

    /** This will be called by the wrapper in order to perform the necessary tests. */
    virtual void prepareToBeTested (const dsp::ProcessSpec& spec) = 0;
    /** This will be called by the wrapper in order to perform the necessary tests. */
    virtual void processToBeTested (const dsp::ProcessContextReplacing<float>& context) = 0;
    /** This will be called by the wrapper in order to perform the necessary tests. */
    virtual void resetToBeTested() = 0;

private:
    	
    dsp::ProcessSpec currentSpec;
    double prepDurationMin = 1.0E100, prepDurationMax = -1.0, prepDurationSum = 0.0, prepDurationCount = 0.0;
    double procDurationMin = 1.0E100, procDurationMax = -1.0, procDurationSum = 0.0, procDurationCount = 0.0;
    double resetDurationMin = 1.0E100, resetDurationMax = -1.0, resetDurationSum = 0.0, resetDurationCount = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestProcessor);
};

/** Returns true if the specified pointer points to 16 byte aligned data. */
static inline bool is_sse_aligned (const float* data);

/** Returns a string describing the buffer alignment status. */
static String get_buffer_alignment_status (AudioBuffer<float>& buffer);
