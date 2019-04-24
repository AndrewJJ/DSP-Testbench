/*
  ==============================================================================

    MeteringComponents.h
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../Processing/MeteringProcessors.h"

/** 
*  A 3 colour vertical meter bar component with dB levels mapped exponentially.
*/
class MeterBar final : public Component, public SettableTooltipClient
{
public:
	
    MeterBar();
    MeterBar (
        const float minimumLevelDb,
        const float cautionLevelDb,
        const float alertLevelDb,
        const float maximumLevelDb
    );
    ~MeterBar() = default;

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
    float scaling = 1.0f;
    float currentLevelDb = -150.0f;
    float currentLevelY = 0.0f;
    Colour backgroundColour = Colours::black;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterBar)
};

/**
 *  Background for the main meter in the AnalyserComponent. *  
 */
class MainMeterBackground : public Component
{
public:
    MainMeterBackground();
    ~MainMeterBackground() = default;

    void paint (Graphics& g) override;
    void resized() override;

    // Set the number of channels
    void setNumChannels (const int numberOfChannels);

    Grid::Px getDesiredWidth() const;
    Rectangle<int> getMeterBarBoundsInParent (const int channel, const bool isPeakMeter) const;
    Rectangle<int> getClipIndicatorBoundsInParent (const int channel) const;
    float getScaleMax() const;
    float getScaleMin() const;

private:
    int numChannels = 0;
    Rectangle<int> clientArea{};
    int channelWidth = 0;
    int getChannelLeft (const int channel, const int offset = 0) const;
    void drawScale(Graphics& g) const;

    const int desiredBarWidth = GUI_BASE_SIZE_I;
    const int gap = GUI_BASE_GAP_I;
    const int dBScaleWidth = GUI_SIZE_I (0.9);
    const int clipIndicatorHeight = GUI_SIZE_I (0.5);
    const float topMargin = static_cast<float> (gap) * 0.3f;
    const float bottomMargin = static_cast<float> (gap) * 1.5f;

    const float scaleMax = 6.0f;
    const float scaleMin = -60.0f;
	const float scaleStep = 6.0f;
    const float scaleSpan = scaleMax - scaleMin;
    const float maxExp = dsp::FastMathApproximations::exp (scaleMax / scaleSpan);
    const float minExp = dsp::FastMathApproximations::exp (scaleMin / scaleSpan);
    float scaling = 0.0f;
};

/**
 *  A component which displays statistics relating to clipping of the audio signal.
 */
class ClipStatsComponent final : public Component
{
public:
    ClipStatsComponent();
    ~ClipStatsComponent() = default;

    void paint (Graphics&) override;
    void resized() override;

    // Set the number of channels
    void setNumChannels (const int numberOfChannels);

    //  Assign the processor that this component will reference
    void assignProcessor (ClipCounterProcessor* clipCounterProcessor);

    //  Update the statistics displayed in this component from the referenced processor
    void updateStats();

    int getMinWidth() const;
    int getDesiredWidth() const;
    int getDesiredHeight() const;

private:

    int numChannels = 0;
    ClipCounterProcessor* processor{};
    const double headingWidth = 3.7;
    const double spacerWidth = 2.5;
    const double channelWidth = 1.8;
    const double rowHeight = 0.6;
    const double gap = 1.0;

    Label lblClippedSamplesTitle;
    Label lblClipEventsTitle;
    Label lblAvgEventLengthTitle;
    Label lblMaxEventLengthTitle;
    TextButton btnReset;

    OwnedArray<Label> lblChannelHeadings{};
    OwnedArray<Label> lblClippedSamples{};
    OwnedArray<Label> lblClipEvents{};
    OwnedArray<Label> lblAvgEventLength{};
    OwnedArray<Label> lblMaxEventLength{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipStatsComponent)
};

/**
 *  Displays clip indicator relating to clipping of the audio signal
 */
class ClipIndicatorComponent final : public Component, public TooltipClient
{
public:
    ClipIndicatorComponent (const int channel, ClipCounterProcessor* clipCounterProcessorToReferTo);
    ~ClipIndicatorComponent() = default;
    
    void paint (Graphics& g) override;
    void mouseDown (const MouseEvent& event) override;
    String getTooltip() override;

private:
    int channelNumber = 0;
    ClipCounterProcessor* clipCounterProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipIndicatorComponent)
};