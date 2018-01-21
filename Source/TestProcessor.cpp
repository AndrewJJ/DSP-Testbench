/*
  ==============================================================================

    TestProcessor.cpp
    Created: 11 Jan 2018 9:47:24am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "TestProcessor.h"

TestProcessor::TestProcessor()
    : currentSpec ()
{ }
void TestProcessor::prepare (const dsp::ProcessSpec& spec)
{
    currentSpec = spec;

    const auto start = Time::getMillisecondCounterHiRes();

// =====================
    prepareToBeTested(spec);
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
void TestProcessor::process (const dsp::ProcessContextReplacing<float>& context)
{
    const auto start = Time::getMillisecondCounterHiRes();
    
// =====================
    processToBeTested (context);
// =====================

    const auto duration = Time::getMillisecondCounterHiRes() - start;
    if (duration<procDurationMin) procDurationMin = duration;
    if (duration>procDurationMax) procDurationMax = duration;
    procDurationSum += duration;
    procDurationCount++;
}
void TestProcessor::reset ()
{
    const auto start = Time::getMillisecondCounterHiRes();

// =====================
    resetToBeTested();
// =====================

    const auto duration = Time::getMillisecondCounterHiRes() - start;
    if (duration<resetDurationMin) resetDurationMin = duration;
    if (duration>resetDurationMax) resetDurationMax = duration;
    resetDurationSum += duration;
    resetDurationCount++;
}
dsp::ProcessSpec TestProcessor::getCurrentProcessSpec () const
{
    return currentSpec;
}
double TestProcessor::queryPrepareDurationAverage () const
{
    return prepDurationSum / prepDurationCount;
}
double TestProcessor::queryPrepareDurationMax () const
{
    return prepDurationMax;
}
double TestProcessor::queryPrepareDurationMin() const
{
    return prepDurationMin;
}
double TestProcessor::queryPrepareDurationNumSamples () const
{
    return prepDurationCount;
}
double TestProcessor::queryProcessingDurationAverage () const
{
    return procDurationSum / procDurationCount;
}
double TestProcessor::queryProcessingDurationMax () const
{
    return procDurationMax;
}
double TestProcessor::queryProcessingDurationMin() const
{
    return procDurationMin;
}
double TestProcessor::queryProcessingDurationNumSamples () const
{
    return procDurationCount;
}
double TestProcessor::queryResetDurationAverage () const
{
    return resetDurationSum / resetDurationCount;
}
double TestProcessor::queryResetDurationMax () const
{
    return resetDurationMax;
}
double TestProcessor::queryResetDurationMin() const
{
    return resetDurationMin;
}
double TestProcessor::queryResetDurationNumSamples () const
{
    return resetDurationCount;
}
void TestProcessor::resetStatistics ()
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


bool is_sse_aligned (const float* data)
{
    return (reinterpret_cast<uintptr_t>(data) & 0xF) == 0;
}

String get_buffer_alignment_status (AudioBuffer<float>& buffer)
{
    auto bufIsAligned = true;
	for (auto ch = 0; ch < buffer.getNumChannels(); ++ch)
		bufIsAligned = bufIsAligned && is_sse_aligned (buffer.getArrayOfReadPointers ()[ch]);

	String alignmentMsg = "AudioSampleBuffer should ";
	if (bufIsAligned)
		alignmentMsg = "AudioSampleBuffer is aligned";
	else
		alignmentMsg = "AudioSampleBuffer is not aligned";

	return alignmentMsg;
}
