/*
  ==============================================================================

    ProcessorComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ProcessorComponent  : public Component,
                            public Slider::Listener
{
public:
    
    ProcessorComponent (const String processorId, const int numberOfControls);
    ~ProcessorComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:
    
    int numControls;

    void toggleSourceA();
    void toggleSourceB();
    void toggleMute();
    void toggleDisable();

    ScopedPointer<Label> lblTitle;
    ScopedPointer<TextButton> btnSourceA;
    ScopedPointer<TextButton> btnSourceB;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TextButton> btnDisable;

    OwnedArray<Label> sliderLabels;
    OwnedArray<Slider> sliders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorComponent)
};