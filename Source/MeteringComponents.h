/*
  ==============================================================================

    MeteringComponents.h
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/** 
*  A 3 colour vertical meter bar component with dB levels mapped exponentially.
*/
class MeterBar final : public Component, public SettableTooltipClient
{
public:
	
    MeterBar();
    MeterBar (const float minimumLevelDb, const float cautionLevelDb,
                               const float alertLevelDb, const float maximumLevelDb);
    ~MeterBar();

    void paint (Graphics&) override;
    void resized() override;

    void setLevel (const float dB);
    void setMinDb (const float dB);
    void setMaxDb (const float dB);
    void setBackgroundColour (const Colour newBackgroundColour);

private:

    float dBtoPx (const float dB) const;

    float minDb       = -60.0f;
    float cautionDb   = -12.0f;
    float alertDb     = -6.0f;
    float maxDb       = 6.0f;
    float rangeDb     = maxDb - minDb;
    float maxExp      = 0.0f;
    float cautionY {};
    float alertY {};
    float conversionFactor = 1.0f;
    float currentLevelDb = -150.0f;
    float currentLevelY = 0.0f;
    Colour backgroundColour = Colours::black;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterBar)
};

/**
 *  Background for the main meter in the AnalyserComponent.
 *  
 */
class MainMeterBackground : public Component
{
public:
    MainMeterBackground();
    ~MainMeterBackground() = default;

    void paint (Graphics& g) override;
    void resized() override;
    Grid::Px getDesiredWidth (const int numChannels) const;
    Rectangle<int> getMeterBarBoundsInParent (const int channel, const int numChannels, const bool isPeakMeter) const;
    float getScaleMax() const;
    float getScaleMin() const;

private:
    Rectangle<int> getBarMeterAreaInParent() const;
    Rectangle<int> getBarMeterArea() const;
    void drawScale(Graphics& g) const;

    int desiredBarWidth = GUI_BASE_SIZE_I;
    int gap = GUI_BASE_GAP_I;
    int dBScaleWidth = GUI_SIZE_I (0.9);

    const float scaleMax = 6.0f;
    const float scaleMin = -60.0f;
	const float scaleStep = 6.0f;
};
