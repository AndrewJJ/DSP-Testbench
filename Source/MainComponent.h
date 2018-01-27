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
    void mouseDown(const MouseEvent&) override
    {
        srcComponentA->reset();

        AudioIODevice* currentDevice = deviceManager.getCurrentAudioDevice();
        dsp::ProcessSpec spec;
        spec.maximumBlockSize = currentDevice->getCurrentBufferSizeSamples();
        spec.numChannels = currentDevice->getActiveOutputChannels().countNumberOfSetBits();
        spec.sampleRate = currentDevice->getCurrentSampleRate();
        srcComponentA->prepare (spec);
    };

private:

    OpenGLContext oglContext;

    ScopedPointer<SourceComponent> srcComponentA;
    ScopedPointer<SourceComponent> srcComponentB;
    ScopedPointer<ProcessorComponent> procComponentA;
    ScopedPointer<ProcessorComponent> procComponentB;
    ScopedPointer<AnalyserComponent> analyserComponent;
    ScopedPointer<MonitoringComponent> monitoringComponent;

    HeapBlock<char> srcBufferMemoryA{}, srcBufferMemoryB{}, tempBufferMemory{};
    dsp::AudioBlock<float> srcBufferA, srcBufferB, tempBuffer;

    void routeSourcesAndProcess (ProcessorComponent* processor, dsp::AudioBlock<float>&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};