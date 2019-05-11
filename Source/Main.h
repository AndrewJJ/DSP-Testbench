/*
  ==============================================================================

    Main.h
    Created: 12 Jan 2018 10:31:34am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once


#include "../JuceLibraryCode/JuceHeader.h"
#include "GUI/LookAndFeel.h"
#include "GUI/MenuBarComponent.h"

class DSPTestbenchApplication final : public JUCEApplication, public TimeSliceThread
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

    static DSPTestbenchApplication& getApp();
    Component& getMainComponent();

    /*
        This class implements the desktop window that contains an instance of
        our MainContentComponent class.
    */
    class MainWindow final : public DocumentWindow
    {
    public:

        explicit MainWindow (const String& name);
        ~MainWindow();
        void closeButtonPressed() override;

        AudioDeviceManager* getAudioDeviceManager();

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        AudioDeviceManager deviceManager;
        std::unique_ptr<DummyMenuBarModel> dummyMenuBarModel{};
        std::unique_ptr<XmlElement> config{};
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    MainWindow& getMainWindow();
    ApplicationProperties appProperties;

private:
    DspTestBenchLnF dspTestBenchLnF{};
    TooltipWindow tooltipWindow;
    std::unique_ptr<MainWindow> mainWindow{};
};
