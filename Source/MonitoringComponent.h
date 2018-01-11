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
                             public Slider::Listener,
                             public Button::Listener
{
public:

    MonitoringComponent ();
    ~MonitoringComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void buttonClicked (Button* buttonThatWasClicked) override;

private:

    ScopedPointer<Label> lblMonitoring;
    ScopedPointer<Label> lblGain;
    ScopedPointer<Slider> sldOutputGain;
    ScopedPointer<ToggleButton> btnLimiter;
    ScopedPointer<TextButton> btnMuteLeft;
    ScopedPointer<TextButton> btnMuteRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonitoringComponent)
};