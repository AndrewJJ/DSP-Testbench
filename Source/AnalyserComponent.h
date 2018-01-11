/*
  ==============================================================================

    AnalyserComponent.h
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AnalyserComponent  :  public Component
{
public:

    AnalyserComponent ();
    ~AnalyserComponent();

    void paint (Graphics& g) override;
    void resized() override;

private:

    void toggleMute();

    ScopedPointer<Label> lblTitle;
    ScopedPointer<TextButton> btnMute;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};