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
    AudioIODevice* cd = deviceManager.getCurrentAudioDevice();
	auto numInputChannels = cd->getActiveInputChannels().countNumberOfSetBits();
    auto numOutputChannels = cd->getActiveOutputChannels().countNumberOfSetBits();
   
    srcBufferA = dsp::AudioBlock<float> (srcBufferMemoryA, numOutputChannels, samplesPerBlockExpected);
    srcBufferB = dsp::AudioBlock<float> (srcBufferMemoryB, numOutputChannels, samplesPerBlockExpected);
    tempBuffer = dsp::AudioBlock<float> (tempBufferMemory, numOutputChannels, samplesPerBlockExpected);

    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlockExpected; // TODO - do we need to inflate this?
    spec.numChannels = numOutputChannels;
    spec.sampleRate = sampleRate;
    
    srcComponentA->prepare (spec);
    srcComponentB->prepare (spec);
    // TODO - prepare processors
    //procComponentA->prepare (spec);
    //procComponentB->prepare (spec);
    // TODO - prepare analysis
    //analyserComponent->prepare (spec);
    // TODO - prepare monitoring
    //monitoringComponent->prepare (spec);
}
void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // TODO - do we need to check buffer size hasn't increased?
    jassert (bufferToFill.numSamples <= srcBufferA.getNumSamples());
    jassert (bufferToFill.numSamples <= srcBufferB.getNumSamples());
    jassert (bufferToFill.numSamples <= tempBuffer.getNumSamples());

    // TODO - do we need a scoped lock?
    dsp::AudioBlock<float> outputBlock (*bufferToFill.buffer, (size_t) bufferToFill.startSample);
    
    // Generate audio from sources
    srcComponentA->process(dsp::ProcessContextReplacing<float> (srcBufferA));
    srcComponentB->process(dsp::ProcessContextReplacing<float> (srcBufferB));

    // Perform processing on Processor A
    if (procComponentA->isProcessorEnabled())
    {
        // TODO - implement this in the processor code - will need to supply two sets of inputs
        // TODO - route signal sources
        if (procComponentA->isSourceConnectedA() && !procComponentA->isSourceConnectedB())
        {
            tempBuffer.copy(srcBufferA); // TODO - remove later
            // TODO - perform processing on source A only
        }
        else if (!procComponentA->isSourceConnectedA() && procComponentA->isSourceConnectedB())
        {
            tempBuffer.copy(srcBufferB); // TODO - remove later
            // TODO - perform processing on source B only
        }
        else if (procComponentA->isSourceConnectedA() && procComponentA->isSourceConnectedB())
        {
            // TODO - sum both sourceA & source B into a temp buffer
            tempBuffer.copy (srcBufferA);
            tempBuffer.add (srcBufferB);
            // TODO - perform processing on both source A & B
        }
        if (!procComponentA->isMuted())
        {
            // TODO - add to output buffer
            outputBlock.add(tempBuffer);
        }
    }

    // TODO - perform processing on Processor B
    
    if (monitoringComponent->isMuted())
        bufferToFill.clearActiveBufferRegion();
    else
    {
        // TODO - apply monitoring gain
        // TODO - apply limiting
    }
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