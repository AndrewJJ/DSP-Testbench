/*
  ==============================================================================

    MenuBarComponent.cpp
    Created: 24 Apr 2019 6:41:55pm
    Author:  Andrew

  ==============================================================================
*/

#include "MenuBarComponent.h"
#include "../Main.h"
#include "LookAndFeel.h"
#include "AboutComponent.h"
#include "BenchmarkComponent.h"

DspTestBenchMenuComponent::DspTestBenchMenuComponent (MainContentComponent* mainContentComponent_)
    : mainContentComponent (mainContentComponent_)
{
    using cols = DspTestBenchLnF::ApplicationColours;

    addAndMakeVisible (lblTitle);
    lblTitle.setText (DSPTestbenchApplication::getApp().getApplicationName(), dontSendNotification);
    lblTitle.setColour (Label::ColourIds::textColourId, cols::titleFontColour());
    lblTitle.setFont (dynamic_cast<DspTestBenchLnF*> (&getLookAndFeel())->getTitleFont());

    // Allow dragging of the window via the underlying functionality from ResizableWindow
    this->setInterceptsMouseClicks (false, true);
    lblTitle.setInterceptsMouseClicks (false, false);

    btnClose.reset (getLookAndFeel().createDocumentWindowButton(4));
    addAndMakeVisible (btnClose.get());
    btnClose->onClick = [this]
    {
        DSPTestbenchApplication::getApp().systemRequestedQuit();
    };

    btnMinimise.reset (getLookAndFeel().createDocumentWindowButton(1));
    addAndMakeVisible (btnMinimise.get());
    btnMinimise->onClick = [this]
    {
        const auto shouldMinimise = !DSPTestbenchApplication::getApp().getMainWindow().isMinimised();
        DSPTestbenchApplication::getApp().getMainWindow().setMinimised (shouldMinimise);        
    };
    
    btnMaximise.reset (getLookAndFeel().createDocumentWindowButton(2));
    addAndMakeVisible (btnMaximise.get());
    btnMaximise->onClick = [this]
    {
        const auto shouldBeFullScreen = !DSPTestbenchApplication::getApp().getMainWindow().isFullScreen();
        DSPTestbenchApplication::getApp().getMainWindow().setFullScreen (shouldBeFullScreen);
    };

    addAndMakeVisible (cpuMeter);

    btnSnapshot.reset (new DrawableButton ("Snapshot", DrawableButton::ImageFitted));
    addAndMakeVisible (btnSnapshot.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnSnapshot.get(), BinaryData::screenshot_svg, BinaryData::screenshot_svgSize, Colours::black, Colours::red);
    btnSnapshot->setTooltip ("Restart audio briefly, then hold a snapshot of the result for analysis");
    btnSnapshot->setClickingTogglesState (true);
    btnSnapshot->onClick = [this]
    {
        if (btnSnapshot->getToggleState())
            mainContentComponent->triggerSnapshot();
        else
            mainContentComponent->resumeStreaming();
    };

    btnBenchmark.reset (new DrawableButton ("Benchmark", DrawableButton::ImageFitted));
    addAndMakeVisible (btnBenchmark.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnBenchmark.get(), BinaryData::dashboard_gauge_svg, BinaryData::dashboard_gauge_svgSize, Colours::black);
    btnBenchmark->setTooltip ("Run performance benchmarks");
    btnBenchmark->onClick = [this]
    {
        AudioDeviceManager* deviceMgr = DSPTestbenchApplication::getApp().getMainWindow().getAudioDeviceManager();
        const auto blockSize = deviceMgr->getCurrentAudioDevice()->getCurrentBufferSizeSamples();
        // TODO - close audio device (and restart on exit)
        DialogWindow::LaunchOptions launchOptions;
        // TODO - change block size in title bar if testing independent of audio device
        launchOptions.dialogTitle = "Performance benchmarks (block size = " + String (blockSize) + ")";
        launchOptions.useNativeTitleBar = false;
        launchOptions.dialogBackgroundColour = cols::componentBackground();
        launchOptions.componentToCentreAround = mainContentComponent;
        launchOptions.content.set (new BenchmarkComponent (mainContentComponent->getProcessorHarness (0), mainContentComponent->getProcessorHarness (1)), true);
        launchOptions.resizable = false;
        launchOptions.launchAsync();
    };

    btnAudioDevice.reset (new DrawableButton ("Audio Settings", DrawableButton::ImageFitted));
    addAndMakeVisible (btnAudioDevice.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnAudioDevice.get(), BinaryData::audio_settings_svg, BinaryData::audio_settings_svgSize, Colours::black);
    btnAudioDevice->setTooltip ("Configure audio device settings");
    btnAudioDevice->onClick = [this]
    {
        AudioDeviceManager* deviceMgr = DSPTestbenchApplication::getApp().getMainWindow().getAudioDeviceManager();
        AudioDeviceSelectorComponent* deviceSelector = new AudioDeviceSelectorComponent (*deviceMgr, 1, 1024, 1, 1024, false, false, false, false);
        deviceSelector->setSize (500, 300);
        DialogWindow::LaunchOptions launchOptions;
        launchOptions.dialogTitle = "Audio device settings";
        //launchOptions.resizable = true;
        launchOptions.useNativeTitleBar = false;
        launchOptions.dialogBackgroundColour = cols::componentBackground();
        launchOptions.componentToCentreAround = mainContentComponent;
        launchOptions.content.set (deviceSelector, true);
        launchOptions.launchAsync();
    };

    btnAbout.reset (new DrawableButton ("About", DrawableButton::ImageFitted));
    addAndMakeVisible (btnAbout.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnAbout.get(), BinaryData::about_svg, BinaryData::about_svgSize, Colours::black);
    btnAbout->setTooltip ("Show information about DSP Testbench (including attributions)");
    btnAbout->onClick = [this]
    {
        DialogWindow::LaunchOptions launchOptions;
        launchOptions.dialogTitle = "About " + DSPTestbenchApplication::getApp().getApplicationName() + " (v" + String (ProjectInfo::versionString) + ")";
        launchOptions.useNativeTitleBar = false;
        launchOptions.dialogBackgroundColour = cols::componentBackground();
        launchOptions.componentToCentreAround = mainContentComponent;
        launchOptions.content.set (new AboutComponent(), true);
        launchOptions.resizable = true;
        auto* dw = launchOptions.launchAsync();
        aboutConstrainer.reset(new ComponentBoundsConstrainer());
        aboutConstrainer->setMinimumSize (800, 600);
        dw->setConstrainer(aboutConstrainer.get());
    };
}
void DspTestBenchMenuComponent::paint(Graphics & g)
{
    g.fillAll (DspTestBenchLnF::ApplicationColours::titleMenuBackground());
}
void DspTestBenchMenuComponent::resized()
{
    using Track = Grid::TrackInfo;
    const auto margin = 2;
    const auto cpuMeterSize = GUI_SIZE_PX (3.0);
    const auto benchmarkButtonSize = GUI_SIZE_PX (1);
    const auto snapshotButtonSize = GUI_SIZE_PX (1);
    const auto audioDeviceBtnSize = GUI_SIZE_PX (1.3);
    const auto aboutBtnSize = GUI_SIZE_PX (1.1);
    const auto windowButtonSize = GUI_BASE_SIZE_PX;
    const auto separatingGap = Track(GUI_GAP_PX (4));
    const auto windowButtonGap = Track(GUI_GAP_PX (4));

    Grid grid;

    grid.templateRows = { Track (1_fr) };
    grid.templateColumns = { 
        Track (GUI_SIZE_PX (6)),
        Track (1_fr),
        Track (cpuMeterSize),
        separatingGap,
        Track (snapshotButtonSize),
        Track (benchmarkButtonSize),
        Track (audioDeviceBtnSize),
        Track (aboutBtnSize),
        windowButtonGap,
        Track (windowButtonSize),
        Track (windowButtonSize),
        Track (windowButtonSize)
    };

    grid.items.addArray({   
        GridItem (lblTitle),
        GridItem (), // expander
        GridItem (cpuMeter),
        GridItem (), // separatingGap
        GridItem (btnSnapshot.get()),
        GridItem (btnBenchmark.get()),
        GridItem (btnAudioDevice.get()),
        GridItem (btnAbout.get()),
        GridItem (), // windowButtonGap
        GridItem (btnMinimise.get()), GridItem (btnMaximise.get()), GridItem (btnClose.get())
    });

    grid.autoFlow = Grid::AutoFlow::column;
    grid.performLayout (getLocalBounds().reduced (margin, margin));
}

DspTestBenchMenuComponent::CpuMeter::CpuMeter()
{
    setOpaque (true);
    setPaintingIsUnclipped (true);
    startTimerHz (updateFrequency);
}
void DspTestBenchMenuComponent::CpuMeter::paint (Graphics & g)
{
    using cols = DspTestBenchLnF::ApplicationColours;

    g.setColour (cols::titleMenuBackground());
    g.fillRect (getLocalBounds());

    const auto canvasRect = getLocalBounds().reduced (0, 4);
    const auto lblWidth = GUI_SIZE_I (1.1);
    const auto lblRect = canvasRect.withWidth (lblWidth);
    const auto meterRect = canvasRect.withTrimmedLeft (lblWidth);

    g.setColour (cols::meterBackground());
    g.fillRect (meterRect);

    const auto w = static_cast<int> (static_cast<double>(meterRect.getWidth()) * cpuEnvelope);
    g.setColour (cols::cpuMeterBarColour());
    g.fillRect (meterRect.withWidth (w));

    g.setFont (GUI_SIZE_F (0.5));
    g.setColour (cols::titleFontColour());
    g.drawText ("CPU", lblRect, Justification::centredLeft, false);
    g.drawText ( String (static_cast<int> (cpuEnvelope * 100.0)) + "%", meterRect, Justification::centred, false);
}
void DspTestBenchMenuComponent::CpuMeter::resized()
{}
void DspTestBenchMenuComponent::CpuMeter::timerCallback()
{
    AudioDeviceManager* deviceMgr = DSPTestbenchApplication::getApp().getMainWindow().getAudioDeviceManager();
    const auto currentCpu = deviceMgr->getCpuUsage();
    if (currentCpu > cpuEnvelope)
        cpuEnvelope = currentCpu; // Instant attack
    else
        cpuEnvelope += (releaseConstant * (currentCpu - cpuEnvelope));
    repaint();
    
    bufferXruns = deviceMgr->getXRunCount();
    if (bufferXruns < bufferXrunOffset)
    {
        // Device must have been reset, so we need to reset the offset to avoid displaying negative result
        bufferXrunOffset = 0;
    }

    setTooltip (String (bufferXruns - bufferXrunOffset) + " under/overruns");
}
void DspTestBenchMenuComponent::CpuMeter::mouseDown (const MouseEvent& /* event */)
{
    AudioDeviceManager* deviceMgr = DSPTestbenchApplication::getApp().getMainWindow().getAudioDeviceManager();
    bufferXrunOffset = deviceMgr->getXRunCount();;
}

StringArray DummyMenuBarModel::getMenuBarNames()
{
    return StringArray();
}
PopupMenu DummyMenuBarModel::getMenuForIndex(int /*topLevelMenuIndex*/, const String& /*menuName*/)
{
    return PopupMenu();
}
void DummyMenuBarModel::menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/)
{
}

