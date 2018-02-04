/*
  ==============================================================================

    Oscilloscope.h
    Created: 4 Feb 2018 1:40:28pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "OscilloscopeProcessor.h"

class Oscilloscope : public Component, public AudioProbe <OscilloscopeProcessor::OscilloscopeFrame>::Listener
{
public:

    /** Defines the method of aggregation used if a number of values fall within the same x pixel value) */
    enum AggregationMethod
    {
        maximum = 1,
        average
    };

    Oscilloscope ();
    ~Oscilloscope ();

    void paint (Graphics& g) override;
    void resized() override;
    void mouseMove(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;

    void assignOscProcessor (OscilloscopeProcessor* oscProcessorPtr);
    void audioProbeUpdated (AudioProbe<OscilloscopeProcessor::OscilloscopeFrame>* audioProbe) override;

    // Set maximum amplitude scale for y-axis (defaults to 1.0 otherwise)
    void setMaxAmplitude (const float maximumAmplitude);
    float getMaxAmplitude() const;

    // Set minimum time value for x-axis (defaults to 10Hz otherwise)
    void setTimeMin (const int minimumTime);
    int getTimeMin() const;

    // Set maximum time value for x-axis (defaults to max buffer size otherwise)
    // Will be limited to max buffer size if set too high
    void setTimeMax (const int maximumTime);
    int getTimeMax() const;

    /** Set aggregation method for sub-pixel x values (otherwise initialised to maximum) */
    void setAggregationMethod (const AggregationMethod method);

    // TODO - implement one-shot mode
    // TODO - look for performance improvements or do drawing on separate thread to prevent message queue from being choked

private:

    class Background : public Component
    {
    public:
        explicit Background (Oscilloscope* parentOscilloscope);
        void paint (Graphics& g) override;
    private:
        Oscilloscope* parentScope;
    };

    class Foreground : public Component
    {
    public:
        explicit Foreground (Oscilloscope* parentOscilloscope);
        void paint (Graphics& g) override;
    private:
        Oscilloscope* parentScope;
    };

    void paintWaveform (Graphics& g) const;
    void paintScale (Graphics& g) const;

    inline float toAmpFromPx (const float yInPixels) const;
    inline float toPxFromAmp (const float amplitude) const;
    inline int toTimeFromPx (const float xInPixels) const;
    inline float toPxFromTime (const int xInSamples) const;

    static Colour getColourForChannel (const int channel);
    void initialise();

    Background background;
    Foreground foreground;
	OscilloscopeProcessor* oscProcessor;
    HeapBlock<float> y;

    const int maxNumSamples = 8192; // Needs to match the processor
    float amplitudeMax = 1.0f;
    int minTime = 0;
    int maxTime = maxNumSamples;
    float xRatio = 1.0f;
    float yRatio = 1.0f;
    float xRatioInv = 1.0f;
    float yRatioInv = 1.0f;
    int currentX = -1;
    int currentY = -1;
    AggregationMethod aggregationMethod = AggregationMethod::maximum;
};