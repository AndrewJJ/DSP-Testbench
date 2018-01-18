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
class SimpleLevelMeterComponent    : public Component
{
public:
	
    /** Color indices used for drawing meters on the GUI */
	enum ColourIds
    {
        outlineColourId			= 0x99600001,	/**< Colour ID for the outline colour. */
        backgroundColourId		= 0x99600002,	/**< Colour ID for the background colour. */
        foregroundColourId		= 0x99600003,	/**< Colour ID for the foreground colour. */
		meterBarNormalColourId	= 0x99600004,	/**< Colour ID for colour of the meter bar when the signal is in the *normal* range. */
		meterBarCautionColourId	= 0x99600005,	/**< Colour ID for colour of the meter bar when the signal is in the *caution* range. */
		meterBarAlertColourId	= 0x99600006	/**< Colour ID for colour of the meter bar when the signal is in the *alert* range. */
    };

    SimpleLevelMeterComponent();
    SimpleLevelMeterComponent (const float minimumLevelDb, const float cautionLevelDb,
                               const float alertLevelDb, const float maximumLevelDb);
    ~SimpleLevelMeterComponent();

    void paint (Graphics&) override;
    void resized() override;

    void setLevel (const float dB);

private:

    float dBtoPx (const float dB) const;

    float minDb, cautionDb, alertDb, maxDb, rangeDb;
    float cautionY, alertY;
    float conversionFactor = 1.0f;
    float currentLevelY = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleLevelMeterComponent)
};
