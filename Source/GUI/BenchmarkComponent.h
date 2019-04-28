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
#include "SourceComponent.h"

class BenchmarkComponent : public Component, public Timer
{
public:

    /** Pass in pointers to both process harnesses and the source component for the audio to into them. */
    BenchmarkComponent (ProcessorHarness* processorHarnessA, ProcessorHarness* processorHarnessB, SourceComponent* sourceComponent);
    ~BenchmarkComponent();
    void paint (Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void setBufferAlignmentStatus (const String &status);

private:

    class BenchmarkThread : public ThreadWithProgressWindow
    {
    public:
        BenchmarkThread (std::vector<ProcessorHarness*>* harnesses, SourceComponent* sourceComponent, BenchmarkComponent* benchmarkComponent);
        ~BenchmarkThread() = default;

        void run() override;
        void threadComplete (bool userPressedCancel) override;

        /** Set number of full test cycles to run (reset, prepare, processing). */
        void setTestCycles (const int cycles);

        /** Set number of times to iterate the processing within each cycle. */
        void setProcessingIterations (const int iterations);
        
        /**< Set ProcessSpec to test against. */
        void setProcessSpec (dsp::ProcessSpec& spec);

    private:

        /** Returns true if the specified pointer points to 16 byte aligned data. */
        static inline bool isSseAligned (const float* data);

        /** Returns a string describing the buffer alignment status. */
        String getAudioBlockAlignmentStatus() const;

        std::vector<ProcessorHarness*>* processingHarnesses{};
        SourceComponent* srcComponent;
        BenchmarkComponent* parent;
        int testCycles = 0;
        int processingIterations = 0;
        dsp::ProcessSpec testSpec {};
        HeapBlock<char> heapBlock{};
        std::unique_ptr<dsp::AudioBlock<float>> audioBlock{};
    };

    int getValueLabelIndex (const int processorIndex, const int routineIndex, const int valueIndex) const;

    OwnedArray<Label> processorLabels{};
    OwnedArray<Label> routineLabels{};
    OwnedArray<Label> valueTitleLabels{};
    OwnedArray<Label> valueLabels{};
    Label lblChannels, lblBlockSize, lblSampleRate, lblCycles, lblIterations, lblBufferAlignmentStatus;
    ComboBox cmbChannels, cmbBlockSize, cmbSampleRate, cmbCycles, cmbIterations;
    TextButton btnStart, btnReset;

    dsp::ProcessSpec spec;

    const std::vector<String> processors = { "Processor A", "Processor B" };
    const std::vector<String> routines = { "Prepare", "Process", "Reset" };
    const std::vector<String> values = { "Min", "Avg", "Max", "#" };
    const std::vector<String> valueTooltips = { "Minimum time for routine (microseconds)", "Average time for routine (microseconds)", "Maximum time for routine (microseconds)", "Number of times this routine was run" };

    std::vector<ProcessorHarness*> harnesses{};
    BenchmarkThread benchmarkThread;
    std::unique_ptr<XmlElement> config {};
    const String keyName = "Benchmarking";
        
    const Font titleFont = Font (GUI_SIZE_F (0.7f)).boldened();
    const Font normalFont = Font (GUI_SIZE_F (0.55f));

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BenchmarkComponent)
};
