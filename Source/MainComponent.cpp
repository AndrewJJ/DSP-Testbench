/*
  ==============================================================================

    MainComponent.cpp
    Created: 10 Jan 2018 11:52:07am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MainComponent.h"
#include "Main.h"

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
    
    // Set small to force resize to minimum resize limit
    setSize (1, 1);

    oglContext.attachTo (*this);

    // specify the number of input and output channels that we want to open
    setAudioChannels (2, 2);
}
MainContentComponent::~MainContentComponent()  // NOLINT
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
    const auto currentDevice = deviceManager.getCurrentAudioDevice();
	const auto numInputChannels = currentDevice->getActiveInputChannels().countNumberOfSetBits();
    const auto numOutputChannels = currentDevice->getActiveOutputChannels().countNumberOfSetBits();
   
    srcBufferA = dsp::AudioBlock<float> (srcBufferMemoryA, numOutputChannels, samplesPerBlockExpected);
    srcBufferB = dsp::AudioBlock<float> (srcBufferMemoryB, numOutputChannels, samplesPerBlockExpected);
    tempBuffer = dsp::AudioBlock<float> (tempBufferMemory, numOutputChannels, samplesPerBlockExpected);

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = jmax (numInputChannels, numOutputChannels);
    
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
    const auto srcComponentHeight = Grid::Px (jmax (srcComponentA->getMinimumHeight(), srcComponentB->getMinimumHeight()));
    const auto procComponentHeight = Grid::Px (jmax (procComponentA->getMinimumHeight(), procComponentB->getMinimumHeight()));
    const auto monitoringComponentHeight = Grid::Px (monitoringComponent->getMinimumHeight());

    // Assume both source components have the same width and that this is also sufficient for the processor components
    const auto srcWidth = srcComponentA->getMinimumWidth();

    Grid grid;
    grid.rowGap = GUI_GAP_PX(2);
    grid.columnGap = GUI_GAP_PX(2);

    using Track = Grid::TrackInfo;

    if (srcWidth * 4 < getWidth())
    {
        // Put sources and processors on first row
        grid.templateRows = {   Track (srcComponentHeight),
                                Track (1_fr),
                                Track (monitoringComponentHeight)
                            };

        grid.templateColumns = { Track (Grid::Px (srcWidth)), Track (Grid::Px (srcWidth)), Track (Grid::Px (srcWidth)), Track (1_fr) };

        grid.items.addArray({   GridItem (srcComponentA), 
                                GridItem (srcComponentB),
                                GridItem (procComponentA),
                                GridItem (procComponentB).withWidth(srcWidth), // set width to prevent stretching
                                GridItem (analyserComponent).withArea ({ }, GridItem::Span (4)),
                                GridItem (monitoringComponent).withArea ({ }, GridItem::Span (4))
                            });
    }
    else
    {
        // First row is sources, second row is processors
        grid.templateRows = {   Track (srcComponentHeight),
                                Track (procComponentHeight),
                                Track (1_fr),
                                Track (monitoringComponentHeight)
                            };

        grid.templateColumns = { Track (Grid::Px (srcWidth)), Track (1_fr) };

        grid.items.addArray({   GridItem (srcComponentA), 
                                GridItem (srcComponentB).withWidth(srcWidth), // set width to prevent stretching
                                GridItem (procComponentA),
                                GridItem (procComponentB).withWidth(srcWidth), // set width to prevent stretching
                                GridItem (analyserComponent).withArea ({ }, GridItem::Span (2)),
                                GridItem (monitoringComponent).withArea ({}, GridItem::Span (2))
                            });
    }

    grid.autoFlow = Grid::AutoFlow::row;


    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
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
