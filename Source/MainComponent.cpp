/*
  ==============================================================================

    MainComponent.cpp
    Created: 10 Jan 2018 11:52:07am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MainComponent.h"
#include "Main.h"

void DspTestBenchLnF::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
	const auto radius = static_cast<float> (jmin (width, height)) * 0.48f;
	const auto centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
	const auto centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const auto rx = centreX - radius;
    const auto ry = centreY - radius;
    const auto rw = radius * 2.0f;
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
	const auto isEnabled = slider.isEnabled();
    const auto isMouseOver = slider.isMouseOverOrDragging() && isEnabled;

	const auto fillColour = isEnabled ? Colours::black : Colours::black.brighter();
	const auto outlineColour = isEnabled ? (isMouseOver ? Colours::white.withAlpha (0.7f) : Colours::black) : Colours::grey.darker (0.6f);
    const auto indicatorColour = isEnabled ? Colours::white : Colours::grey;

	// Draw knob body
	{
		Path  p;
		p.addEllipse (rx, ry, rw, rw);
        g.setColour (fillColour);
		g.fillPath (p);
		g.setColour (outlineColour);
		g.strokePath (p, PathStrokeType (radius * 0.075f));
	}

	// Draw rotating pointer
	{
		Path l;
		l.startNewSubPath (0.0f, radius * -0.95f);
		l.lineTo (0.0f, radius * -0.50f);
		g.setColour (indicatorColour);
		g.strokePath (l, PathStrokeType (2.5f), AffineTransform::rotation (angle).translated (centreX, centreY));
	}
}

MainContentComponent::MainContentComponent(AudioDeviceManager& deviceManager)
    : AudioAppComponent (deviceManager),
      customDeviceManager (&deviceManager)
{
    LookAndFeel::setDefaultLookAndFeel (&dspTestBenchLnF);

    addAndMakeVisible (srcComponentA = new SourceComponent ("A", &deviceManager));
    addAndMakeVisible (srcComponentB = new SourceComponent ("B", &deviceManager));
    addAndMakeVisible (procComponentA = new ProcessorComponent ("A", 3));
    addAndMakeVisible (procComponentB = new ProcessorComponent ("B", 3));
    addAndMakeVisible (analyserComponent = new AnalyserComponent());
    addAndMakeVisible (monitoringComponent = new MonitoringComponent(customDeviceManager));

    srcComponentA->setOtherSource (srcComponentB);
    srcComponentB->setOtherSource (srcComponentA);

    // Set small to force resize to minimum resize limit
    setSize (1, 1);

    oglContext.attachTo (*this);

    // Listen for changes to audio device so we can save the state
    customDeviceManager->addChangeListener(this);

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
    //if (dynamic_cast<AudioDeviceManager*>(source) == &deviceManager)
    if (source== &deviceManager)
    {
        std::unique_ptr<XmlElement> xml(deviceManager.createStateXml());
        DSPTestbenchApplication::getApp().appProperties.getUserSettings()->setValue("AudioDeviceState", xml.get());
    }
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
