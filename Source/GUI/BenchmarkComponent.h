/*
  ==============================================================================

    BenchmarkComponent.h
    Created: 24 Apr 2019 9:26:12am
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../Processing/ProcessorHarness.h"

class BenchmarkComponent : public Component, public Timer
{
public:

    BenchmarkComponent (ProcessorHarness* processorHarnessA, ProcessorHarness* processorHarnessB);
    ~BenchmarkComponent() = default;
    void paint (Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:

    int getValueLabelIndex (const int processorIndex, const int routineIndex, const int valueIndex) const;

    OwnedArray<Label> processorLabels;
    OwnedArray<Label> routineLabels;
    OwnedArray<Label> valueTitleLabels;
    OwnedArray<Label> valueLabels;

    const std::vector<String> processors = { "Processor A", "Processor B" };
    const std::vector<String> routines = { "Prepare", "Process", "Reset" };
    const std::vector<String> values = { "Min", "Avg", "Max", "#" };
    const std::vector<String> valueTooltips = { "Minimum time for routine (msec)", "Average time for routine (msec)", "Maximum time for routine (msec)", "Number of times this routine was run" };

    std::vector<ProcessorHarness*> harnesses;
    
    const Font titleFont = Font (GUI_SIZE_F (0.7f)).boldened();
    const Font normalFont = Font (GUI_SIZE_F (0.55f));

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BenchmarkComponent)
};