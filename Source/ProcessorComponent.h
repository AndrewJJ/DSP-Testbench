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

    bool isSourceConnectedA() const;
    bool isSourceConnectedB() const;
    bool isMuted() const;
    bool isProcessorEnabled() const;

private:
    
    int numControls;

    ScopedPointer<Label> lblTitle;
    ScopedPointer<TextButton> btnSourceA;
    ScopedPointer<TextButton> btnSourceB;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TextButton> btnDisable;

    OwnedArray<Label> sliderLabels;
    OwnedArray<Slider> sliders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorComponent)
};