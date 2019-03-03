/*
  ==============================================================================

    Main.cpp
    Created: 10 Jan 2018 10:31:34am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "Main.h"
#include "MainComponent.h"

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

int DspTestBenchLnF::getDefaultMenuBarHeight()
{
    return GUI_SIZE_I (titleMenuScalingFactor) + GUI_GAP_I (2.0 * titleMenuScalingFactor);
}

Font DspTestBenchLnF::getTitleFont() const
{
    return Font (GUI_SIZE_F (titleMenuScalingFactor));
}

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

    mainWindow = new MainWindow (getApplicationName());
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
    addAndMakeVisible (lblTitle = new Label());
    lblTitle->setText(getApp().getApplicationName(), dontSendNotification);
    lblTitle->setColour(Label::ColourIds::textColourId, Colours::white);
    lblTitle->setFont(dynamic_cast<DspTestBenchLnF*> (&getLookAndFeel())->getTitleFont());
    
    addAndMakeVisible (btnClose = getLookAndFeel().createDocumentWindowButton(4));
    btnClose->onClick = [this]
    {
        getApp().systemRequestedQuit();
    };
    
    addAndMakeVisible (btnMinimise = getLookAndFeel().createDocumentWindowButton(1));
    btnMinimise->onClick = [this]
    {
        const auto shouldMinimise = !getApp().getMainWindow().isMinimised();
        getApp().getMainWindow().setMinimised (shouldMinimise);        
    };
    
    addAndMakeVisible (btnMaximise = getLookAndFeel().createDocumentWindowButton(2));
    btnMaximise->onClick = [this]
    {
        const auto shouldBeFullScreen = !getApp().getMainWindow().isFullScreen();
        getApp().getMainWindow().setFullScreen (shouldBeFullScreen);
    };

    // TODO: picture button for hold
    addAndMakeVisible (btnHold = new TextButton("Hold"));
    btnHold->setTooltip("Send audio test signals for a short while, then hold the result for analysis");
    btnHold->setColour (TextButton::buttonOnColourId, Colours::darkred);
    btnHold->setClickingTogglesState(true);
    btnHold->onClick = [this]
    {
        if (btnHold->getToggleState())
            mainContentComponent->triggerHoldMode();
    };

    // TODO: picture button for resume
    addAndMakeVisible (btnResume = new TextButton("Resume"));
    btnResume->setTooltip("Resume normal audio streaming");
    btnResume->onClick = [this]
    {
        mainContentComponent->resumeStreaming();
        btnHold->setToggleState (false, sendNotificationSync);
    };

    // TODO: picture button for audio settings
    addAndMakeVisible (btnAudioDevice = new TextButton("Audio Settings"));
    btnAudioDevice->setTooltip("Configure audio device settings");
    btnAudioDevice->onClick = [this]
    {
        AudioDeviceManager* deviceMgr =  getApp().getMainWindow().getAudioDeviceManager();
        deviceSelector = new AudioDeviceSelectorComponent (*deviceMgr, 1, 1024, 1, 1024, false, false, false, false);
        deviceSelector->setSize (500,300);
        DialogWindow::showDialog ("Audio device settings", deviceSelector, nullptr, Colours::darkgrey, true);
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
    const auto holdButtonSize = GUI_SIZE_PX (1.9);
    const auto resumeButtonSize = GUI_SIZE_PX (2.4);
    const auto audioDeviceBtnSize = GUI_SIZE_PX (3.6);
    const auto windowButtonSize = GUI_BASE_SIZE_PX;
    const auto gap = Track(GUI_BASE_GAP_PX);
    const auto windowButtonGap = Track(GUI_GAP_PX (5));

    Grid grid;

    grid.templateRows = { Track (1_fr) };
    grid.templateColumns = { Track (GUI_SIZE_PX (6)), Track (1_fr), Track (holdButtonSize), gap, Track (resumeButtonSize), gap, Track (audioDeviceBtnSize), windowButtonGap, Track (windowButtonSize), Track (windowButtonSize), Track (windowButtonSize) };

    grid.items.addArray({   
                            GridItem (lblTitle),
                            GridItem (), // expander
                            GridItem (btnHold),
                            GridItem (), // gap
                            GridItem (btnResume),
                            GridItem (), // gap
                            GridItem (btnAudioDevice),
                            GridItem (), // windowButtonGap
                            GridItem (btnMinimise), GridItem (btnMaximise), GridItem (btnClose)
                        });

    grid.autoFlow = Grid::AutoFlow::column;
    grid.performLayout (getLocalBounds().reduced (margin, margin));
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
    setResizable (true, false);
    setResizeLimits(992, 768, 10000, 10000);
    // Need to dummy this up so we can change the menu component height
    setMenuBar(dummyMenuBarModel = new DummyMenuBarModel);
    setMenuBarComponent (new DspTestBenchMenuComponent (mainContentComponent));
    
    centreWithSize (getWidth(), getHeight());
    Component::setVisible (true);    
}
DSPTestbenchApplication::MainWindow::~MainWindow()
{
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