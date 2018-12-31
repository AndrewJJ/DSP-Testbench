/*
  ==============================================================================

    Main.h
    Created: 12 Jan 2018 10:31:34am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

//Component* createMainContentComponent();

class DSPTestbenchApplication  : public JUCEApplication, public TimeSliceThread
{
public:

    DSPTestbenchApplication();

    const String getApplicationName() override      { return ProjectInfo::projectName; }
    const String getApplicationVersion() override   { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override      { return true; }

    void initialise (const String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted (const String& commandLine) override;

    /*
        This class implements the desktop window that contains an instance of
        our MainContentComponent class.
    */
    class MainWindow    : public DocumentWindow
    {
    public:

        MainWindow (String name);
        ~MainWindow();
        void closeButtonPressed() override;

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

        private:
        AudioDeviceManager deviceManager;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    static DSPTestbenchApplication& getApp();
    MainWindow& getMainWindow();
    Component& getMainComponent();

private:
    ScopedPointer<MainWindow> mainWindow;
    TooltipWindow tooltipWindow;
};