/*
  ==============================================================================

    MainComponent.cpp
    Created: 10 Jan 2018 11:52:07am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    addAndMakeVisible (srcComponentA = new SourceComponent ("A"));
    addAndMakeVisible (srcComponentB = new SourceComponent ("B"));
    addAndMakeVisible (procComponentA = new ProcessorComponent ("A", 3));
    addAndMakeVisible (procComponentB = new ProcessorComponent ("B", 3));
    addAndMakeVisible (analyserComponent = new AnalyserComponent());
    addAndMakeVisible (monitoringComponent = new MonitoringComponent());

    srcComponentA->setOtherSource (srcComponentB);
    srcComponentB->setOtherSource (srcComponentA);
    procComponentB->mute();
    
    setSize (1024, 768);
    oglContext.attachTo (*this);

    // specify the number of input and output channels that we want to open
    setAudioChannels (2, 2);
}
MainContentComponent::~MainContentComponent()
{
    shutdownAudio();

    srcComponentA = nullptr;
    srcComponentB = nullptr;
    procComponentA = nullptr;
    procComponentB = nullptr;
    analyserComponent = nullptr;
    monitoringComponent = nullptr;
}
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    AudioIODevice* currentDevice = deviceManager.getCurrentAudioDevice();
	auto numInputChannels = currentDevice->getActiveInputChannels().countNumberOfSetBits();
    auto numOutputChannels = currentDevice->getActiveOutputChannels().countNumberOfSetBits();
   
    srcBufferA = dsp::AudioBlock<float> (srcBufferMemoryA, numOutputChannels, samplesPerBlockExpected);
    srcBufferB = dsp::AudioBlock<float> (srcBufferMemoryB, numOutputChannels, samplesPerBlockExpected);
    tempBuffer = dsp::AudioBlock<float> (tempBufferMemory, numOutputChannels, samplesPerBlockExpected);

    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = jmax (numInputChannels, numOutputChannels);
    spec.sampleRate = sampleRate;
    
    srcComponentA->setNumChannels (numInputChannels, numOutputChannels);
    srcComponentB->setNumChannels (numInputChannels, numOutputChannels);
    srcComponentA->prepare (spec);
    srcComponentB->prepare (spec);
    procComponentA->prepare (spec);
    procComponentB->prepare (spec);
    analyserComponent->prepare (spec);
    monitoringComponent->prepare (spec);
}
void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // If these asserts fail then we need to handle larger than expected buffer sizes
    jassert (bufferToFill.numSamples <= srcBufferA.getNumSamples());
    jassert (bufferToFill.numSamples <= srcBufferB.getNumSamples());
    jassert (bufferToFill.numSamples <= tempBuffer.getNumSamples());

    dsp::AudioBlock<float> outputBlock (*bufferToFill.buffer, (size_t) bufferToFill.startSample);
    
    // Copy current block into source buffers if needed
    if (srcComponentA->getMode() == SourceComponent::Mode::AudioIn)
        srcBufferA.copy (outputBlock);
    if (srcComponentB->getMode() == SourceComponent::Mode::AudioIn)
        srcBufferB.copy (outputBlock);

    // Generate audio from sources
    srcComponentA->process(dsp::ProcessContextReplacing<float> (srcBufferA));
    srcComponentB->process(dsp::ProcessContextReplacing<float> (srcBufferB));

    // Run audio through processors
    if (procComponentA->isActive())
    {
        routeSourcesAndProcess (procComponentA, tempBuffer);
        outputBlock.copy (tempBuffer);
        if (procComponentB->isActive()) // both active
        {
            routeSourcesAndProcess (procComponentB, tempBuffer);
            outputBlock.add (tempBuffer);
        }
    }
    else if (procComponentB->isActive()) // processor A inactive
    {
        routeSourcesAndProcess (procComponentB, tempBuffer);
        outputBlock.copy (tempBuffer);
    }
    else // neither is active
        outputBlock.clear();

    // Run audio through analyser (note that the analyser isn't expected to alter the outputBlock)
    if (analyserComponent->isActive())
        analyserComponent->process (dsp::ProcessContextReplacing<float> (outputBlock));

    // Run audio through monitoring section
    if (monitoringComponent->isMuted())
        outputBlock.clear();
    else
        monitoringComponent->process (dsp::ProcessContextReplacing<float> (outputBlock));
}
void MainContentComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}
void MainContentComponent::paint (Graphics& g)
{
    //g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.fillAll (Colour (0xff323e44));
}
void MainContentComponent::resized()
{
    Grid grid;
    grid.rowGap = 10_px;
    grid.columnGap = 10_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (3_fr),
                            Track (2_fr),
                            Track (4_fr),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (srcComponentA),
                            GridItem (srcComponentB),
                            GridItem (procComponentA),
                            GridItem (procComponentB),
                            GridItem (analyserComponent).withArea({ }, GridItem::Span (2)),
                            GridItem (monitoringComponent).withArea({ }, GridItem::Span (2))
                        });

    const auto marg = 10;
    // .withTrimmedTop(proportionOfHeight(0.1f))
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void MainContentComponent::routeSourcesAndProcess (ProcessorComponent* processor, dsp::AudioBlock<float>& temporaryBuffer)
{
    if (processor->isProcessorEnabled())
    {
        // Route signal sources
        if (processor->isSourceConnectedA())
        {
            temporaryBuffer.copy(srcBufferA);
            if (processor->isSourceConnectedB()) // both sources connected
                temporaryBuffer.add (srcBufferB);
        }
        else if (processor->isSourceConnectedB()) // source A not connected
            temporaryBuffer.copy(srcBufferB);
        else // Neither source is connected
            temporaryBuffer.clear(); 
        
        // Perform processing
        // TODO - consider not encapsulating process methods in ProcessorComponents (in which case would need to pass a pointer to correct processor object)
        processor->process (dsp::ProcessContextReplacing<float> (temporaryBuffer));
        
        // Invert processor output as appropriate
        if (processor->isInverted())
            temporaryBuffer.multiply(-1.0f);
    }
}
