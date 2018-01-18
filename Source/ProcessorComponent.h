/*
  ==============================================================================

    ProcessorComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ProcessorComponent  : public Component, public Slider::Listener, dsp::ProcessorBase
{
public:
    
    ProcessorComponent (const String processorId, const int numberOfControls);
    ~ProcessorComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    bool isSourceConnectedA() const;
    bool isSourceConnectedB() const;
    bool isProcessorEnabled() const;
    bool isInverted() const;
    bool isMuted() const;
    // Returns true if this processor is producing audio
    bool isActive() const;
    void mute();

private:
    
    int numControls;

    ScopedPointer<Label> lblTitle;
    ScopedPointer<TextButton> btnSourceA;
    ScopedPointer<TextButton> btnSourceB;
    ScopedPointer<TextButton> btnDisable;
    ScopedPointer<TextButton> btnInvert;
    ScopedPointer<TextButton> btnMute;
    // TODO - add a bypass button to allow direct analysis of a source?

    bool statusSourceA = true;
    bool statusSourceB = false;
    bool statusDisable = false;
    bool statusInvert = false;
    bool statusMute = false;

    OwnedArray<Label> sliderLabels;
    OwnedArray<Slider> sliders;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorComponent)
};