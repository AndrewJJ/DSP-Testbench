/*
  ==============================================================================

    ProcessorHarness.cpp
    Created: 11 Jan 2018 9:47:24am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "ProcessorHarness.h"

ProcessorHarness::ProcessorHarness (const int numberOfControlValues)
    : currentSpec ()
{
    for (auto i = 0; i < numberOfControlValues; ++i)
        controlValues.emplace_back (0.0f);
}
void ProcessorHarness::prepareHarness (const dsp::ProcessSpec& spec)
{
    currentSpec = spec;

    const auto start = Time::getMillisecondCounterHiRes();

// =====================
    prepare (spec);
// =====================

    const auto duration = Time::getMillisecondCounterHiRes() - start;
    if (duration<prepDurationMin) prepDurationMin = duration;
    if (duration>prepDurationMax) prepDurationMax = duration;
    prepDurationSum += duration;
    prepDurationCount++;

    // Reset process duration statistics as these will not be valid if the spec has changed
    procDurationMin = 0.0;
    procDurationMax = 0.0;
    procDurationSum = 0.0;
    procDurationCount = 0.0;
}
void ProcessorHarness::processHarness (const dsp::ProcessContextReplacing<float>& context)
{
    const auto start = Time::getMillisecondCounterHiRes();
    
// =====================
    process (context);
// =====================

    const auto duration = Time::getMillisecondCounterHiRes() - start;
    if (duration<procDurationMin) procDurationMin = duration;
    if (duration>procDurationMax) procDurationMax = duration;
    procDurationSum += duration;
    procDurationCount++;
}
void ProcessorHarness::resetHarness ()
{
    const auto start = Time::getMillisecondCounterHiRes();

// =====================
    reset();
// =====================

    const auto duration = Time::getMillisecondCounterHiRes() - start;
    if (duration<resetDurationMin) resetDurationMin = duration;
    if (duration>resetDurationMax) resetDurationMax = duration;
    resetDurationSum += duration;
    resetDurationCount++;
}
void ProcessorHarness::setControlValue(const int index, const float value)
{
    jassert (index >= 0 && index < static_cast<int>(controlValues.size()));
    controlValues[index].set (value);
}
float ProcessorHarness::getControlValue(const int index) const
{
    jassert (index >= 0 && index < static_cast<int>(controlValues.size()));
    return controlValues[index].get();
}
dsp::ProcessSpec ProcessorHarness::getCurrentProcessSpec () const
{
    return currentSpec;
}
double ProcessorHarness::queryPrepareDurationAverage () const
{
    return prepDurationSum / prepDurationCount;
}
double ProcessorHarness::queryPrepareDurationMax () const
{
    return prepDurationMax;
}
double ProcessorHarness::queryPrepareDurationMin() const
{
    return prepDurationMin;
}
double ProcessorHarness::queryPrepareDurationNumSamples () const
{
    return prepDurationCount;
}
double ProcessorHarness::queryProcessingDurationAverage () const
{
    return procDurationSum / procDurationCount;
}
double ProcessorHarness::queryProcessingDurationMax () const
{
    return procDurationMax;
}
double ProcessorHarness::queryProcessingDurationMin() const
{
    return procDurationMin;
}
double ProcessorHarness::queryProcessingDurationNumSamples () const
{
    return procDurationCount;
}
double ProcessorHarness::queryResetDurationAverage () const
{
    return resetDurationSum / resetDurationCount;
}
double ProcessorHarness::queryResetDurationMax () const
{
    return resetDurationMax;
}
double ProcessorHarness::queryResetDurationMin() const
{
    return resetDurationMin;
}
double ProcessorHarness::queryResetDurationNumSamples () const
{
    return resetDurationCount;
}
double ProcessorHarness::queryByIndex (const int routineIndex, const int valueIndex) const
{
    jassert (routineIndex >= 0 && routineIndex < 3);
    jassert (valueIndex >= 0 && valueIndex < 4);
    switch (routineIndex)
    {
        case 0:
            switch (valueIndex)
            {
                case 0: return queryPrepareDurationMin();
                case 1: return queryPrepareDurationAverage();
                case 2: return queryPrepareDurationMax();
                case 3: return queryPrepareDurationNumSamples();
                default: return 0.0;
            }
        case 1:
            switch (valueIndex)
            {
                case 0: return queryProcessingDurationMin();
                case 1: return queryProcessingDurationAverage();
                case 2: return queryProcessingDurationMax();
                case 3: return queryProcessingDurationNumSamples();
                default: return 0.0;
            }
        case 2:
            switch (valueIndex)
            {
                case 0: return queryResetDurationMin();
                case 1: return queryResetDurationAverage();
                case 2: return queryResetDurationMax();
                case 3: return queryResetDurationNumSamples();
                default: return 0.0;
            }
        default: return 0.0;
    }
}
int ProcessorHarness::getQueryIndex (const int routineIndex, const int valueIndex)
{
    jassert (routineIndex >= 0 && routineIndex < 3);
    jassert (valueIndex >= 0 && valueIndex < 4);
    return routineIndex * 4 + valueIndex ;
}
void ProcessorHarness::resetStatistics ()
{
    prepDurationMin = 1.0E100;
    prepDurationMax = -1.0;
    prepDurationSum = 0.0;
    prepDurationCount = 0.0;
    
    procDurationMin = 1.0E100;
    procDurationMax = -1.0;
    procDurationSum = 0.0;
    procDurationCount = 0.0;

    resetDurationMin = 1.0E100;
    resetDurationMax = -1.0;
    resetDurationSum = 0.0;
    resetDurationCount = 0.0;
}


// ======================================================


bool is_sse_aligned (const float* data)
{
    return (reinterpret_cast<uintptr_t>(data) & 0xF) == 0;
}

String get_buffer_alignment_status (AudioBuffer<float>& buffer)
{
    auto bufIsAligned = true;
	for (auto ch = 0; ch < buffer.getNumChannels(); ++ch)
		bufIsAligned = bufIsAligned && is_sse_aligned (buffer.getArrayOfReadPointers ()[ch]);

	String alignmentMsg;
	if (bufIsAligned)
		alignmentMsg = "AudioSampleBuffer is aligned";
	else
		alignmentMsg = "AudioSampleBuffer is not aligned";

	return alignmentMsg;
}