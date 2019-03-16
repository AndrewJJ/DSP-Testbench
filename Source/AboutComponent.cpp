/*
  ==============================================================================

    AboutComponent.cpp
    Created: 16 Mar 2019 4:17:47pm
    Author:  Andrew

  ==============================================================================
*/

#include "AboutComponent.h"

AboutComponent::AboutComponent ()
    : resizableBorderComponent (this, &constrainer)
{
    addAndMakeVisible (lblVersion);
    lblVersion.setText (TRANS ("Version"), dontSendNotification);
    lblVersion.setJustificationType (Justification::centredRight);
	
    addAndMakeVisible (txtVersion);
    txtVersion.setMultiLine (false);
    txtVersion.setReadOnly (true);
    txtVersion.setCaretVisible (false);
    txtVersion.setPopupMenuEnabled (false);
    txtVersion.setColour (TextEditor::ColourIds::outlineColourId, Colours::transparentBlack);
    txtVersion.setText (String (ProjectInfo::versionString));

    addAndMakeVisible (lblCredits);
    lblCredits.setText (TRANS ("Credits"), dontSendNotification);
    lblCredits.setJustificationType (Justification::topRight);

    addAndMakeVisible (txtCredits);
    txtCredits.setMultiLine (true);
    txtCredits.setReadOnly (true);
    txtCredits.setCaretVisible (false);
    txtCredits.setPopupMenuEnabled (false);
    txtCredits.setColour (TextEditor::ColourIds::outlineColourId, Colours::transparentBlack);
    txtCredits.setText (CharPointer_UTF8 ("ASIO Interface Technology by Steinberg Media Technologies GmbH\n"
    "\n"
    "This software makes use of certain code libraries, those portions of code are copyright as per below:\n\n"
    "\xE2\x80\xA2 JUCE 5 - copyright \xc2\xa9 2018 Raw Material Software\n"
    "\xE2\x80\xA2 Oblique Audio - copyright \xc2\xa9 2018\n"
    "\xE2\x80\xA2 Fast maths approximations - \xc2\xa9 Paul Mineiro 2011\n"
    "\xE2\x80\xA2 rand31pmc white noise generator - \xc2\xa9 Robin Whittle 2005\n"
    "\xE2\x80\xA2 Pink noise filter - Paul Kellett\n"
    "\xE2\x80\xA2 PolyBLEP/BLAMP adapted from Tebjan Halm (vvvv.org)\n"
    ));

    addAndMakeVisible (lblDisclaimer);
    lblDisclaimer.setText (TRANS ("Software Disclaimer"), dontSendNotification);
    lblDisclaimer.setJustificationType (Justification::topRight);

    addAndMakeVisible (txtDisclaimer);
    txtDisclaimer.setMultiLine (true);
    txtDisclaimer.setReadOnly (true);
    txtDisclaimer.setCaretVisible (false);
    txtDisclaimer.setPopupMenuEnabled (false);
    txtDisclaimer.setColour (TextEditor::ColourIds::outlineColourId, Colours::transparentBlack);
    txtDisclaimer.setText ("This software is provided \'\'as is\'\' and you use it at your own risk.\n"
    "\n"
    "The developers make no warranties as to performance, merchantability, fitness for a particular purpose, or any other warranties whether expressed or implied.\n"
    "\n"
    "No oral or written communication from or information provided by the developers shall create a warranty.\n"
    "\n"
    "Under no circumstances shall the developers or contributors be liable for direct, indirect, special, incidental, or consequential damages resulting from the use, misuse, or inability to use this software, even if the developers have been advised of the possibility of such damages.\n"
    "\n"
    "These exclusions and limitations may not apply in all jurisdictions. You may have additional rights and some of these limitations may not apply to you.");

    setSize (800, 600);
}

void AboutComponent::paint(Graphics & g)
{
    g.fillAll (Colour (0xff323e44));
}

void AboutComponent::resized()
{
    resizableBorderComponent.setBounds (getBounds());

    using Track = Grid::TrackInfo;

    Grid grid;
    grid.rowGap = GUI_GAP_PX(2);
    grid.columnGap = GUI_GAP_PX(2);
    grid.templateRows = { Track (GUI_BASE_SIZE_PX), Track (10_fr), Track (13_fr) };
    grid.templateColumns = { Track (GUI_SIZE_PX(4.2)), Track (1_fr) };
    grid.items.addArray({
                            GridItem (lblVersion),
                            GridItem (txtVersion),
                            GridItem (lblCredits),
                            GridItem (txtCredits),
                            GridItem (lblDisclaimer),
                            GridItem (txtDisclaimer)
                        });
    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
