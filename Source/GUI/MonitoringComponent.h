/*
  ==============================================================================

    MonitoringComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ProcessorComponent.h"

class MonitoringComponent final : public Component, public dsp::ProcessorBase
{
public:

    explicit MonitoringComponent (AudioDeviceManager* audioDeviceManager, ProcessorComponent* processorA, ProcessorComponent* processorB);
    ~MonitoringComponent() override;

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    bool isMuted() const;

private:

    AudioDeviceManager* deviceManager;
    String keyName;
    std::unique_ptr<XmlElement> config;
    ProcessorComponent* processorComponentA;
    ProcessorComponent* processorComponentB;

    Label lblTitle;
    Slider sldGain;
    TextButton btnCompare;
    TextButton btnLimiter;
    TextButton btnMute;

    bool statusLimiter;
    bool statusMute;
    dsp::Gain<float> monitoringGain;
    double sampleRate{};
    const float limiterCeiling = Decibels::decibelsToGain (-0.2f);      //  -6 .. 0 dB range
    const float limiterThreshold = Decibels::decibelsToGain (-0.5f);    // -30 .. 0 dB range
    const float limiterRange = limiterCeiling / limiterThreshold;
    const float limiterRelease = 0.2f;                                  // 0.0 .. 0.5 range
    float limiterHoldTime{};
    float limiterReleaseTimer1{};
    float limiterReleaseTimer2{};
    float limiterMax1{};
    float limiterMax2{};
    float limiterEnvelope{};
    float limiterReleaseFactor{};
    
    bool isLimited() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonitoringComponent)
};