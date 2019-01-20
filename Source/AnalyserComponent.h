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
#include "MeteringProcessors.h"
#include "SimpleLevelMeterComponent.h"

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
    void reset () override;

    bool isActive() const noexcept;

    // TODO - build synch trigger functionality throughout the app (hosted in MainComponent::getNextAudioBlock()
    // TODO - add RMS & VU metering?
    // TODO - add clearable clip indicator?
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
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserConfigComponent);
    };

    class MeterBackground : public Component
    {
    public:
        MeterBackground();
        ~MeterBackground() = default;

        void paint (Graphics& g) override;
        void resized() override;
        Grid::Px getDesiredWidth (const int numChannels) const;
        Rectangle<int> getBarBoundsInParent (const int channel, const int numChannels) const;
        float getScaleMax() const;
        float getScaleMin() const;

    private:
        Rectangle<int> getBarMeterAreaInParent() const;
        Rectangle<int> getBarMeterArea() const;
        void drawScale(Graphics& g) const;

        int desiredBarWidth = GUI_BASE_SIZE_I;
        int gap = GUI_BASE_GAP_I;
        int dBScaleWidth = GUI_SIZE_I(1.2);

        const float scaleMax = 0.0f;
        const float scaleMin = -100.0f;
	    const float stepSize = 10.0f;
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

    SimplePeakMeterProcessor peakMeterProcessor;
    MeterBackground meterBackground;
    OwnedArray<SimplePeakMeterComponent> meterBars;
    int numChannels = 0;

    Atomic<bool> statusActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};