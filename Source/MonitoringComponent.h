/*
  ==============================================================================

    MonitoringComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MonitoringComponent final : public Component, public dsp::ProcessorBase
{
public:

    explicit MonitoringComponent (AudioDeviceManager* audioDeviceManager);
    ~MonitoringComponent();

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

    Label lblTitle;
    Slider sldGain;
    TextButton btnLimiter;
    TextButton btnMute;

    bool statusLimiter;
    bool statusMute;
    dsp::Gain<float> monitoringGain;
    double sampleRate;
    const float limiterCeiling = Decibels::decibelsToGain (-0.2f); // -6..0 dB range
    const float limiterThreshold = Decibels::decibelsToGain (-0.5f); // -30..0 dB range
    const float limiterRange = limiterCeiling / limiterThreshold;
    const float limiterRelease = 200.0f / 1000.0f; // 0..500 range for numerator
    float limiterHoldTime;
    float limiterReleaseTimer1;
    float limiterReleaseTimer2;
    float limiterMax1;
    float limiterMax2;
    float limiterEnvelope;
    float limiterReleaseFactor;
    
    bool isLimited() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonitoringComponent)
};