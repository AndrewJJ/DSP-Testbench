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
    dsp::Gain<float> gain;

    bool isLimited() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonitoringComponent)
};