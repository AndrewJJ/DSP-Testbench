/*
  ==============================================================================

    MainComponent.h
    Created: 12 Jan 2018 11:52:07am
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SourceComponent.h"
#include "ProcessorComponent.h"
#include "MonitoringComponent.h"
#include "AnalyserComponent.h"

class MainContentComponent : public AudioAppComponent
{
public:

    MainContentComponent();
    ~MainContentComponent();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint (Graphics& g) override;
    void resized() override;

private:

    OpenGLContext oglContext;

    ScopedPointer<SourceComponent> srcComponentA;
    ScopedPointer<SourceComponent> srcComponentB;
    ScopedPointer<ProcessorComponent> procComponentA;
    ScopedPointer<ProcessorComponent> procComponentB;
    ScopedPointer<AnalyserComponent> analyserComponent;
    ScopedPointer<MonitoringComponent> monitoringComponent;

    HeapBlock<char> srcBufferMemoryA, srcBufferMemoryB, tempBufferMemory;
    dsp::AudioBlock<float> srcBufferA, srcBufferB, tempBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

// (This function is called by the app startup code to create our main component)
// TODO - remove
//Component* createMainContentComponent()     { return new MainContentComponent(); }
