/*
  ==============================================================================

    GuiComponent.cpp
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "GuiComponent.h"


GuiComponent::GuiComponent ()
{
    addAndMakeVisible (srcComponentA = new SourceComponent ("A"));
    addAndMakeVisible (srcComponentB = new SourceComponent ("B"));
    addAndMakeVisible (procComponentA = new ProcessorComponent ("A", 3));
    addAndMakeVisible (procComponentB = new ProcessorComponent ("B", 3));
    addAndMakeVisible (analyserComponent = new AnalyserComponent());
    addAndMakeVisible (monitoringComponent = new MonitoringComponent());

    setSize (1024, 768);
}

GuiComponent::~GuiComponent()
{
    srcComponentA = nullptr;
    srcComponentB = nullptr;
    procComponentA = nullptr;
    procComponentB = nullptr;
    analyserComponent = nullptr;
    monitoringComponent = nullptr;
}

void GuiComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xff323e44));
}

void GuiComponent::resized()
{
    Grid grid;
    grid.rowGap = 10_px;
    grid.columnGap = 10_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (3_fr),
                            Track (2_fr),
                            Track (4_fr),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (srcComponentA),
                            GridItem (srcComponentB),
                            GridItem (procComponentA),
                            GridItem (procComponentB),
                            GridItem (analyserComponent).withArea({ }, GridItem::Span (2)),
                            GridItem (monitoringComponent).withArea({ }, GridItem::Span (2))
                        });

    const auto marg = 10;
    // .withTrimmedTop(proportionOfHeight(0.1f))
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}