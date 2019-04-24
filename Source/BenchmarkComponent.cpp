/*
  ==============================================================================

    BenchmarkComponent.cpp
    Created: 24 Apr 2019 9:26:12am
    Author:  Andrew

  ==============================================================================
*/

#include "BenchmarkComponent.h"
#include "LookAndFeel.h"

BenchmarkComponent::BenchmarkComponent (ProcessorHarness* processorHarnessA, ProcessorHarness* processorHarnessB)
{
    jassert (values.size() == valueTooltips.size());

    harnesses.emplace_back (processorHarnessA);
    harnesses.emplace_back (processorHarnessB);

    using cols = DspTestBenchLnF::ApplicationColours;

    // Initialise labels
    for (const auto& p : processors)
    {
        auto* lblP = processorLabels.add (new Label ("", p));
        lblP->setFont (titleFont);
        lblP->setColour (Label::backgroundColourId, cols::benchmarkHeadingBackground());
        lblP->setColour (Label::textColourId, cols::titleFontColour());
        addAndMakeVisible (lblP);
        for (const auto& r : routines)
        {
            auto* lblR = routineLabels.add (new Label ("", r));
            lblR->setFont (normalFont);
            if (r == "Process")
                lblR->setColour (Label::backgroundColourId, cols::benchmarkAlternateRow());
            else
                lblR->setColour (Label::backgroundColourId, cols::benchmarkRow());
            lblR->setColour (Label::textColourId, cols::titleFontColour());
            addAndMakeVisible (lblR);
            for (auto v = 0; v < static_cast<int> (values.size()); ++v)
            {
                auto* lblV = valueLabels.add (new Label ("", "-"));
                lblV->setFont (normalFont);
                if (r == "Process")
                    lblV->setColour (Label::backgroundColourId, cols::benchmarkAlternateRow());
                else
                    lblV->setColour (Label::backgroundColourId, cols::benchmarkRow());
                lblV->setJustificationType (Justification::centred);
                lblR->setColour (Label::textColourId, cols::normalFontColour());
                lblV->setTooltip (valueTooltips[v]);
                addAndMakeVisible (lblV);
            }
        }
        for (auto v = 0; v < static_cast<int> (values.size()); ++v)
        {
            auto* lblVT = valueTitleLabels.add (new Label ("", values[v]));
            lblVT->setTooltip (valueTooltips[v]);
            lblVT->setFont (normalFont);
            lblVT->setColour (Label::textColourId, cols::titleFontColour());
            lblVT->setColour (Label::backgroundColourId, cols::benchmarkHeadingBackground());
            lblVT->setJustificationType (Justification::centred);
            addAndMakeVisible (lblVT);
        }
    }

    setSize (690, 290);

    // TODO - start running benchmarks
    // TODO - maybe allow user to run using different block sizes?

    startTimerHz (5);
}
void BenchmarkComponent::paint(Graphics & g)
{
    g.fillAll (DspTestBenchLnF::ApplicationColours::componentBackground());
}
void BenchmarkComponent::resized()
{
    using Track = Grid::TrackInfo;

    const auto titleRowHeight = Grid::Px (titleFont.getHeight() * 1.15f);
    const auto valueRowHeight = Grid::Px (normalFont.getHeight() * 1.25f);
    const auto titleColumnWidth = GUI_SIZE_PX (5.0);
    const auto valueColumnWidth = GUI_SIZE_PX (3.7);
    const auto gap = GUI_BASE_GAP_PX;

    Grid grid;
    grid.rowGap = gap;
    grid.columnGap = gap;
    grid.templateRows = {
        Track (1_fr),               // row  1 is for centering
        Track (titleRowHeight),     // row  2 is for processor A title and value column titles
        Track (valueRowHeight),     // row  3 is for processor A prepare routine results
        Track (valueRowHeight),     // row  4 is for processor A processing routine results
        Track (valueRowHeight),     // row  5 is for processor A reset routine results
        Track (GUI_GAP_PX (2)),     // row  6 is the blank row between processors
        Track (titleRowHeight),     // row  7 is for processor B title and value column titles
        Track (valueRowHeight),     // row  8 is for processor B prepare routine results
        Track (valueRowHeight),     // row  9 is for processor B processing routine results
        Track (valueRowHeight),     // row 10 is for processor B reset routine results
        Track (1_fr)                // row 11 is for centering
    };
    grid.templateColumns = {
        Track (1_fr),               // column 1 is for centering 
        Track (titleColumnWidth),   // column 2 is for titles
        Track (valueColumnWidth),   // column 3 is for min values
        Track (valueColumnWidth),   // column 4 is for avg values
        Track (valueColumnWidth),   // column 5 is for max values
        Track (valueColumnWidth),   // column 6 is for sample counts
        Track (1_fr)                // column 7 is for centering
    };

    grid.items.addArray({

        GridItem().withArea (1, 1),

        GridItem (processorLabels[0]).withArea (2, 2),
        GridItem (valueTitleLabels[0]).withArea (2, 3),
        GridItem (valueTitleLabels[1]).withArea (2, 4),
        GridItem (valueTitleLabels[2]).withArea (2, 5),
        GridItem (valueTitleLabels[3]).withArea (2, 6),
        GridItem (routineLabels[0]).withArea (3, 2),
        GridItem (routineLabels[1]).withArea (4, 2),
        GridItem (routineLabels[2]).withArea (5, 2),

        GridItem().withArea (6, 1),

        GridItem (processorLabels[1]).withArea (7, 2),
        GridItem (valueTitleLabels[4]).withArea (7, 3),
        GridItem (valueTitleLabels[5]).withArea (7, 4),
        GridItem (valueTitleLabels[6]).withArea (7, 5),
        GridItem (valueTitleLabels[7]).withArea (7, 6),
        GridItem (routineLabels[3]).withArea (8, 2),
        GridItem (routineLabels[4]).withArea ( 9, 2),
        GridItem (routineLabels[5]).withArea (10, 2),
        
        GridItem().withArea (11, 7)
    });

    const auto numRoutines = static_cast<int> (routines.size());
    const auto numValues = static_cast<int> (values.size());

    for (auto p = 0; p < static_cast<int> (processors.size()); ++p)
    {
        const auto offsetP = p * (numRoutines + 2) + 3;
        for (auto r = 0; r < numRoutines; ++r)
        {
            for (auto v = 0; v < numValues; ++v)
            {
                const auto idxLabel = getValueLabelIndex (p, r, v);
                const auto row = offsetP + r;
                const auto col = v + 3;
                grid.items.add (GridItem (valueLabels[idxLabel]).withArea (row, col));
            }
        }
    }
    grid.performLayout (getLocalBounds());
}
void BenchmarkComponent::timerCallback()
{
    for (auto p = 0; p < static_cast<int> (processors.size()); ++p)
    {
        if (auto* harness = harnesses[p])
        {
            for (auto r = 0; r < static_cast<int> (routines.size()); ++r)
            {
                for (auto v = 0; v < static_cast<int> (values.size()); ++v)
                {
                    const auto idxLabel = getValueLabelIndex (p, r, v);
                    const auto queryValue = harness->queryByIndex (r, v);
                    auto txt = String ("-");
                    switch (v)
                    {
                        case 0: if (queryValue < 1.0E100)
                                    txt = String (queryValue, 3);
                                break;
                        case 1: if (isfinite(queryValue))
                                    txt = String (queryValue, 3);
                                break;
                        case 2: if (queryValue > 0.0)
                                    txt = String (queryValue, 3);
                                break;
                        case 3: if (queryValue > 0.0)
                                    txt = String (queryValue, 0);
                                break;
                        default: break;
                    }
                    valueLabels[idxLabel]->setText (txt, sendNotificationAsync);
                }
            }
        }
    }
}
int BenchmarkComponent::getValueLabelIndex (const int processorIndex, const int routineIndex, const int valueIndex) const
{
    const auto offset = (processorIndex == 0) ? 0 : static_cast<int> (routines.size() * values.size());
    return ProcessorHarness::getQueryIndex (routineIndex, valueIndex) + offset;
}
