/*
  ==============================================================================

    AboutComponent.cpp
    Created: 16 Mar 2019 4:17:47pm
    Author:  Andrew

  ==============================================================================
*/

#include "AboutComponent.h"

AboutComponent::AboutComponent()
{
    addAndMakeVisible (txtEditor);
    txtEditor.setMultiLine (true);
    txtEditor.setReadOnly (true);
    txtEditor.setCaretVisible (false);
    txtEditor.setPopupMenuEnabled (false);
    txtEditor.setColour (TextEditor::ColourIds::outlineColourId, Colours::transparentBlack);
    txtEditor.setColour (TextEditor::ColourIds::backgroundColourId, Colours::black);

    insertTitle ("Help notes", false);
    insertText ("DSP Testbench is designed to help users of the JUCE framework to test their DSP code. ");
    insertText ("It provides a test harness for code inheriting from juce::dsp::ProcessorBase (see README.md for more details). ");
    insertText ("The harness provides signal sources, routing, analysis and monitoring functions.", true);

    insertSubtitle ("Signal sources");
    insertText ("Blah, blah, blah", true);

    insertSubtitle ("Processor control");
    insertText ("Blah, blah, blah", true);

    insertSubtitle ("Analysis");
    insertText ("Blah, blah, blah", true);
    insertText ("White & pink noise show up as a circle on the phase scope because we generate different samples on each channel whereas the other oscillators generate the same samples on each channel", true);

    insertSubtitle ("Monitoring");
    insertText ("Blah, blah, blah", true);

    insertTitle ("Credits & Attributions");
    insertText ("ASIO Interface Technology by Steinberg Media Technologies GmbH", true);
    insertBreak();
    insertText ("This software makes use of certain code libraries, those portions of code are copyright as per below:", true);
    insertBreak (0.1f);
    insertBullet (false); insertText ("JUCE 5"); insertCopyright ("Raw Material Software", 2019);
    insertBullet(); insertText ("Application code"); insertCopyright ("Oblique Audio", 2019);
    insertBullet(); insertText ("Fast maths approximations"); insertCopyright ("Paul Mineiro", 2011);
    insertBullet(); insertText ("rand31pmc white noise generator"); insertCopyright ("Robin Whittle", 2005);
    insertBullet(); insertText ("Pink noise filter - Paul Kellett");
    insertBullet(); insertText ("PolyBLEP/BLAMP - adapted from Tebjan Halm (vvvv.org)");
    insertBullet(); insertText ("MGA JS Limiter"); insertCopyright ("Michael Gruhn", 2008);
    insertBreak();

    insertTitle ("Software Disclaimer");
    insertText ("This software is provided \'\'as is\'\' and you use it at your own risk.", true);
    insertBreak();
    insertText ("The developers make no warranties as to performance, merchantability, fitness for a particular purpose, or any other warranties whether expressed or implied. ");
    insertText ("No oral or written communication from or information provided by the developers shall create a warranty.", true);
    insertBreak();
    insertText ("Under no circumstances shall the developers or contributors be liable for direct, indirect, special, incidental, or consequential damages resulting from the use, ");
    insertText ("misuse, or inability to use this software, even if the developers have been advised of the possibility of such damages.", true);
    insertBreak();
    insertText ("These exclusions and limitations may not apply in all jurisdictions. You may have additional rights and some of these limitations may not apply to you.", true);
    insertBreak (0.5f);

    setSize (800, 600);
}
void AboutComponent::paint(Graphics & g)
{
    g.fillAll (Colour (0xff323e44));
}
void AboutComponent::resized()
{
    txtEditor.setBounds(getLocalBounds().reduced(2,2));
}
void AboutComponent::insertTitle (const String& title, const bool insertLineBefore)
{
    txtEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
    if (insertLineBefore)
        insertBreak (1.2f);
    txtEditor.setFont (titleFont);
    txtEditor.insertTextAtCaret (title.toUpperCase() + "\n");
    insertBreak (0.3f);
}
void AboutComponent::insertSubtitle(const String & subtitle, const bool insertLineBefore)
{
    txtEditor.setColour (TextEditor::ColourIds::textColourId, Colours::white);
    if (insertLineBefore)
        insertBreak (0.5f);
    txtEditor.setFont (subtitleFont);
    txtEditor.insertTextAtCaret (subtitle.toUpperCase() + "\n");
    insertBreak (0.15f);
}
void AboutComponent::insertText (const String& text, const bool breakLine)
{
    txtEditor.setColour (TextEditor::ColourIds::textColourId, Colour (0xffd0d0d0));
    txtEditor.setFont (textFont);
    txtEditor.insertTextAtCaret (text);
    if (breakLine)
        txtEditor.insertTextAtCaret ("\n");
}
void AboutComponent::insertBreak (const float height)
{
    txtEditor.setFont (Font (GUI_SIZE_F(height)));
    txtEditor.insertTextAtCaret ("\n");
}
void AboutComponent::insertBullet (const bool insertBreakBefore)
{
    txtEditor.setFont (textFont);
    if (insertBreakBefore)
        txtEditor.insertTextAtCaret ("\n");
    txtEditor.insertTextAtCaret (CharPointer_UTF8("\xE2\x80\xA2 "));
}
void AboutComponent::insertCopyright(const String& copyrightOwner, const int year)
{
    jassert (year > 1900);
    jassert (copyrightOwner.length() > 0);
    txtEditor.setFont (textFont);
    txtEditor.insertTextAtCaret (String(CharPointer_UTF8(" - copyright \xc2\xa9 ")) + String (year) + " " + copyrightOwner);
}