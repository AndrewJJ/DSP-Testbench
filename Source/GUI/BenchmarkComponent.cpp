/*
  ==============================================================================

    BenchmarkComponent.cpp
    Created: 24 Apr 2019 9:26:12am
    Author:  Andrew

  ==============================================================================
*/

#include "BenchmarkComponent.h"
#include "../Main.h"

BenchmarkComponent::BenchmarkComponent (ProcessorHarness* processorHarnessA,
                                        ProcessorHarness* processorHarnessB,
                                        SourceComponent* sourceComponent)
    : spec (),
      benchmarkThread (&harnesses, sourceComponent)
{
    benchmarkThread.setPriority (Thread::realtimeAudioPriority);

    harnesses.emplace_back (processorHarnessA);
    harnesses.emplace_back (processorHarnessB);
    if (processorHarnessA) processorHarnessA->resetStatistics();
    if (processorHarnessB) processorHarnessB->resetStatistics();

    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config.reset (propertiesFile->getXmlValue (keyName));
    if (!config)
        config.reset(new XmlElement (keyName));

    // Initialise labels for showing results
    using cols = DspTestBenchLnF::ApplicationColours;
    jassert (values.size() == valueTooltips.size());
    for (const auto& p : processors)
    {
        auto* lblP = processorLabels.add (new Label ("", p));
        lblP->setFont (titleFont);
        lblP->setColour (Label::backgroundColourId, cols::benchmarkHeadingBackground());
        lblP->setColour (Label::textColourId, cols::titleFontColour());
        addAndMakeVisible (lblP);
        for (const auto& r : routines)
        {
            auto* lblR = routineLabels.add (new Label ("", r));
            lblR->setFont (normalFont);
            if (r == "Process")
                lblR->setColour (Label::backgroundColourId, cols::benchmarkAlternateRow());
            else
                lblR->setColour (Label::backgroundColourId, cols::benchmarkRow());
            lblR->setColour (Label::textColourId, cols::titleFontColour());
            addAndMakeVisible (lblR);
            for (auto v = 0; v < static_cast<int> (values.size()); ++v)
            {
                auto* lblV = valueLabels.add (new Label ("", "-"));
                lblV->setFont (normalFont);
                if (r == "Process")
                    lblV->setColour (Label::backgroundColourId, cols::benchmarkAlternateRow());
                else
                    lblV->setColour (Label::backgroundColourId, cols::benchmarkRow());
                lblV->setJustificationType (Justification::centred);
                lblR->setColour (Label::textColourId, cols::normalFontColour());
                lblV->setTooltip (valueTooltips[v]);
                addAndMakeVisible (lblV);
            }
        }
        for (auto v = 0; v < static_cast<int> (values.size()); ++v)
        {
            auto* lblVT = valueTitleLabels.add (new Label ("", values[v]));
            lblVT->setTooltip (valueTooltips[v]);
            lblVT->setFont (normalFont);
            lblVT->setColour (Label::textColourId, cols::titleFontColour());
            lblVT->setColour (Label::backgroundColourId, cols::benchmarkHeadingBackground());
            lblVT->setJustificationType (Justification::centred);
            addAndMakeVisible (lblVT);
        }
    }

    lblBlockSize.setText ("Block size", dontSendNotification);
    lblBlockSize.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblBlockSize);
    cmbBlockSize.setTooltip ("This allows you to assess overhead, loop unrolling optimisations, etc. - but note that accuracy can be poor for very short processing times (e.g. single channel at a small block size)");
    for (auto i = 1; i < 9; ++i)
    {
        const auto id = static_cast<int> (pow (2, i + 4));
        cmbBlockSize.addItem (String (id), id);
    }
    cmbBlockSize.onChange = [this] { spec.maximumBlockSize = cmbBlockSize.getSelectedId(); };
    cmbBlockSize.setSelectedId (config->getIntAttribute ("BlockSize", 1024));
    addAndMakeVisible (cmbBlockSize);

    lblChannels.setText ("Channels", dontSendNotification);
    lblChannels.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblChannels);
    for (auto i = 1; i < 6; ++i)
    {
        const auto id = static_cast<int> (pow (2, i - 1));
        cmbChannels.addItem (String (id), id);
    }
    cmbChannels.onChange = [this] { spec.numChannels = cmbChannels.getSelectedId(); };
    cmbChannels.setSelectedId (config->getIntAttribute ("NumChannels", 2));
    addAndMakeVisible (cmbChannels);

    lblSampleRate.setText ("Sample rate", dontSendNotification);
    lblSampleRate.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblSampleRate);
    cmbSampleRate.addItem ("32000", 32000);
    cmbSampleRate.addItem ("44100", 44100);
    cmbSampleRate.addItem ("48000", 48000);
    cmbSampleRate.addItem ("96000", 96000);
    cmbSampleRate.addItem ("192000", 192000);
    cmbSampleRate.onChange = [this] { spec.sampleRate = cmbSampleRate.getSelectedId(); };
    cmbSampleRate.setSelectedId (config->getIntAttribute ("SampleRate", 44100));
    addAndMakeVisible (cmbSampleRate);

    lblCycles.setText ("Test cycles", dontSendNotification);
    lblCycles.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblCycles);
    cmbCycles.setTooltip ("Number of full test cycles to run (reset, prepare, processing)");
    cmbCycles.addItem ("10", 10);
    cmbCycles.addItem ("100", 100);
    cmbCycles.onChange = [this] { benchmarkThread.setTestCycles (cmbCycles.getSelectedId()); };
    cmbCycles.setSelectedId (config->getIntAttribute ("TestCycles", 10));
    addAndMakeVisible (cmbCycles);

    lblIterations.setText ("Process iterations", dontSendNotification);
    lblIterations.setJustificationType (Justification::centredRight);
    addAndMakeVisible (lblIterations);
    cmbIterations.setTooltip ("Number of times to iterate the processing within each cycle");
    cmbIterations.addItem ("10", 10);
    cmbIterations.addItem ("100", 100);
    cmbIterations.addItem ("1000", 1000);
    cmbIterations.addItem ("10000", 10000);
    cmbIterations.onChange = [this] { benchmarkThread.setProcessingIterations (cmbIterations.getSelectedId()); };
    cmbIterations.setSelectedId (config->getIntAttribute ("ProcessIterations", 1000));
    addAndMakeVisible (cmbIterations);

    btnStart.setButtonText ("Start tests");
    btnStart.setColour (TextButton::buttonColourId, Colours::green);
    btnStart.onClick = [this]
    {
        // Start running benchmarks on a different thread
        benchmarkThread.setProcessSpec (spec);
        benchmarkThread.runThread();
    };
    addAndMakeVisible (btnStart);

    btnReset.setButtonText ("Reset stats");
    btnReset.onClick = [this]
    {
        for (auto h : harnesses)
            if (h) h->resetStatistics();
    };
    addAndMakeVisible (btnReset);

    setSize (690, 430);
    startTimerHz (5);
}
BenchmarkComponent::~BenchmarkComponent()
{
    auto* deviceMgr = DSPTestbenchApplication::getApp().getMainWindow().getAudioDeviceManager();
    deviceMgr->restartLastAudioDevice();

    // Update configuration from class state
    config->setAttribute ("BlockSize", cmbBlockSize.getSelectedId());
    config->setAttribute ("NumChannels", cmbChannels.getSelectedId());
    config->setAttribute ("SampleRate", cmbSampleRate.getSelectedId());
    config->setAttribute ("TestCycles", cmbCycles.getSelectedId());
    config->setAttribute ("ProcessIterations", cmbIterations.getSelectedId());
    
    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue(keyName, config.get());
    propertiesFile->saveIfNeeded();
}
void BenchmarkComponent::paint (Graphics & g)
{
    g.fillAll (DspTestBenchLnF::ApplicationColours::componentBackground());
}
void BenchmarkComponent::resized()
{
    using Track = Grid::TrackInfo;

    const auto titleRowHeight = Grid::Px (titleFont.getHeight() * 1.15f);
    const auto valueRowHeight = Grid::Px (normalFont.getHeight() * 1.25f);
    const auto titleColumnWidth = GUI_SIZE_PX (5.0);
    const auto valueColumnWidth = GUI_SIZE_PX (3.7);
    const auto gap = GUI_BASE_GAP_PX;

    Grid resultsGrid;
    resultsGrid.rowGap = gap;
    resultsGrid.columnGap = gap;
    resultsGrid.templateRows = {
        Track (1_fr),               // row  1 is for centering
        Track (titleRowHeight),     // row  2 is for processor A title and value column titles
        Track (valueRowHeight),     // row  3 is for processor A prepare routine results
        Track (valueRowHeight),     // row  4 is for processor A processing routine results
        Track (valueRowHeight),     // row  5 is for processor A reset routine results
        Track (GUI_GAP_PX (2)),     // row  6 is the blank row between processors
        Track (titleRowHeight),     // row  7 is for processor B title and value column titles
        Track (valueRowHeight),     // row  8 is for processor B prepare routine results
        Track (valueRowHeight),     // row  9 is for processor B processing routine results
        Track (valueRowHeight),     // row 10 is for processor B reset routine results
        Track (1_fr)                // row 11 is for centering
    };
    resultsGrid.templateColumns = {
        Track (1_fr),               // column 1 is for centering 
        Track (titleColumnWidth),   // column 2 is for titles
        Track (valueColumnWidth),   // column 3 is for min values
        Track (valueColumnWidth),   // column 4 is for avg values
        Track (valueColumnWidth),   // column 5 is for max values
        Track (valueColumnWidth),   // column 6 is for sample counts
        Track (1_fr)                // column 7 is for centering
    };
    resultsGrid.items.addArray({

        GridItem().withArea (1, 1),

        GridItem (processorLabels[0]).withArea (2, 2),
        GridItem (valueTitleLabels[0]).withArea (2, 3),
        GridItem (valueTitleLabels[1]).withArea (2, 4),
        GridItem (valueTitleLabels[2]).withArea (2, 5),
        GridItem (valueTitleLabels[3]).withArea (2, 6),
        GridItem (routineLabels[0]).withArea (3, 2),
        GridItem (routineLabels[1]).withArea (4, 2),
        GridItem (routineLabels[2]).withArea (5, 2),

        GridItem().withArea (6, 1),

        GridItem (processorLabels[1]).withArea (7, 2),
        GridItem (valueTitleLabels[4]).withArea (7, 3),
        GridItem (valueTitleLabels[5]).withArea (7, 4),
        GridItem (valueTitleLabels[6]).withArea (7, 5),
        GridItem (valueTitleLabels[7]).withArea (7, 6),
        GridItem (routineLabels[3]).withArea (8, 2),
        GridItem (routineLabels[4]).withArea ( 9, 2),
        GridItem (routineLabels[5]).withArea (10, 2),
        
        GridItem().withArea (11, 7)
    });

    const auto numRoutines = static_cast<int> (routines.size());
    const auto numValues = static_cast<int> (values.size());

    for (auto p = 0; p < static_cast<int> (processors.size()); ++p)
    {
        const auto offsetP = p * (numRoutines + 2) + 3;
        for (auto r = 0; r < numRoutines; ++r)
        {
            for (auto v = 0; v < numValues; ++v)
            {
                const auto idxLabel = getValueLabelIndex (p, r, v);
                const auto row = offsetP + r;
                const auto col = v + 3;
                resultsGrid.items.add (GridItem (valueLabels[idxLabel]).withArea (row, col));
            }
        }
    }

    
    const auto controlColumnWidth = GUI_SIZE_PX (4.2);
    const auto controlRowHeight = GUI_SIZE_PX (0.8);

    Grid controlsGrid;
    controlsGrid.rowGap = gap;
    controlsGrid.columnGap = gap;
    controlsGrid.templateRows = {
        Track (controlRowHeight),
        Track (controlRowHeight),
        Track (controlRowHeight),
        Track (controlRowHeight)
    };
    controlsGrid.templateColumns = {
        Track (1_fr),               // for centering 
        Track (controlColumnWidth),
        Track (controlColumnWidth),
        Track (GUI_GAP_PX (2)),     // for spacing
        Track (controlColumnWidth),
        Track (controlColumnWidth),
        Track (1_fr)                // centering
    };
    controlsGrid.items.addArray({
        GridItem().withArea (1, 1, 6, 1),
        GridItem().withArea (1, 7, 6, 7),
        GridItem (lblBlockSize),    GridItem (cmbBlockSize),    GridItem(),     GridItem (lblCycles),       GridItem (cmbCycles),
        GridItem (lblChannels),     GridItem (cmbChannels),     GridItem(),     GridItem (lblIterations),   GridItem (cmbIterations),
        GridItem (lblSampleRate),   GridItem (cmbSampleRate),   GridItem(),     GridItem(),                 GridItem(),
        GridItem(),                 GridItem (),                GridItem(),     GridItem (btnStart),        GridItem (btnReset)
    });

    resultsGrid.performLayout (getLocalBounds().withHeight (290));
    controlsGrid.performLayout (getLocalBounds().withTrimmedTop (290));
}
void BenchmarkComponent::timerCallback()
{
    for (auto p = 0; p < static_cast<int> (processors.size()); ++p)
    {
        if (auto* harness = harnesses[p])
        {
            for (auto r = 0; r < static_cast<int> (routines.size()); ++r)
            {
                for (auto v = 0; v < static_cast<int> (values.size()); ++v)
                {
                    const auto idxLabel = getValueLabelIndex (p, r, v);
                    const auto queryValue = harness->queryByIndex (r, v);
                    auto txt = String ("-");
                    switch (v)
                    {
                        case 0: if (queryValue < 1.0E100)
                                    txt = String (queryValue * 1000.0, 1);
                                break;
                        case 1: if (isfinite (queryValue))
                                    txt = String (queryValue * 1000.0, 1);
                                break;
                        case 2: if (queryValue > 0.0)
                                    txt = String (queryValue * 1000.0, 1);
                                break;
                        case 3: if (queryValue > 0.0)
                                    txt = String (static_cast<int> (queryValue));
                                break;
                        default: break;
                    }
                    valueLabels[idxLabel]->setText (txt, sendNotificationAsync);
                }
            }
        }
    }
}
int BenchmarkComponent::getValueLabelIndex (const int processorIndex, const int routineIndex, const int valueIndex) const
{
    const auto offset = (processorIndex == 0) ? 0 : static_cast<int> (routines.size() * values.size());
    return ProcessorHarness::getQueryIndex (routineIndex, valueIndex) + offset;
}

BenchmarkComponent::BenchmarkThread::BenchmarkThread (std::vector<ProcessorHarness*>* harnesses, SourceComponent* sourceComponent)
    : ThreadWithProgressWindow ("Benchmark is running", true, true),
      srcComponent (sourceComponent)
{
    // Note that we use the default timeout of 10 seconds for exiting the thread. This is needed as the benchmark can cause a NaN on one of the audio channels if
    // processing on the ordinary thread is resumed while this one is still running. This is quite unlikely if given a second to exit, so 10 seconds should be pretty safe.

    processingHarnesses = harnesses;
}
void BenchmarkComponent::BenchmarkThread::run()
{
    jassert (testCycles > 0 && processingIterations > 0);
    jassert (testSpec.numChannels > 0 && testSpec.maximumBlockSize > 0 && testSpec.sampleRate > 0);

    const dsp::ProcessContextReplacing<float> context (*audioBlock.get());
    
    // Only count non null harnesses
    auto numHarnesses = 0;
    if ((*processingHarnesses)[0]) numHarnesses++;
    if ((*processingHarnesses)[1]) numHarnesses++;

    auto numerator = 0;
    const auto denominator = static_cast<double> (numHarnesses * testCycles * (2 + processingIterations));

    setProgress (0.0);
    for (auto c = 0; c < testCycles; ++c)
    {
        for (auto p : *processingHarnesses)
        {
            if (p)
            {
                p->resetHarness();
                numerator++;
                if (threadShouldExit()) return;
                
                p->prepareHarness (testSpec);
                numerator++;
                if (threadShouldExit()) return;
                
                for (auto i = 0; i < processingIterations; ++i)
                {
                    p->processHarness (context);
                    numerator++;
                    if (threadShouldExit()) return;
                    setProgress (static_cast<double> (numerator) / denominator);
                    yield();
                }
            }
        }
    }
}
void BenchmarkComponent::BenchmarkThread::threadComplete (bool /* userPressedCancel */)
{
    // Nothing to do here!
}
void BenchmarkComponent::BenchmarkThread::setTestCycles (const int cycles)
{
    testCycles = cycles;
}
void BenchmarkComponent::BenchmarkThread::setProcessingIterations (const int iterations)
{
    processingIterations = iterations;
}
void BenchmarkComponent::BenchmarkThread::setProcessSpec (dsp::ProcessSpec & spec)
{
    jassert (spec.numChannels > 0 && spec.maximumBlockSize > 0 && spec.sampleRate > 0);
    testSpec = spec;

    // Initialise audio block
    audioBlock.reset (new dsp::AudioBlock<float> (heapBlock, testSpec.numChannels, testSpec.maximumBlockSize));

    // Fill block with audio data from source component
    jassert (srcComponent);
    srcComponent->prepare (spec);
    const dsp::ProcessContextReplacing<float> context (*audioBlock.get());
    srcComponent->process (context);
}
