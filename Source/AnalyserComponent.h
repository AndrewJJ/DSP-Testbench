/*
  ==============================================================================

    AnalyserComponent.h
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "FftProcessor.h"
#include "FftScope.h"
#include "AudioScopeProcessor.h"
#include "Oscilloscope.h"
#include "Goniometer.h"
#include "MeteringProcessors.h"
#include "MeteringComponents.h"

class AnalyserComponent final :  public Component, public dsp::ProcessorBase, public Timer
{
public:

    AnalyserComponent();
    ~AnalyserComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    bool isProcessing() const noexcept;
    void activateProcessing();
    void suspendProcessing();

    void showClipStats();

private:

    int getOscilloscopeMaximumBlockSize() const;

    class AnalyserConfigComponent : public Component
    {
    public:
        explicit AnalyserConfigComponent (AnalyserComponent* analyserToConfigure);
        ~AnalyserConfigComponent()
        = default;

        //void paint (Graphics& g) override;
        void resized () override;

    private:
        AnalyserComponent* analyserComponent;
        Label lblFftAggregation;
        ComboBox cmbFftAggregation;
        Label lblFftRelease;
        ComboBox cmbFftRelease;
        Label lblScopeAggregation;
        ComboBox cmbScopeAggregation;
        TextEditor txtHelp;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserConfigComponent);
    };

    String keyName;
    std::unique_ptr<XmlElement> config{};

    Label lblTitle;
    std::unique_ptr<DrawableButton> btnConfig{};
    std::unique_ptr<DrawableButton> btnPause{};
    std::unique_ptr<DrawableButton> btnExpand{};
    std::unique_ptr<AnalyserConfigComponent> configComponent{};

    FftProcessor<12> fftProcessor;
    FftScope<12> fftScope;

    AudioScopeProcessor audioScopeProcessor;
    Oscilloscope oscilloscope;
    Goniometer goniometer;

    PeakMeterProcessor peakMeterProcessor{};
    VUMeterProcessor vuMeterProcessor{};
    MainMeterBackground mainMeterBackground{};
    OwnedArray<MeterBar> peakMeterBars{};
    OwnedArray<MeterBar> vuMeterBars{};
    OwnedArray<ClipIndicatorComponent> clipIndicators{};

    ClipCounterProcessor clipCounterProcessor{};
    ClipStatsComponent clipStatsComponent{};

    int numChannels = 0;

    Atomic<bool> statusActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};