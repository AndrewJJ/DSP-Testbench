/*
  ==============================================================================

    AboutComponent.h
    Created: 16 Mar 2019 4:17:47pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AboutComponent: public Component
{
public:
    AboutComponent ();
    ~AboutComponent() = default;

    void paint (Graphics& g) override;
    void resized() override;

private:
    ComponentBoundsConstrainer constrainer;
    ResizableBorderComponent resizableBorderComponent;

    ScopedPointer<Label> lblVersion;
    ScopedPointer<TextEditor> txtVersion;
    ScopedPointer<Label> lblCredits;
    ScopedPointer<TextEditor> txtDisclaimer;
    ScopedPointer<Label> lblDisclaimer;
    ScopedPointer<TextEditor> txtCredits;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutComponent)
};