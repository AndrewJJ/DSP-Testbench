/*
  ==============================================================================

    ProcessorComponent.cpp
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "ProcessorComponent.h"
#include "../Main.h"

ProcessorComponent::ProcessorComponent (const String& processorId, ProcessorHarness* processorToTest)
    :   keyName ("Processor" + processorId),
        controlArrayComponent (&controlArray)
{
    processor.reset (processorToTest);

    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config = propertiesFile->getXmlValue (keyName);
    if (!config)
    {
        // Define default properties to be used if user settings not already saved
        config = std::make_unique<XmlElement> (keyName);
        config->setAttribute ("SourceA", true);
        config->setAttribute ("SourceB", false);
        config->setAttribute ("Disable", false);
        config->setAttribute ("Invert", false);
        config->setAttribute ("Mute", true);
    }

    addAndMakeVisible (lblTitle);
    lblTitle.setText (processorToTest->getProcessorName(), dontSendNotification);
    lblTitle.setTooltip ("Processor " + processorId);
    lblTitle.setFont (Font (GUI_SIZE_F(0.7), Font::bold));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btnSourceA);
    btnSourceA.setTooltip (TRANS("Process input from source A"));
    btnSourceA.setClickingTogglesState (true);
    btnSourceA.setButtonText (TRANS("Src A"));
    btnSourceA.setColour (TextButton::buttonOnColourId, Colours::green);
    statusSourceA.set (config->getBoolAttribute ("SourceA"));
    btnSourceA.setToggleState (statusSourceA.get(), dontSendNotification);
    btnSourceA.onClick = [this] { statusSourceA = btnSourceA.getToggleState(); };

    addAndMakeVisible (btnSourceB);
    btnSourceB.setTooltip (TRANS("Process input from source B"));
    btnSourceB.setClickingTogglesState (true);
    btnSourceB.setButtonText (TRANS("Src B"));
    btnSourceB.setColour (TextButton::buttonOnColourId, Colours::green);
    statusSourceB.set (config->getBoolAttribute ("SourceB"));
    btnSourceB.setToggleState (statusSourceB.get(), dontSendNotification);
    btnSourceB.onClick = [this] { statusSourceB = btnSourceB.getToggleState(); };

    addAndMakeVisible (btnDisable);
    btnDisable.setButtonText (TRANS("Disable"));
    btnDisable.setClickingTogglesState (true);
    btnDisable.setColour (TextButton::buttonOnColourId, Colours::darkred);
    statusDisable.set (config->getBoolAttribute ("Disable"));
    btnDisable.setToggleState (statusDisable.get(), dontSendNotification);
    btnDisable.onClick = [this] { statusDisable = btnDisable.getToggleState(); };

    addAndMakeVisible (btnInvert);
    btnInvert.setButtonText (TRANS("Invert"));
    btnInvert.setClickingTogglesState (true);
    btnInvert.setColour (TextButton::buttonOnColourId, Colours::darkorange);
    statusInvert.set (config->getBoolAttribute ("Invert"));
    btnInvert.setToggleState (statusInvert.get(), dontSendNotification);
    btnInvert.onClick = [this] { statusInvert = btnInvert.getToggleState(); };

    addAndMakeVisible (btnMute);
    btnMute.setButtonText (TRANS("Mute"));
    btnMute.setClickingTogglesState (true);
    btnMute.setColour (TextButton::buttonOnColourId, Colours::darkred);
    statusMute.set (config->getBoolAttribute ("Mute"));
    btnMute.setToggleState (statusMute.get(), dontSendNotification);
    btnMute.onClick = [this] { statusMute = btnMute.getToggleState(); };

    if (!processor)
    {
        disableProcessor();
        muteProcessor();
        setEnabled (false);
    }
    else
    {
        for (auto i = 0; i < processor->getNumControls(); ++i)
            controlArray.add (new ControlComponent (i, processor.get()));
    }
    controlArrayComponent.initialiseControls();

    viewport.setScrollBarsShown (true, false);
    viewport.setViewedComponent (&controlArrayComponent);
    addAndMakeVisible (viewport);
}
ProcessorComponent::~ProcessorComponent()
{
    // Update configuration from class state
    config->setAttribute ("SourceA", statusSourceA.get());
    config->setAttribute ("SourceB", statusSourceB.get());
    config->setAttribute ("Disable", statusDisable.get());
    config->setAttribute ("Invert", statusInvert.get());
    config->setAttribute ("Mute", statusMute.get());

    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue(keyName, config.get());
    propertiesFile->saveIfNeeded();
}
void ProcessorComponent::paint (Graphics& g)
{
    g.setColour (DspTestBenchLnF::ApplicationColours::processorBackground());
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void ProcessorComponent::resized()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.templateRows = {   
        Track (GUI_BASE_SIZE_PX),
        Track (GUI_SIZE_PX(0.2)),   // Blank row
        Track (1_fr)                // Remainder used for viewport
    };
    grid.templateColumns = { Track (GUI_SIZE_PX(3)), Track (GUI_SIZE_PX(1.5)), Track (1_fr), Track (GUI_SIZE_PX(1.8)), Track (GUI_SIZE_PX(1.8)), Track(GUI_BASE_GAP_PX), Track (GUI_SIZE_PX(2.2)), Track (GUI_SIZE_PX(2)), Track (GUI_SIZE_PX(1.7)) };
    grid.autoFlow = Grid::AutoFlow::row;
    grid.items.addArray({  
        GridItem (lblTitle).withArea ({}, GridItem::Span (2)),
        GridItem(),
        GridItem (btnSourceA),
        GridItem (btnSourceB),
        GridItem(),
        GridItem (btnDisable),
        GridItem (btnInvert),
        GridItem (btnMute),
        GridItem().withArea ({}, GridItem::Span (9)), // Blank row
        GridItem (viewport).withArea ({}, GridItem::Span (9))
    });

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));

    const auto controlWidth = viewport.getWidth() - viewport.getLookAndFeel().getDefaultScrollbarWidth();
    controlArrayComponent.setSize (controlWidth, static_cast<int> (controlArrayComponent.getPreferredHeight()));
}
float ProcessorComponent::getPreferredHeight() const
{
    // This is the height needed to show 3 controls (176 pixels converted back to GUI units for a base size of 30.0)
    const auto maxHeight = GUI_SIZE_F(5.866666666f);

    // This is the height needed to show all controls
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_SIZE_F(1 + 0.2) + controlArrayComponent.getPreferredHeight();
    const auto totalItemGaps = GUI_GAP_F(2);
    const auto height = innerMargin + totalItemHeight + totalItemGaps;

    return  jmin (height, maxHeight);
}
void ProcessorComponent::prepare (const dsp::ProcessSpec& spec)
{
    if (processor)
        processor->prepareHarness (spec);
}
void ProcessorComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (processor)
        processor->processHarness (context);
    if (statusMute.get())
        context.getOutputBlock().clear();
}
void ProcessorComponent::reset()
{
    if (processor)
        processor->resetHarness();
}
bool ProcessorComponent::isSourceConnectedA() const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusSourceA.get();
}
bool ProcessorComponent::isSourceConnectedB() const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusSourceB.get();
}
bool ProcessorComponent::isProcessorEnabled() const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return !statusDisable.get();
}
bool ProcessorComponent::isInverted() const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusInvert.get();
}
bool ProcessorComponent::isMuted() const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusMute.get();
}
void ProcessorComponent::muteProcessor (const bool shouldBeMuted)
{
    btnMute.setToggleState (shouldBeMuted, sendNotificationSync);
}
void ProcessorComponent::disableProcessor (const bool shouldBeDisabled)
{
    btnDisable.setToggleState (shouldBeDisabled, sendNotificationSync);
}

ProcessorComponent::ControlComponent::ControlComponent (const int index, ProcessorHarness* processorBeingControlled)
    : controlIndex (index),
      processor (processorBeingControlled)
{
    auto controlName = "Control " + String (index + 1);
    auto defaultControlValue = 0.0;
    if (processor)
    {
        controlName = processor->getControlName (controlIndex);
        defaultControlValue = processor->getDefaultControlValue (controlIndex);
    }
    lblControl.setText (controlName, dontSendNotification);
    lblControl.setFont (Font (GUI_SIZE_F(0.5), Font::plain).withTypefaceStyle ("Regular"));
    lblControl.setJustificationType (Justification::centredLeft);
    lblControl.setEditable (false, false, false);
    lblControl.setColour (TextEditor::textColourId, Colours::black);
    lblControl.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible (lblControl);

    sldControl.setRange (0, 1, 0.001);
    sldControl.setDoubleClickReturnValue (true, defaultControlValue);
    sldControl.setSliderStyle (Slider::LinearHorizontal);
    sldControl.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldControl.onValueChange = [this]
    {
        processor->setControlValue (controlIndex, sldControl.getValue());
    };
    sldControl.setValue (defaultControlValue, sendNotificationSync);
    addAndMakeVisible (sldControl);
}
void ProcessorComponent::ControlComponent::paint (Graphics&)
{ }
void ProcessorComponent::ControlComponent::resized()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = { Track (GUI_BASE_SIZE_PX) };
    
    grid.templateColumns = { Track (GUI_SIZE_PX (3)), Track (1_fr) };

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({ GridItem (lblControl), GridItem (sldControl) });

    grid.performLayout (getLocalBounds());
}
double ProcessorComponent::ControlComponent::getCurrentControlValue() const
{
    return currentControlValue.get();
}
ProcessorComponent::ControlArrayComponent::ControlArrayComponent (OwnedArray<ControlComponent>* controlComponentsToReferTo)
    : controlComponents (controlComponentsToReferTo)
{
}
void ProcessorComponent::ControlArrayComponent::paint (Graphics&)
{ }
void ProcessorComponent::ControlArrayComponent::resized()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);
    grid.autoFlow = Grid::AutoFlow::row;

    for (auto controlComponent : *controlComponents)
        grid.items.add (GridItem (controlComponent));

    grid.performLayout (getLocalBounds().withTrimmedRight (GUI_BASE_GAP_I));
}
float ProcessorComponent::ControlArrayComponent::getPreferredHeight() const
{
    // This is an exact calculation of the height we want
    const auto numControls = controlComponents->size();
    const auto innerMargin = GUI_GAP_F(2);
    const auto totalItemHeight = GUI_SIZE_F(numControls);
    const auto totalItemGaps = GUI_GAP_F(numControls - 1);
    return innerMargin + totalItemHeight + totalItemGaps;
}
void ProcessorComponent::ControlArrayComponent::initialiseControls()
{
    deleteAllChildren();
    for (auto controlComponent : *controlComponents)
        addAndMakeVisible (controlComponent);
}
