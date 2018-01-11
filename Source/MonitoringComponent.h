/*
  ==============================================================================

    MonitoringComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MonitoringComponent  : public Component,
                             public Slider::Listener
{
public:

    MonitoringComponent ();
    ~MonitoringComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:

    void toggleLimiter();
    void toggleMute();

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldOutputGain;
    ScopedPointer<TextButton> btnLimiter;
    ScopedPointer<TextButton> btnMute;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonitoringComponent)
};