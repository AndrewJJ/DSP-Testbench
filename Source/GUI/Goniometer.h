/*
  ==============================================================================

    Goniometer.h
    Created: 16 Mar 2019 1:13:13pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../Processing/AudioScopeProcessor.h"

class Goniometer final : public Component, public Timer
{
public:

    Goniometer();
    ~Goniometer();

    void paint (Graphics& g) override;
    void resized() override;
    
    // As the frame size for the audioScopeProcessor is set to 4096, updates arrive at ~11 Hz for a sample rate of 44.1 KHz.
    // Instead of repainting on a fixed timer we poll an atomic flag set from the audio thread to see if there is fresh data.
    void timerCallback() override;

    void assignAudioScopeProcessor (AudioScopeProcessor* audioScopeProcessorPtr);

    // Must be called after AudioScopeProcessor:prepare() so that the AudioProbe listeners can be set up properly
    void prepare();

private:

    class Background final : public Component
    {
    public:
        explicit Background (Goniometer* parentGoniometer);
        void paint (Graphics& g) override;
    private:
        Goniometer* parentScope;
    };

    class Foreground final : public Component
    {
    public:
        explicit Foreground (Goniometer* parentGoniometer);
        void paint (Graphics& g) override;
    private:
        Goniometer* parentScope;
    };

    void paintWaveform (Graphics& g) const;
    void paintScale (Graphics& g) const;
    Rectangle<int> getPlotBounds() const;

    Background background;
    Foreground foreground;
	AudioScopeProcessor* audioScopeProcessor;
    AudioBuffer<float> buffer;
    CriticalSection criticalSection;
    ListenerRemovalCallback removeListenerCallback = {};
    WeakReference<Goniometer>::Master masterReference;
    friend class WeakReference<Goniometer>;
    Atomic<bool> dataFrameReady;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Goniometer);
};