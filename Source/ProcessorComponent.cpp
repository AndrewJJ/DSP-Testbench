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
    addAndMakeVisible (lblProcessor = new Label ("Processor label", TRANS("Processor") + " " + processorId));
    lblProcessor->setFont (Font (15.00f, Font::bold));
    lblProcessor->setJustificationType (Justification::topLeft);
    lblProcessor->setEditable (false, false, false);
    lblProcessor->setColour (TextEditor::textColourId, Colours::black);
    lblProcessor->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (btnSourceA = new ToggleButton ("Source A toggle button"));
    btnSourceA->setTooltip (TRANS("Process input from source A"));
    btnSourceA->setButtonText (TRANS("Source A"));
    btnSourceA->addListener (this);

    addAndMakeVisible (btnSourceB = new ToggleButton ("Source B toggle button"));
    btnSourceB->setTooltip (TRANS("Process input from source B"));
    btnSourceB->setButtonText (TRANS("Source B"));
    btnSourceB->addListener (this);

    addAndMakeVisible (btnMute = new TextButton ("Mute button"));
    btnMute->setButtonText (TRANS("Mute"));
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnMute->addListener (this);

    addAndMakeVisible (btnDisable = new TextButton ("Disable button"));
    btnDisable->setButtonText (TRANS("Disable"));
    btnDisable->setClickingTogglesState (true);
    btnDisable->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnDisable->addListener (this);

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
    lblProcessor = nullptr;
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
    //lblProcessor->setBounds (proportionOfWidth (0.0207f), proportionOfHeight (0.0277f), proportionOfWidth (0.3343f), proportionOfHeight (0.1423f));
    //btnSourceA->setBounds (proportionOfWidth (0.0385f), proportionOfHeight (0.2134f), proportionOfWidth (0.2604f), proportionOfHeight (0.0791f));
    //btnSourceB->setBounds (proportionOfWidth (0.2988f), proportionOfHeight (0.2134f), proportionOfWidth (0.2604f), proportionOfHeight (0.0791f));
    //btnMute->setBounds (proportionOfWidth (0.8402f), proportionOfHeight (0.0514f), proportionOfWidth (0.1213f), proportionOfHeight (0.0791f));
    //btnDisable->setBounds (proportionOfWidth (0.7012f), proportionOfHeight (0.0514f), proportionOfWidth (0.1213f), proportionOfHeight (0.0791f));

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

    grid.items.addArray({   GridItem (lblProcessor),
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
    //const auto marg = jmin (proportionOfWidth(0.05f), proportionOfHeight(0.05f));
    //.withTrimmedTop(proportionOfHeight(0.32f))
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}

void ProcessorComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == btnSourceA)
    {
    }
    else if (buttonThatWasClicked == btnSourceB)
    {
    }
    else if (buttonThatWasClicked == btnMute)
    {
    }
    else if (buttonThatWasClicked == btnDisable)
    {
    }
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