/*
  ==============================================================================

    SimpleLevelMeterComponent.h
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/** 
*  A 3 colour vertical meter bar component.
*  
*  The 3 colours are used to represent the normal, caution, and alert signal ranges. Alert
*  would typically be used to indicate the onset of signal clipping, while caution is a warning
*  level ahead of that.
*/
class SimplePeakMeterComponent : public Component, public SettableTooltipClient
{
public:
	
    SimplePeakMeterComponent();
    SimplePeakMeterComponent (const float minimumLevelDb, const float cautionLevelDb,
                               const float alertLevelDb, const float maximumLevelDb);
    ~SimplePeakMeterComponent();

    void paint (Graphics&) override;
    void resized() override;

    void setLevel (const float dB);

private:

    float dBtoPx (const float dB) const;

    float minDb       = -50.0f;
    float cautionDb   = -15.0f;
    float alertDb     = -3.0f;
    float maxDb       = 0.0f;
    float rangeDb     = maxDb - minDb;
    float cautionY, alertY;
    float conversionFactor = 1.0f;
    float currentLevelDb = -100.0f;
    float currentLevelY = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplePeakMeterComponent)
};
