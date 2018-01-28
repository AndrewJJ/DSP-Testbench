/*
  ==============================================================================

    ProcessorComponent.cpp
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "ProcessorComponent.h"

ProcessorComponent::ProcessorComponent (const String processorId, const int numberOfControls)
    :   controlArrayComponent (&controlArray)
{
    addAndMakeVisible (lblTitle = new Label ("Processor label", TRANS("Processor") + " " + processorId));
    lblTitle->setFont (Font (GUI_SIZE_F(0.7), Font::bold));
    lblTitle->setJustificationType (Justification::topLeft);
    lblTitle->setEditable (false, false, false);
    lblTitle->setColour (TextEditor::textColourId, Colours::black);
    lblTitle->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btnSourceA = new TextButton ("Source A button"));
    btnSourceA->setTooltip (TRANS("Process input from source A"));
    btnSourceA->setClickingTogglesState (true);
    btnSourceA->setButtonText (TRANS("Src A"));
    btnSourceA->setColour(TextButton::buttonOnColourId, Colours::green);
    btnSourceA->onClick = [this] { statusSourceA = btnSourceA->getToggleState(); };
    btnSourceA->setToggleState(true, dontSendNotification);

    addAndMakeVisible (btnSourceB = new TextButton ("Source B button"));
    btnSourceB->setTooltip (TRANS("Process input from source B"));
    btnSourceB->setClickingTogglesState (true);
    btnSourceB->setButtonText (TRANS("Src B"));
    btnSourceB->setColour(TextButton::buttonOnColourId, Colours::green);
    btnSourceB->onClick = [this] { statusSourceB = btnSourceB->getToggleState(); };
    btnSourceB->setToggleState(false, dontSendNotification);

    addAndMakeVisible (btnDisable = new TextButton ("Disable button"));
    btnDisable->setButtonText (TRANS("Disable"));
    btnDisable->setClickingTogglesState (true);
    btnDisable->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnDisable->onClick = [this] { statusDisable = btnDisable->getToggleState(); };

    addAndMakeVisible (btnInvert = new TextButton ("Invert button"));
    btnInvert->setButtonText (TRANS("Invert"));
    btnInvert->setClickingTogglesState (true);
    btnInvert->setColour(TextButton::buttonOnColourId, Colours::green);
    btnInvert->onClick = [this] { statusInvert = btnInvert->getToggleState(); };

    addAndMakeVisible (btnMute = new TextButton ("Mute button"));
    btnMute->setButtonText (TRANS("Mute"));
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnMute->onClick = [this] { statusMute = btnMute->getToggleState(); };

    for (auto i = 0; i < numberOfControls; ++i)
        controlArray.add (new ControlComponent ("Control " + String (i+1)));
    controlArrayComponent.initialiseControls();

    viewport.setScrollBarsShown (true, false);
    viewport.setViewedComponent (&controlArrayComponent);
    addAndMakeVisible (viewport);
}
ProcessorComponent::~ProcessorComponent()
{
    lblTitle = nullptr;
    btnSourceA = nullptr;
    btnSourceB = nullptr;
    btnDisable = nullptr;
    btnInvert = nullptr;
    btnMute = nullptr;
}
void ProcessorComponent::paint (Graphics& g)
{
    const Colour fillColour = Colour (0x300081ff);
    g.setColour (fillColour);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void ProcessorComponent::resized()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX),
                            Track (GUI_SIZE_PX(0.2)),   // Blank row
                            Track (1_fr)                // Remainder used for viewport
                        };
    
    grid.templateColumns = { Track (GUI_SIZE_PX(3)), Track (GUI_SIZE_PX(1.5)), Track (GUI_SIZE_PX(1.8)), Track (GUI_SIZE_PX(1.8)), Track (1_fr), Track (GUI_SIZE_PX(2.1)), Track (GUI_SIZE_PX(2)), Track (GUI_SIZE_PX(1.7)) };

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle).withArea ({}, GridItem::Span (2)),
                            GridItem (btnSourceA),
                            GridItem (btnSourceB),
                            GridItem(),
                            GridItem (btnDisable),
                            GridItem (btnInvert),
                            GridItem (btnMute),
                            GridItem().withArea ({}, GridItem::Span (8)), // Blank row
                            GridItem (viewport).withArea ({}, GridItem::Span (8))
                        });

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));

    const auto controlWidth = viewport.getWidth() - viewport.getLookAndFeel().getDefaultScrollbarWidth();
    controlArrayComponent.setSize (controlWidth, static_cast<int> (controlArrayComponent.getPreferredHeight()));
}
float ProcessorComponent::getMinimumWidth() const
{
    // Nominal value, because we are happy to have same width as the source component
    return GUI_SIZE_F(10);
}
float ProcessorComponent::getMinimumHeight() const
{
    // This is an exact calculation of the height we want
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_SIZE_F(1 + 0.2) + controlArrayComponent.getPreferredHeight();
    const auto totalItemGaps = GUI_GAP_F(2);
    return innerMargin + totalItemHeight + totalItemGaps;
}
void ProcessorComponent::prepare (const dsp::ProcessSpec&)
{
    // TODO - ProcessorComponent::prepare
}
void ProcessorComponent::process (const dsp::ProcessContextReplacing<float>&)
{
    // TODO - ProcessorComponent::process
    //const auto controlValue0 = controlArray[0]->getCurrentControlValue();
}
void ProcessorComponent::reset ()
{
    // TODO - ProcessorComponent::reset
}
bool ProcessorComponent::isSourceConnectedA () const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusSourceA.get();
}
bool ProcessorComponent::isSourceConnectedB () const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusSourceB.get();
}
bool ProcessorComponent::isProcessorEnabled () const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return !statusDisable.get();
}
bool ProcessorComponent::isInverted () const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusInvert.get();
}
bool ProcessorComponent::isMuted () const noexcept
{
    // We use a local variable so method is safe to use for audio processing
    return statusMute.get();
}
bool ProcessorComponent::isActive () const noexcept
{
    return !statusDisable.get() && !statusMute.get();
}
void ProcessorComponent::mute ()
{
    btnMute->setToggleState (true, sendNotificationSync);
}


ProcessorComponent::ControlComponent::ControlComponent (String controlName)
    : lblControl (controlName),
      sldControl (controlName + " slider")
{
    lblControl.setText (controlName, dontSendNotification);
    lblControl.setFont (Font (GUI_SIZE_F(0.5), Font::plain).withTypefaceStyle ("Regular"));
    lblControl.setJustificationType (Justification::centredLeft);
    lblControl.setEditable (false, false, false);
    lblControl.setColour (TextEditor::textColourId, Colours::black);
    lblControl.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    addAndMakeVisible (lblControl);

    sldControl.setRange (0, 1, 0.001);
    sldControl.setSliderStyle (Slider::LinearHorizontal);
    sldControl.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldControl.addListener (this);
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

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX)
                        };
    
    grid.templateColumns = { Track (GUI_SIZE_PX(3)), Track (1_fr) };

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblControl),
                            GridItem (sldControl)
                        });

    grid.performLayout (getLocalBounds());
}
void ProcessorComponent::ControlComponent::sliderValueChanged (Slider* sliderThatWasChanged)
{
    if (sliderThatWasChanged == &sldControl)
        currentControlValue.set (sldControl.getValue());
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

    grid.performLayout (getLocalBounds());
}
float ProcessorComponent::ControlArrayComponent::getPreferredHeight() const
{
    // This is an exact calculation of the height we want
    const auto numControls = controlComponents->size();
    const auto innerMargin = GUI_GAP_F(2);
    const auto totalItemHeight = GUI_SIZE_F(numControls);
    const auto totalItemGaps = GUI_GAP_F(numControls -1);
    return innerMargin + totalItemHeight + totalItemGaps;
}
void ProcessorComponent::ControlArrayComponent::initialiseControls()
{
    deleteAllChildren();
    for (auto controlComponent : *controlComponents)
        addAndMakeVisible (controlComponent);
}
