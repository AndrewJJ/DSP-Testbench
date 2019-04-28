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

    /** You will need to override this to return the name of your processor. */
    virtual String getProcessorName() = 0;

    /** You will need to override this to return the names of your controls. */
    virtual String getControlName (const int index) = 0;

    /** You will need to override this to return the default values of your controls.
     *  NOTE - the controls are floats in the range 0..1.
     */
    virtual double getDefaultControlValue (const int index) = 0;
    
    // =================================================================================================================================

    /** Harness which wraps your prepare() function. */
    void prepareHarness (const dsp::ProcessSpec& spec);

    /** Harness which wraps your process() method. */
    void processHarness (const dsp::ProcessContextReplacing<float>& context);

    /** Harness which wraps your reset() method. */
    void resetHarness();


    /** Returns the number of control values for this processor. */
    int getNumControls() const;

    /** Set control value (0..1). */
    void setControlValue (const int index, const double value);
    
    /** Get control value (0..1). */
    double getControlValue (const int index) const;

    /** Get control value as float (0..1). */
    float getControlValueAsFloat (const int index) const;


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

    std::vector <Atomic<double>> controlValues;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorHarness);
};