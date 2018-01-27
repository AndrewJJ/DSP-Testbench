/*
  ==============================================================================

    MonitoringComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MonitoringComponent  : public Component, public Slider::Listener, public dsp::ProcessorBase
{
public:

    MonitoringComponent ();
    ~MonitoringComponent();

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();

    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    bool isMuted() const;

private:

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldGain;
    ScopedPointer<TextButton> btnLimiter;
    ScopedPointer<TextButton> btnMute;
    
    bool statusLimiter = true;
    bool statusMute = false;
    dsp::Gain<float> gain;

    bool isLimited() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonitoringComponent)
};