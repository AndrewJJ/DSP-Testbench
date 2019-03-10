/*
  ==============================================================================

    MainComponent.cpp
    Created: 10 Jan 2018 11:52:07am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MainComponent.h"
#include "Main.h"

MainContentComponent::MainContentComponent(AudioDeviceManager& deviceManager)
    : AudioAppComponent (deviceManager)
{
    holdAudio.set (false);

    addAndMakeVisible (srcComponentA = new SourceComponent ("A", &deviceManager));
    addAndMakeVisible (srcComponentB = new SourceComponent ("B", &deviceManager));
    addAndMakeVisible (procComponentA = new ProcessorComponent ("A", 3));
    addAndMakeVisible (procComponentB = new ProcessorComponent ("B", 3));
    addAndMakeVisible (analyserComponent = new AnalyserComponent());
    addAndMakeVisible (monitoringComponent = new MonitoringComponent(&deviceManager));

    srcComponentA->setOtherSource (srcComponentB);
    srcComponentB->setOtherSource (srcComponentA);

    // Set small to force resize to minimum resize limit
    setSize (1, 1);

    oglContext.attachTo (*this);

    // Listen for changes to audio device so we can save the state
    deviceManager.addChangeListener(this);

    // Read saved audio device state from user settings
    std::unique_ptr<XmlElement> savedAudioDeviceState (DSPTestbenchApplication::getApp().appProperties.getUserSettings()->getXmlValue("AudioDeviceState"));

    // Specify the number of input and output channels that we want to open
    setAudioChannels (2, 2, savedAudioDeviceState.get());
}
MainContentComponent::~MainContentComponent()  // NOLINT
{
    srcComponentA = nullptr;
    srcComponentB = nullptr;
    procComponentA = nullptr;
    procComponentB = nullptr;
    analyserComponent = nullptr;
    monitoringComponent = nullptr;
}
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    sampleCounter.set(0);
    // Oscilloscope and FftScope use 4096 sample frames
    // If we want to have a bigger scope buffer then we would need to devise a fancier hold mechanism inside the AnalyserComponent
    holdSize.set (4096);

    const auto currentDevice = deviceManager.getCurrentAudioDevice();
	const auto numInputChannels = static_cast<uint32> (currentDevice->getActiveInputChannels().countNumberOfSetBits());
    const auto numOutputChannels = static_cast<uint32> (currentDevice->getActiveOutputChannels().countNumberOfSetBits());

    srcBufferA = dsp::AudioBlock<float> (srcBufferMemoryA, numOutputChannels, samplesPerBlockExpected);
    srcBufferB = dsp::AudioBlock<float> (srcBufferMemoryB, numOutputChannels, samplesPerBlockExpected);
    tempBuffer = dsp::AudioBlock<float> (tempBufferMemory, numOutputChannels, samplesPerBlockExpected);

    const dsp::ProcessSpec spec {
        sampleRate,
        static_cast<uint32> (samplesPerBlockExpected),
        jmax (numInputChannels, numOutputChannels)
    };
    
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

    dsp::AudioBlock<float> outputBlock (*bufferToFill.buffer, static_cast<size_t>(bufferToFill.startSample));
    
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

    if (holdAudio.get())
    {
        sampleCounter.set (sampleCounter.get() + bufferToFill.numSamples);
        if (sampleCounter.get() > holdSize.get())
        {
            // Close audio device from another thread (note that addJob isn't usually safe on the audio thread - but we're closing it anyway!)
            threadPool.addJob ([this] { deviceManager.closeAudioDevice(); });
        }
    }
}
void MainContentComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.
    srcBufferA.clear();
    srcBufferB.clear();
    tempBuffer.clear();
}
void MainContentComponent::paint (Graphics& g)
{
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

    const auto margin = GUI_GAP_I(2);
    auto requiredWidthForSourcesAndProcessors = srcWidth * 4;
    requiredWidthForSourcesAndProcessors += GUI_GAP_I(2) * 3; // Gaps between components
    requiredWidthForSourcesAndProcessors += margin * 2; // Gaps at edges

    if (requiredWidthForSourcesAndProcessors < getWidth())
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

    grid.performLayout (getLocalBounds().reduced (margin, margin));
}
void MainContentComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &deviceManager)
    {
        std::unique_ptr<XmlElement> xml(deviceManager.createStateXml());
        DSPTestbenchApplication::getApp().appProperties.getUserSettings()->setValue("AudioDeviceState", xml.get());
    }
}
void MainContentComponent::triggerSnapshot ()
{
    deviceManager.closeAudioDevice();
    
    // Reset components to ensure consistent behaviour for hold function
    srcComponentA->reset();
    srcComponentB->reset();
    procComponentA->reset();
    procComponentB->reset();
    analyserComponent->reset();
    monitoringComponent->reset();

    // Set a flag & sample counter so we can stop the device again once a certain number of samples have been processed        
    holdAudio.set (true);
    sampleCounter.set (0);

    // Note that restarting the audio device will cause prepare to be called
    deviceManager.restartLastAudioDevice();
}
void MainContentComponent::resumeStreaming()
{
    holdAudio.set (false);
    deviceManager.restartLastAudioDevice();
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
