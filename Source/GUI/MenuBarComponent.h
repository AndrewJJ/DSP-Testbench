/*
  ==============================================================================

    MenuBarComponent.h
    Created: 24 Apr 2019 6:41:55pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"

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
        CpuMeter();
        ~CpuMeter() override = default;
        void paint (Graphics& g) override;
        void timerCallback() override;

    private:
        double cpuEnvelope = 0.0;
        int updateFrequency = 25;
        double releaseTime = 0.65 * static_cast<double> (updateFrequency);
        double releaseConstant = 1.0f - exp (-1.0f / releaseTime);
    };

    class XRunMeter final : public Component, public Timer, public SettableTooltipClient
    {
    public:
        XRunMeter();
        ~XRunMeter() override = default;
        void paint (Graphics& g) override;
        void timerCallback() override;
        void mouseDown (const MouseEvent& event) override;

    private:
        int bufferXruns = 0;
        int bufferXrunOffset = 0;
        int updateFrequency = 25;
    };

private:
    MainContentComponent* mainContentComponent;
    Label lblTitle;
    std::unique_ptr<Button> btnClose{};
    std::unique_ptr<Button> btnMinimise{};
    std::unique_ptr<Button> btnMaximise{};
    std::unique_ptr<DrawableButton> btnAudioDevice{};
    std::unique_ptr<DrawableButton> btnSnapshot{};
    std::unique_ptr<DrawableButton> btnBenchmark{};
    std::unique_ptr<DrawableButton> btnAbout{};
    std::unique_ptr<ComponentBoundsConstrainer> aboutConstrainer{};
    CpuMeter cpuMeter;
    XRunMeter xRunMeter;

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