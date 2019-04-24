/*
  ==============================================================================

    ProcessorHarness.h
    Created: 11 Jan 2018 9:47:24am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/** 
 * Inherit from this and implement the virtual methods in order to create a customised ProcessorHarness.
 */
class ProcessorHarness : public dsp::ProcessorBase
{
public:
	
    ProcessorHarness (const int numberOfControlValues);
	~ProcessorHarness() = default;

    // =================================================================================================================================

    /** You will need to override this prepare() method with your own code. */
    void prepare (const dsp::ProcessSpec& spec) override = 0;

    /** You will need to override this process() method with your own code. */
    void process (const dsp::ProcessContextReplacing<float>& context) override = 0;

    /** You will need to override this reset() method with your own code. */
    void reset() override = 0;

    /** You will need to override this to return the names of your controls. */
    virtual String getControlName (const int index) = 0;

    /** You will need to override this to return the default values of your controls.
     *  NOTE - the controls are floats in the range 0..1.
     */
    virtual float getDefaultControlValue (const int index) = 0;
    
    // =================================================================================================================================

    /** Harness which wraps your prepare() function. */
    void prepareHarness (const dsp::ProcessSpec& spec);

    /** Harness which wraps your process() method. */
    void processHarness (const dsp::ProcessContextReplacing<float>& context);

    /** Harness which wraps your reset() method. */
    void resetHarness();


    /** Set control value (0..1). */
    void setControlValue (const int index, const float value);
    
    /** Get control value (0..1). */
    float getControlValue (const int index) const;


    dsp::ProcessSpec getCurrentProcessSpec() const;


    /** Returns the average time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryPrepareDurationAverage() const;

    /** Returns the maximum time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryPrepareDurationMax() const;

    /** Returns the minimum time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryPrepareDurationMin() const;

    /** Returns the number of times processToBeTested() has been called since statistics were reset. */
    double queryPrepareDurationNumSamples() const;


    /** Returns the average time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryProcessingDurationAverage() const;
    
    /** Returns the maximum time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryProcessingDurationMax() const;
    
    /** Returns the minimum time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryProcessingDurationMin() const;
    
    /** Returns the number of times processToBeTested() has been called since statistics were reset. */
    double queryProcessingDurationNumSamples() const;

    
    /** Returns the average time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryResetDurationAverage() const;
    
    /** Returns the maximum time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryResetDurationMax() const;
    
    /** Returns the minimum time it takes to run processToBeTested() at the current ProcessSpec (in milliseconds). */
    double queryResetDurationMin() const;
    
    /** Returns the number of times processToBeTested() has been called since statistics were reset. */
    double queryResetDurationNumSamples() const;

    
    /** Utility function to query performance by routine & value indices (returns time statistics in milliseconds). */
    double queryByIndex (const int routineIndex, const int valueIndex) const;

    /** Utility function to determine query index according to routine & value indices (returns time statistics in milliseconds). */
    static int getQueryIndex (const int routineIndex, const int valueIndex);


    /** Reset statistics */
    void resetStatistics();

private:
    	
    dsp::ProcessSpec currentSpec;
    double prepDurationMin = 1.0E100, prepDurationMax = -1.0, prepDurationSum = 0.0, prepDurationCount = 0.0;
    double procDurationMin = 1.0E100, procDurationMax = -1.0, procDurationSum = 0.0, procDurationCount = 0.0;
    double resetDurationMin = 1.0E100, resetDurationMax = -1.0, resetDurationSum = 0.0, resetDurationCount = 0.0;
    
    std::vector <Atomic<float>> controlValues;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorHarness);
};


// ======================================================


/** Returns true if the specified pointer points to 16 byte aligned data. */
static inline bool is_sse_aligned (const float* data);

/** Returns a string describing the buffer alignment status. */
static String get_buffer_alignment_status (AudioBuffer<float>& buffer);