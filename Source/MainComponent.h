/*
  ==============================================================================

    MainComponent.h
    Created: 12 Jan 2018 11:52:07am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SourceComponent.h"
#include "ProcessorComponent.h"
#include "MonitoringComponent.h"
#include "AnalyserComponent.h"

class DspTestBenchLnF : public LookAndFeel_V4
{
public:

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
};

class MainContentComponent : public AudioAppComponent, public ChangeListener
{
public:

    MainContentComponent(AudioDeviceManager& deviceManager);
    ~MainContentComponent();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void paint (Graphics& g) override;
    void resized() override;

    void changeListenerCallback (ChangeBroadcaster* source) override;

private:

    DspTestBenchLnF dspTestBenchLnF;
    OpenGLContext oglContext;
    AudioDeviceManager* customDeviceManager;

    ScopedPointer<SourceComponent> srcComponentA;
    ScopedPointer<SourceComponent> srcComponentB;
    ScopedPointer<ProcessorComponent> procComponentA{};
    ScopedPointer<ProcessorComponent> procComponentB{};
    ScopedPointer<AnalyserComponent> analyserComponent{};
    ScopedPointer<MonitoringComponent> monitoringComponent{};

    HeapBlock<char> srcBufferMemoryA{}, srcBufferMemoryB{}, tempBufferMemory{};
    dsp::AudioBlock<float> srcBufferA, srcBufferB, tempBuffer;

    void routeSourcesAndProcess (ProcessorComponent* processor, dsp::AudioBlock<float>&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};