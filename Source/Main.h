/*
  ==============================================================================

    Main.h
    Created: 12 Jan 2018 10:31:34am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once


#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"
#include "LookAndFeel.h"

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

    // Custom menu bar component for this application
    class DspTestBenchMenuComponent : public Component
    {
    public:
        explicit DspTestBenchMenuComponent (MainContentComponent* mainContentComponent);
        void paint (Graphics& g) override;
        void resized() override;
        
        class CpuMeter final : public Component, public Timer, public SettableTooltipClient
        {
        public:
            CpuMeter ();
            ~CpuMeter() = default;
            void paint (Graphics& g) override;
            void resized() override;
            void timerCallback() override;
            void mouseDown (const MouseEvent& event) override;

        private:
            int bufferXrunCount = 0;
            double cpuEnvelope = 0.0;
            int updateFrequency = 25;
            double releaseTime = 0.65 * static_cast<double> (updateFrequency);
            double releaseConstant = 1.0f - exp (-1.0f / releaseTime);
        };

    private:
        MainContentComponent* mainContentComponent;
        Label lblTitle;
        std::unique_ptr<Button> btnClose;
        std::unique_ptr<Button> btnMinimise;
        std::unique_ptr<Button> btnMaximise;
        std::unique_ptr<DrawableButton> btnAudioDevice;
        std::unique_ptr<DrawableButton> btnSnapshot;
        std::unique_ptr<DrawableButton> btnAbout;
        std::unique_ptr<ComponentBoundsConstrainer> aboutConstrainer;
        CpuMeter cpuMeter;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspTestBenchMenuComponent)
    };

    // Dummy menu bar model that we use to trick the menu system into setting the menu height
    class DummyMenuBarModel final : public MenuBarModel
    {
    public:
        StringArray getMenuBarNames() override;
        PopupMenu getMenuForIndex(int topLevelMenuIndex, const String & menuName) override;
        void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
    };

    /*
        This class implements the desktop window that contains an instance of
        our MainContentComponent class.
    */
    class MainWindow final : public DocumentWindow
    {
    public:

        MainWindow (String name);
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
        TooltipWindow tooltipWindow;
        AudioDeviceManager deviceManager;
        std::unique_ptr<DummyMenuBarModel> dummyMenuBarModel{};
        std::unique_ptr<XmlElement> config{};
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    static DSPTestbenchApplication& getApp();
    MainWindow& getMainWindow();
    Component& getMainComponent();
    ApplicationProperties appProperties;

private:
    DspTestBenchLnF dspTestBenchLnF{};
    std::unique_ptr<MainWindow> mainWindow{};
};
