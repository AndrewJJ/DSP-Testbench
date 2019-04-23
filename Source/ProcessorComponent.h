/*
  ==============================================================================

    ProcessorComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ProcessorHarness.h"

class ProcessorComponent final : public Component, dsp::ProcessorBase
{
public:
    
    ProcessorComponent (const String& processorId, ProcessorHarness* processorToTest, const int numberOfControls);
    ~ProcessorComponent();

    void paint (Graphics& g) override;
    void resized() override;
    float getMinimumWidth() const;
    float getMinimumHeight() const;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    bool isSourceConnectedA() const noexcept;
    bool isSourceConnectedB() const noexcept;
    bool isProcessorEnabled() const noexcept;
    bool isInverted() const noexcept;
    bool isMuted() const noexcept;
    // Returns true if this processor is producing audio
    bool isActive() const noexcept;
    void mute();

    // TODO - provide methods to name controls (perhaps an array of names)

private:
    
    class ControlComponent : public Component
    {
    public:
        ControlComponent (const int index, ProcessorHarness* processorBeingControlled);
        void paint (Graphics& g) override;
        void resized() override;
        double getCurrentControlValue() const;

    private:
        int controlIndex;
        ProcessorHarness* processor;
        Label lblControl {};
        Slider sldControl {};
        Atomic<double> currentControlValue;
    };

    class ControlArrayComponent : public Component
    {
    public:
        explicit ControlArrayComponent (OwnedArray<ControlComponent>* controlComponentsToReferTo);

        void paint (Graphics& g) override;
        void resized() override;
        float getPreferredHeight() const;

        /** This must be called to add the controls and make them visible (once they are set up in the parent). */
        void initialiseControls();

    private:
        OwnedArray<ControlComponent>* controlComponents {};
    };

    String keyName;
    std::unique_ptr<XmlElement> config;

    Label lblTitle;
    TextButton btnSourceA;
    TextButton btnSourceB;
    TextButton btnDisable;
    TextButton btnInvert;
    TextButton btnMute;

    // TODO - add a bypass button to allow direct analysis of a source?
    // TODO - add variable delay so that signals can be time aligned?

    Atomic<bool> statusSourceA = true;
    Atomic<bool> statusSourceB = false;
    Atomic<bool> statusDisable = false;
    Atomic<bool> statusInvert = false;
    Atomic<bool> statusMute = false;

    Viewport viewport;
    OwnedArray<ControlComponent> controlArray {};
    ControlArrayComponent controlArrayComponent;

    std::shared_ptr<ProcessorHarness> processor {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorComponent)
};