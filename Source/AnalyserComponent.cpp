/*
  ==============================================================================

    AnalyserComponent.cpp
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "AnalyserComponent.h"

AnalyserComponent::AnalyserComponent ()
{
    addAndMakeVisible (lblTitle = new Label ("Analyser label", TRANS("Analyser")));
    lblTitle->setFont (Font (15.00f, Font::bold));
    lblTitle->setJustificationType (Justification::topLeft);
    lblTitle->setEditable (false, false, false);
    lblTitle->setColour (TextEditor::textColourId, Colours::black);
    lblTitle->setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    
    addAndMakeVisible (btnDisable = new TextButton ("Disable button"));
    btnDisable->setButtonText ("Disable");
    btnDisable->setClickingTogglesState (true);
    btnDisable->setColour(TextButton::buttonOnColourId, Colours::darkred);
    btnDisable->onClick = [this] { statusActive = !btnDisable->getToggleState(); };
}
AnalyserComponent::~AnalyserComponent()
{
    lblTitle = nullptr;
    btnDisable = nullptr;
}
void AnalyserComponent::paint (Graphics& g)
{
    g.setColour (Colours::black);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), 10.000f);
}
void AnalyserComponent::resized()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr),
                            Track (6_fr)
                        };

    //grid.templateColumns = { Track (1_fr), Track (6_fr), Track (1_fr), Track (1_fr) };
    grid.templateColumns = { Track (6_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (btnDisable).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f))
                        });

    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void AnalyserComponent::prepare (const dsp::ProcessSpec&)
{
    // TODO - AnalyserComponent::prepare
}
void AnalyserComponent::process (const dsp::ProcessContextReplacing<float>&)
{
    // TODO - AnalyserComponent::process
}
void AnalyserComponent::reset ()
{
    // TODO - AnalyserComponent::reset
}
bool AnalyserComponent::isActive () const
{
    return statusActive;
}
