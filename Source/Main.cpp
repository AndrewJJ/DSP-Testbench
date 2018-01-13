/*
  ==============================================================================

    Main.cpp
    Created: 10 Jan 2018 10:31:34am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "Main.h"

DSPTestbenchApplication::DSPTestbenchApplication ()
    : TimeSliceThread ("Audio File Reader Thread")
{
}

void DSPTestbenchApplication::initialise (const String& commandLine)
{
    formatManager.registerBasicFormats();
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
void DSPTestbenchApplication::anotherInstanceStarted (const String& commandLine)
{
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
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
Component& DSPTestbenchApplication::getMainComponent ()
{
    auto* comp = getMainWindow().getContentComponent();
    jassert (comp != nullptr);
    return *comp;
}
AudioDeviceManager& DSPTestbenchApplication::getDeviceManager()
{
    return reinterpret_cast<MainContentComponent*> (mainWindow->getContentComponent())->deviceManager;
}
AudioFormatManager& DSPTestbenchApplication::getFormatManager()
{
    return formatManager;
}

DSPTestbenchApplication::MainWindow::MainWindow (String name)
    : DocumentWindow (name,
                      Desktop::getInstance().getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (false);
    //setContentOwned (createMainContentComponent(), true);
    setContentOwned (new MainContentComponent(), true);
    setResizable (true, true);

    centreWithSize (getWidth(), getHeight());
    Component::setVisible (true);    
}
void DSPTestbenchApplication::MainWindow::closeButtonPressed()
{
    // This is called when the user tries to close this window. Here, we'll just
    // ask the app to quit when this happens, but you can change this to do
    // whatever you need.
    JUCEApplication::getInstance()->systemRequestedQuit();
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (DSPTestbenchApplication)
