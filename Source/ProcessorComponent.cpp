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
    lblTitle->setFont (Font (15.00f, Font::bold));
    lblTitle->setJustificationType (Justification::topLeft);
    lblTitle->setEditable (false, false, false);
    lblTitle->setColour (TextEditor::textColourId, Colours::black);
    lblTitle->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btnSourceA = new TextButton ("Source A button"));
    btnSourceA->setTooltip (TRANS("Process input from source A"));
    btnSourceA->setClickingTogglesState (true);
    btnSourceA->setButtonText (TRANS("Source A"));
    btnSourceA->setColour(TextButton::buttonOnColourId, Colours::green);
    btnSourceA->setToggleState(true, dontSendNotification);

    addAndMakeVisible (btnSourceB = new TextButton ("Source B button"));
    btnSourceB->setTooltip (TRANS("Process input from source B"));
    btnSourceB->setClickingTogglesState (true);
    btnSourceB->setButtonText (TRANS("Source B"));
    btnSourceB->setColour(TextButton::buttonOnColourId, Colours::green);
    btnSourceB->setToggleState(false, dontSendNotification);

    addAndMakeVisible (btnMute = new TextButton ("Mute button"));
    btnMute->setButtonText (TRANS("Mute"));
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);

    addAndMakeVisible (btnDisable = new TextButton ("Disable button"));
    btnDisable->setButtonText (TRANS("Disable"));
    btnDisable->setClickingTogglesState (true);
    btnDisable->setColour(TextButton::buttonOnColourId, Colours::darkred);

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

    //setSize (400, 300);
}

ProcessorComponent::~ProcessorComponent()
{
    lblTitle = nullptr;
    btnSourceA = nullptr;
    btnSourceB = nullptr;
    btnMute = nullptr;
    btnDisable = nullptr;
}

void ProcessorComponent::paint (Graphics& g)
{
    const Colour fillColour = Colour (0x300081ff);
    g.setColour (fillColour);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), 10.000f);
}

void ProcessorComponent::resized()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (btnSourceA),
                            GridItem (btnSourceB),
                            GridItem (btnDisable),
                            GridItem (btnMute)
                        });
    for (auto i = 0; i<numControls; ++i)
    {
        grid.items.addArray({ GridItem (sliderLabels[i]),
                              GridItem (sliders[i]).withArea ({}, GridItem::Span (4))
                            });
    }

    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}

void ProcessorComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sliders[0])
    {
    }
    else if (sliderThatWasMoved == sliders[1])
    {
    }
}

bool ProcessorComponent::isSourceConnectedA () const
{
    return btnSourceA->getToggleState();
}

bool ProcessorComponent::isSourceConnectedB () const
{
    return btnSourceB->getToggleState();
}

bool ProcessorComponent::isMuted () const
{
    return btnMute->getToggleState();
}

bool ProcessorComponent::isProcessorEnabled () const
{
    return !btnDisable->getToggleState();
}
