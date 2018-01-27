/*
  ==============================================================================

    ProcessorComponent.cpp
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "ProcessorComponent.h"

ProcessorComponent::ProcessorComponent (const String processorId, const int numberOfControls)
    : numControls (numberOfControls)
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

    // Assumes sliderLabels and sliders are empty
    for (auto i = 0; i<numControls; ++i)
    {
        addAndMakeVisible (sliderLabels.add (new Label ("Parameter " + String(i+1) + " slider label", TRANS ("Parameter") + " " + String (i+1))));
        sliderLabels[i]->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
        sliderLabels[i]->setJustificationType (Justification::centredLeft);
        sliderLabels[i]->setEditable (false, false, false);
        sliderLabels[i]->setColour (TextEditor::textColourId, Colours::black);
        sliderLabels[i]->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (sliders.add (new Slider("Parameter " + String(i+1) + " slider")));
        sliders[i]->setRange (0, 1, 0);
        sliders[i]->setSliderStyle (Slider::LinearHorizontal);
        sliders[i]->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
        sliders[i]->addListener (this);
    }
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

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX)
                        };
    
    grid.templateColumns = { Track (GUI_SIZE_PX(3)), Track (GUI_SIZE_PX(1.5)), Track (GUI_SIZE_PX(1.8)), Track (GUI_SIZE_PX(1.8)), Track (1_fr), Track (GUI_SIZE_PX(2.1)), Track (GUI_SIZE_PX(2)), Track (GUI_SIZE_PX(1.7)) };

    grid.autoRows = Track (GUI_BASE_SIZE_PX);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle).withArea ({}, GridItem::Span (2)),
                            GridItem (btnSourceA),
                            GridItem (btnSourceB),
                            GridItem(),
                            GridItem (btnDisable),
                            GridItem (btnInvert),
                            GridItem (btnMute)
                        });

    // TODO - use a viewport for control sliders
    for (auto i = 0; i<numControls; ++i)
    {
        grid.items.addArray({ GridItem (sliderLabels[i]),
                              GridItem (sliders[i]).withArea ({}, GridItem::Span (7))
                            });
    }

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
float ProcessorComponent::getMinimumWidth() const
{
    // TODO
    return 0.0f;
}
float ProcessorComponent::getMinimumHeight() const
{
    // This is an exact calculation of the height we want
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_SIZE_F(1 + numControls);
    const auto totalItemGaps = GUI_BASE_GAP_F * numControls;
    return innerMargin + totalItemHeight + totalItemGaps;
}
void ProcessorComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    // TODO - implement processor control (local variables for normalised parameter values)
    if (sliderThatWasMoved == sliders[0])
    {
    }
    else if (sliderThatWasMoved == sliders[1])
    {
    }
}
void ProcessorComponent::prepare (const dsp::ProcessSpec&)
{
    // TODO - ProcessorComponent::prepare
}
void ProcessorComponent::process (const dsp::ProcessContextReplacing<float>&)
{
    // TODO - ProcessorComponent::process
}
void ProcessorComponent::reset ()
{
    // TODO - ProcessorComponent::reset
}
bool ProcessorComponent::isSourceConnectedA () const
{
    // We use a local variable so method is safe to use for audio processing
    return statusSourceA;
}
bool ProcessorComponent::isSourceConnectedB () const
{
    // We use a local variable so method is safe to use for audio processing
    return statusSourceB;
}
bool ProcessorComponent::isProcessorEnabled () const
{
    // We use a local variable so method is safe to use for audio processing
    return !statusDisable;
}
bool ProcessorComponent::isInverted () const
{
    // We use a local variable so method is safe to use for audio processing
    return statusInvert;
}
bool ProcessorComponent::isMuted () const
{
    // We use a local variable so method is safe to use for audio processing
    return statusMute;
}
bool ProcessorComponent::isActive () const
{
    return !statusDisable && !statusMute;
}
void ProcessorComponent::mute ()
{
    btnMute->setToggleState (true, sendNotificationSync);
}
