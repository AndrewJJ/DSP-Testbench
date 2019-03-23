/*
  ==============================================================================

    Oscilloscope.h
    Created: 4 Feb 2018 1:40:28pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "AudioScopeProcessor.h"

class Oscilloscope final : public Component, public Timer
{
public:

    /** Defines the method of aggregation used if a number of values fall within the same x pixel value) */
    enum AggregationMethod
    {
        NearestSample = 1,  // This offers the best performance, but shows variable amplitude for higher frequency content
        Maximum,            // This better represents the envelope of higher frequency content - uses max amplitude (i.e. takes absolute value)
        Average             // This tends to show even lower amplitudes for higher frequency content than the NearestSample method
    };

    Oscilloscope();
    ~Oscilloscope();

    void paint (Graphics& g) override;
    void resized() override;
    void mouseDown (const MouseEvent& event) override;
    void mouseDrag (const MouseEvent& event) override;
    void mouseDoubleClick (const MouseEvent& event) override;
    void mouseMove (const MouseEvent& event) override;
    void mouseExit (const MouseEvent& event) override;
    void mouseWheelMove (const MouseEvent& event, const MouseWheelDetails& wheel) override;
    
    // As the frame size for the audioScopeProcessor is set to 4096, updates arrive at ~11 Hz for a sample rate of 44.1 KHz.
    // Instead of repainting on a fixed timer we poll an atomic flag set from the audio thread to see if there is fresh data.
    void timerCallback() override;

    void assignAudioScopeProcessor (AudioScopeProcessor* audioScopeProcessorPtr);

    // Must be called after AudioScopeProcessor:prepare() so that the AudioProbe listeners can be set up properly
    void prepare();

    // Set maximum amplitude scale for y-axis (defaults to 1.0 otherwise)
    void setMaxAmplitude (const float maximumAmplitude);
    float getMaxAmplitude() const;

    // Set minimum time value for x-axis in samples (defaults to 0 otherwise)
    void setXMin (const int minimumX);
    int getXMin() const;

    // Set maximum time value for x-axis in samples (defaults to max buffer size otherwise)
    // Will be limited to max buffer size if set too high
    void setXMax (const int maximumX);
    int getXMax() const;

    // Get the maximum block size (as defined by the processor)
    int getMaximumBlockSize() const;

    /** Get aggregation method for sub-pixel x values */
    AggregationMethod getAggregationMethod() const;

    /** Set aggregation method for sub-pixel x values (otherwise initialised to maximum) */
    void setAggregationMethod (const AggregationMethod method);

    /** Allows mouse moves over this component to trigger repaints. This enables cursor co-ordinates to be painted even if audio has been suspended. */
    void setMouseMoveRepaintEnablement (const bool enableRepaints);

private:

    class Background final : public Component
    {
    public:
        explicit Background (Oscilloscope* parentOscilloscope);
        void paint (Graphics& g) override;
    private:
        Oscilloscope* parentScope;
    };

    class Foreground final : public Component
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
    void preCalculateVariables();

    Background background;
    Foreground foreground;
	AudioScopeProcessor* audioScopeProcessor;
    
    float amplitudeMax = 1.0f;
    int minXSamples = 0;
    int maxXSamples = 0;
    float xRatio = 1.0f;
    float yRatio = 1.0f;
    float xRatioInv = 1.0f;
    float yRatioInv = 1.0f;
    int currentX = -1;
    int currentY = -1;
    AggregationMethod aggregationMethod = AggregationMethod::NearestSample;
    bool mouseMoveRepaintsEnabled = false;
    const int controlSize = GUI_SIZE_I(0.6);
    Rectangle<int> xAxisControlArea;
    Rectangle<int> yAxisControlArea;
    int xMinAtLastMouseDown = 0;
    int xMaxAtLastMouseDown = 0;
    MouseCursor zoomCursor;

    AudioBuffer<float> buffer;
    CriticalSection criticalSection;

    ListenerRemovalCallback removeListenerCallback = {};
    WeakReference<Oscilloscope>::Master masterReference;
    friend class WeakReference<Oscilloscope>;

    Atomic<bool> dataFrameReady;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscilloscope);
};