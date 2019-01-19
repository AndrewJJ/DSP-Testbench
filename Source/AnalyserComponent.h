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
#include "OscilloscopeProcessor.h"
#include "Oscilloscope.h"

class AnalyserComponent final :  public Component, public dsp::ProcessorBase
{
public:

    AnalyserComponent();
    ~AnalyserComponent();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    bool isActive() const noexcept;

    // TODO - build synch trigger functionality throughout the app (hosted in MainComponent::getNextAudioBlock()
    // TODO - add detailed metering (peak, RMS, VU)
    // TODO - add phase scope

private:

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
        Label lblScopeAggregation;
        ComboBox cmbScopeAggregation;
        Label lblScopeScaleX;
        Slider sldScopeScaleX;
        Label lblScopeScaleY;
        Slider sldScopeScaleY;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalyserConfigComponent);
    };

    String keyName;
    std::unique_ptr<XmlElement> config;

    Label lblTitle;
    TextButton btnConfig;
    TextButton btnDisable;
    std::unique_ptr<AnalyserConfigComponent> configComponent;

    FftProcessor<12> fftMult;
    FftScope<12> fftScope;

    OscilloscopeProcessor oscProcessor;
    Oscilloscope oscilloscope;

    Atomic<bool> statusActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};