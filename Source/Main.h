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

//Component* createMainContentComponent();
class DspTestBenchLnF : public LookAndFeel_V4
{
public:

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
    int getDefaultMenuBarHeight() override;
    Font getTitleFont() const;
private:
    const double titleMenuScalingFactor = 0.75;
};

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
        DspTestBenchMenuComponent (MainContentComponent* mainContentComponent);
        void paint (Graphics& g) override;
        void resized() override;

    private:
        MainContentComponent* mainContentComponent;
        ScopedPointer<Label> lblTitle;
        ScopedPointer<Button> btnClose;
        ScopedPointer<Button> btnMinimise;
        ScopedPointer<Button> btnMaximise;
        ScopedPointer<TextButton> btnAudioDevice;
        ScopedPointer<TextButton> btnOneShot;
        ScopedPointer<TextButton> btnResume;
        ScopedPointer<AudioDeviceSelectorComponent> deviceSelector;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspTestBenchMenuComponent)
    };

    // Dummy menu bar model that we use to trick the menu system into setting the menu height
    class DummyMenuBarModel : public MenuBarModel
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
        AudioDeviceManager deviceManager;
        ScopedPointer<DummyMenuBarModel> dummyMenuBarModel;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    static DSPTestbenchApplication& getApp();
    MainWindow& getMainWindow();
    Component& getMainComponent();
    ApplicationProperties appProperties;

private:
    DspTestBenchLnF dspTestBenchLnF;
    ScopedPointer<MainWindow> mainWindow;
    TooltipWindow tooltipWindow;
};