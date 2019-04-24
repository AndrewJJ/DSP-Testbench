/*
  ==============================================================================

    Main.cpp
    Created: 10 Jan 2018 10:31:34am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "Main.h"
#include "MainComponent.h"
#include "AboutComponent.h"

DSPTestbenchApplication::DSPTestbenchApplication ()
    : TimeSliceThread ("Audio File Reader Thread")
{
    LookAndFeel::setDefaultLookAndFeel(&dspTestBenchLnF);
}
void DSPTestbenchApplication::initialise (const String&)
{
    PropertiesFile::Options options;
    options.applicationName = "DSP Testbench";
    options.filenameSuffix = ".settings";
    options.folderName = "DSP Testbench";
    options.osxLibrarySubFolder = "Application Support";
    appProperties.setStorageParameters(options);

    mainWindow.reset (new MainWindow (getApplicationName()));
    startThread();
}
void DSPTestbenchApplication::shutdown()
{
    stopThread (100);
    mainWindow = nullptr; // (deletes our window)
}
void DSPTestbenchApplication::systemRequestedQuit()
{
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app to close.
    quit();
}
void DSPTestbenchApplication::anotherInstanceStarted (const String&)
{
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
}

DSPTestbenchApplication::DspTestBenchMenuComponent::DspTestBenchMenuComponent (MainContentComponent* mainContentComponent_)
    : mainContentComponent (mainContentComponent_)
{
    addAndMakeVisible (lblTitle);
    lblTitle.setText (getApp().getApplicationName(), dontSendNotification);
    lblTitle.setColour (Label::ColourIds::textColourId, cols::titleFontColour());
    lblTitle.setFont (dynamic_cast<DspTestBenchLnF*> (&getLookAndFeel())->getTitleFont());

    // Allow dragging of the window via the underlying functionality from ResizableWindow
    this->setInterceptsMouseClicks (false, true);
    lblTitle.setInterceptsMouseClicks (false, false);

    btnClose.reset (getLookAndFeel().createDocumentWindowButton(4));
    addAndMakeVisible (btnClose.get());
    btnClose->onClick = [this]
    {
        getApp().systemRequestedQuit();
    };

    btnMinimise.reset (getLookAndFeel().createDocumentWindowButton(1));
    addAndMakeVisible (btnMinimise.get());
    btnMinimise->onClick = [this]
    {
        const auto shouldMinimise = !getApp().getMainWindow().isMinimised();
        getApp().getMainWindow().setMinimised (shouldMinimise);        
    };
    
    btnMaximise.reset (getLookAndFeel().createDocumentWindowButton(2));
    addAndMakeVisible (btnMaximise.get());
    btnMaximise->onClick = [this]
    {
        const auto shouldBeFullScreen = !getApp().getMainWindow().isFullScreen();
        getApp().getMainWindow().setFullScreen (shouldBeFullScreen);
    };

    addAndMakeVisible (cpuMeter);

    btnSnapshot.reset (new DrawableButton ("Snapshot", DrawableButton::ImageFitted));
    addAndMakeVisible (btnSnapshot.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnSnapshot.get(), BinaryData::screenshot_svg, BinaryData::screenshot_svgSize, Colours::black, Colours::red);
    btnSnapshot->setTooltip ("Restart audio briefly, then hold a snapshot of the result for analysis");
    btnSnapshot->setClickingTogglesState(true);
    btnSnapshot->onClick = [this]
    {
        if (btnSnapshot->getToggleState())
            mainContentComponent->triggerSnapshot();
        else
            mainContentComponent->resumeStreaming();
    };

    btnAudioDevice.reset (new DrawableButton ("Audio Settings", DrawableButton::ImageFitted));
    addAndMakeVisible (btnAudioDevice.get());
    DspTestBenchLnF::setImagesForDrawableButton (btnAudioDevice.get(), BinaryData::audio_settings_svg, BinaryData::audio_settings_svgSize, Colours::black);
    btnAudioDevice->setTooltip ("Configure audio device settings");
    btnAudioDevice->onClick = [this]
    {
        AudioDeviceManager* deviceMgr =  getApp().getMainWindow().getAudioDeviceManager();
        AudioDeviceSelectorComponent* deviceSelector = new AudioDeviceSelectorComponent (*deviceMgr, 1, 1024, 1, 1024, false, false, false, false);
        deviceSelector->setSize (500, 300);
        DialogWindow::LaunchOptions launchOptions;
        launchOptions.dialogTitle = "Audio device settings";
        //launchOptions.resizable = true;
        launchOptions.useNativeTitleBar = false;
        launchOptions.dialogBackgroundColour = Colour (0xff323e44);
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
        launchOptions.dialogTitle = "About " + getApp().getApplicationName() + " (v" + String (ProjectInfo::versionString) + ")";
        launchOptions.useNativeTitleBar = false;
        launchOptions.dialogBackgroundColour = Colour (0xff323e44);
        launchOptions.componentToCentreAround = mainContentComponent;
        launchOptions.content.set (new AboutComponent(), true);
        launchOptions.resizable = true;
        auto* dw = launchOptions.launchAsync();
        aboutConstrainer.reset(new ComponentBoundsConstrainer());
        aboutConstrainer->setMinimumSize (800, 600);
        dw->setConstrainer(aboutConstrainer.get());
    };
}
void DSPTestbenchApplication::DspTestBenchMenuComponent::paint(Graphics & g)
{
    g.fillAll (Colour(0xff263238));
}
void DSPTestbenchApplication::DspTestBenchMenuComponent::resized()
{
    using Track = Grid::TrackInfo;
    const auto margin = 2;
    const auto cpuMeterSize = GUI_SIZE_PX (3.0);
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
        GridItem (btnAudioDevice.get()),
        GridItem (btnAbout.get()),
        GridItem (), // windowButtonGap
        GridItem (btnMinimise.get()), GridItem (btnMaximise.get()), GridItem (btnClose.get())
    });

    grid.autoFlow = Grid::AutoFlow::column;
    grid.performLayout (getLocalBounds().reduced (margin, margin));
}

DSPTestbenchApplication::DspTestBenchMenuComponent::CpuMeter::CpuMeter()
{
    setOpaque (true);
    setPaintingIsUnclipped (true);
    startTimerHz (updateFrequency);
}
void DSPTestbenchApplication::DspTestBenchMenuComponent::CpuMeter::paint (Graphics & g)
{
    g.setColour (Colour (0xff263238));
    g.fillRect (getLocalBounds());

    const auto canvasRect = getLocalBounds().reduced (0, 4);
    const auto lblWidth = GUI_SIZE_I (1.1);
    const auto lblRect = canvasRect.withWidth (lblWidth);
    const auto meterRect = canvasRect.withTrimmedLeft (lblWidth);

    g.setColour (Colours::black);
    g.fillRect (meterRect);

    const auto w = static_cast<int> (static_cast<double>(meterRect.getWidth()) * cpuEnvelope);
    g.setColour (Colour (0xff705090));
    g.fillRect (meterRect.withWidth (w));

    g.setFont (GUI_SIZE_F (0.5));
    g.setColour (Colours::white);
    g.drawText ("CPU", lblRect, Justification::centredLeft, false);
    g.drawText ( String (static_cast<int> (cpuEnvelope * 100.0)) + "%", meterRect, Justification::centred, false);
}
void DSPTestbenchApplication::DspTestBenchMenuComponent::CpuMeter::resized()
{}
void DSPTestbenchApplication::DspTestBenchMenuComponent::CpuMeter::timerCallback()
{
    AudioDeviceManager* deviceMgr =  getApp().getMainWindow().getAudioDeviceManager();
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
void DSPTestbenchApplication::DspTestBenchMenuComponent::CpuMeter::mouseDown (const MouseEvent& /* event */)
{
    AudioDeviceManager* deviceMgr =  getApp().getMainWindow().getAudioDeviceManager();
    bufferXrunOffset = deviceMgr->getXRunCount();;
}

StringArray DSPTestbenchApplication::DummyMenuBarModel::getMenuBarNames()
{
    return StringArray();
}
PopupMenu DSPTestbenchApplication::DummyMenuBarModel::getMenuForIndex(int /*topLevelMenuIndex*/, const String& /*menuName*/)
{
    return PopupMenu();
}
void DSPTestbenchApplication::DummyMenuBarModel::menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/)
{
}

DSPTestbenchApplication& DSPTestbenchApplication::getApp()
{
    auto* app = dynamic_cast<DSPTestbenchApplication*> (JUCEApplication::getInstance());
    jassert (app != nullptr);
    return *app;
}
DSPTestbenchApplication::MainWindow& DSPTestbenchApplication::getMainWindow()
{
    jassert(mainWindow != nullptr);
    return *mainWindow;
}
Component& DSPTestbenchApplication::getMainComponent()
{
    auto* comp = getMainWindow().getContentComponent();
    jassert (comp != nullptr);
    return *comp;
}

DSPTestbenchApplication::MainWindow::MainWindow (String name)
    : DocumentWindow (name,
                      Desktop::getInstance().getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (false);
    setTitleBarHeight (0);
    MainContentComponent* mainContentComponent;
    setContentOwned (mainContentComponent = new MainContentComponent(deviceManager), true);

    const auto minWidth = 992;
    const auto minHeight = 768;
    setResizable (true, false);
    setResizeLimits (minWidth, minHeight, 10000, 10000);

    // Need to dummy this up so we can change the menu component height
    dummyMenuBarModel.reset (new DummyMenuBarModel);
    setMenuBar (dummyMenuBarModel.get());
    setMenuBarComponent (new DspTestBenchMenuComponent (mainContentComponent));

    // Read application properties from settings file
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config.reset (propertiesFile->getXmlValue ("Application"));
    if (!config)
        config.reset(new XmlElement ("Application"));
    const auto width = config->getIntAttribute ("WindowWidth", minWidth);
    const auto height = config->getIntAttribute ("WindowHeight", minHeight);

    // Set window size
    centreWithSize (width, height);
    Component::setVisible (true);    
}
DSPTestbenchApplication::MainWindow::~MainWindow()
{
    // Update configuration from class state
    config->setAttribute ("WindowWidth", getWidth());
    config->setAttribute ("WindowHeight", getHeight());
    
    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue ("Application", config.get());
    propertiesFile->saveIfNeeded();

    // This ensures that we shutdown audio before deviceManager is removed, thus preventing an access violation
    dynamic_cast<AudioAppComponent*>(getContentComponent())->shutdownAudio();
}
void DSPTestbenchApplication::MainWindow::closeButtonPressed()
{
    // This is called when the user tries to close this window. Here, we'll just
    // ask the app to quit when this happens, but you can change this to do
    // whatever you need.
    JUCEApplication::getInstance()->systemRequestedQuit();
}
AudioDeviceManager* DSPTestbenchApplication::MainWindow::getAudioDeviceManager ()
{
    return &deviceManager;
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (DSPTestbenchApplication)