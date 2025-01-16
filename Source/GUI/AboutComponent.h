/*
  ==============================================================================

    AboutComponent.h
    Created: 16 Mar 2019 4:17:47pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "LookAndFeel.h"

class AboutComponent : public Component
{
public:

    AboutComponent();
    ~AboutComponent() override = default;
    void paint (Graphics& g) override;
    void resized() override;

private:

    using cols = DspTestBenchLnF::ApplicationColours;

    void addAboutText();
    void insertTitle (const String& title, const Colour textColour = cols::titleFontColour(), const bool insertLineBefore = true);
    void insertSubtitle (const String& subtitle, const Colour textColour = cols::titleFontColour(), const bool insertLineBefore = true);
    void insertText (const String& text, const bool breakLine = false, const Colour textColour = cols::normalFontColour());
    void insertBreak (const float height = 0.2f);
    void insertBullet (const bool insertBreakBefore = true);
    void insertCopyright (const String& copyrightOwner, const int year);

    TextEditor txtEditor;    
    const Font titleFont = Font (FontOptions (GUI_SIZE_F (0.85)));
    const Font subtitleFont = Font (FontOptions (GUI_SIZE_F (0.65)));
    const Font textFont = Font (FontOptions (GUI_SIZE_F (0.60)));

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutComponent)
};