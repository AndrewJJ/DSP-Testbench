/*
  ==============================================================================

    GuiComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SourceComponent.h"
#include "ProcessorComponent.h"
#include "MonitoringComponent.h"
#include "AnalyserComponent.h"


class GuiComponent  : public Component
{
public:

    GuiComponent ();
    ~GuiComponent();

    void paint (Graphics& g) override;
    void resized() override;

private:

    ScopedPointer<SourceComponent> srcComponentA;
    ScopedPointer<SourceComponent> srcComponentB;
    ScopedPointer<ProcessorComponent> procComponentA;
    ScopedPointer<ProcessorComponent> procComponentB;
    ScopedPointer<AnalyserComponent> analyserComponent;
    ScopedPointer<MonitoringComponent> monitoringComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuiComponent)
};
