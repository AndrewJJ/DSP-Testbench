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

    Timer::callAfterDelay (10, [this] { addAboutText(); });

    setSize (800, 600);
}
void AboutComponent::paint(Graphics & g)
{
    g.fillAll (DspTestBenchLnF::ApplicationColours::componentBackground());
}
void AboutComponent::resized()
{
    txtEditor.setBounds(getLocalBounds().reduced(2,2));
}
void AboutComponent::addAboutText()
{
    txtEditor.clear();

    insertBreak();
    insertTitle ("Application description", Colours::white, false);
    insertText ("DSP Testbench is designed to help developers using the JUCE framework to analyse their DSP by providing ");
    insertText ("a test harness for code inheriting from juce::dsp::ProcessorBase. The harness provides signal sources, ");
    insertText ("routing, analysis and monitoring functions. See README.md for developer notes.", true);

    insertSubtitle ("Title bar controls");
    insertText ("Besides the usual window sizing buttons, the title bar hosts the following controls:", true);
    insertBullet(); insertText ("CPU meter");
    insertBullet(); insertText ("Snapshot");
    insertBullet(); insertText ("Performance benchmarks");
    insertBullet(); insertText ("Audio device settings");
    insertBullet(); insertText ("About (you already know what this does :) )", true);
    insertBreak();
    insertText ("Most of the application settings are automatically saved and restored each time you restart the app.", true);

    insertSubtitle ("Signal sources");
    insertText ("Two individual signal source modules generate synthesised signals, play back audio files, or pass through from an audio interface. ");
    insertText ("The synthesis tab provides periodic waveforms with sweepable frequencies, as well as impulse & step functions and noise generators. ");
    insertText ("Each source can be muted, inverted or gain trimmed, and the periodic waveforms can be synchronised between the two source modules.", true);
    insertBreak();
    insertText ("The triangle, square and saw oscillators are implemented with a PolyBLEP implementation to reduce aliasing, however it will still be ");
    insertText ("visible in the analyser at higher frequencies.", true);
    insertBreak();
    insertText ("Note that white and pink noise show up as a circle on the phase scope because we generate different samples on each channel. ");
    insertText ("The other oscillators generate the same samples on each channel.", true);

    insertSubtitle ("Processor control");
    insertText ("Each of the two processor modules are used to host and control your DSP code. Either or both signal sources can be routed ");
    insertText ("to each processor; and the output can be inverted, or muted.", true);
    insertBreak();
    insertText ("One use case is to verify that code optimisations do not alter the output. You can achieve this by hosting different versions ");
    insertText ("of your DSP code in either module, routing the same audio to both, and inverting the output of one processor to ensure perfect cancellation.", true);

    insertSubtitle ("Analysis");
    insertText ("The analyser provides the following:", true);
    insertBullet(); insertText ("FFT scope with logarithmic frequency scale (0 to -80dB amplitude scale)");
    insertBullet(); insertText ("Oscilloscope");
    insertBullet(); insertText ("Phase scope");
    insertBullet(); insertText ("Level meter with VU (narrow) and peak (wide) meters for each channel");
    insertBullet(); insertText ("Clip indicators (click to popup clip stats window with reset button)");
    insertBullet(); insertText ("Pause button to freeze the display (audio is not paused)");
    insertBullet(); insertText ("Expand button");
    insertBullet(); insertText ("Advanced settings for scopes", true);
    insertBreak();
    insertText ("The oscilloscope can be zoomed using the mouse wheel (hold shift to zoom amplitude instead of time) and you can pan by ");
    insertText ("clicking and dragging. Double click anywhere on the oscilloscope to reset scale", true);

    insertSubtitle ("Monitoring");
    insertText ("The monitoring section has a gain control and mute button to control the output level of the application. An optional output limiter ");
    insertText ("is also provided to prevent digital overs (this is applied after the processors so does not affect their behaviour).", true);

    insertSubtitle ("Snapshot");
    insertText ("The snapshot functionality allows you to pass 4096 samples through the processor then pause the analysis and audio so you can forensically ");
    insertText ("examine the resulting output. Normal operation can be resumed by toggling the snapshot button again. When a snapshot is triggered, the audio ");
    insertText ("device is stopped and restarted and all modules are reset so that the same 4096 samples will be generated and processed every single time. ");
    insertText ("The only exception to this is if the wave file player has its' right hand button disabled, in which case playback will be from the current position.", true);
    
    insertSubtitle ("Performance benchmarks");
    insertText ("The benchmark functionality starts your processor(s) on another thread and pumps audio through, gathering statistics on how much time has ");
    insertText ("been spent running your routines. A single block of audio is repeated from source A (using live audio input will not work).", true);
    
    insertTitle ("Credits & Attributions");
    insertText ("ASIO Interface Technology by Steinberg Media Technologies GmbH", true);
    insertBreak();
    insertText ("This software makes use of certain code libraries, those portions of code are copyright as per below:", true);
    insertBullet(); insertText ("JUCE 7"); insertCopyright ("Raw Material Software", 2022);
    insertBullet(); insertText ("Application code"); insertCopyright ("Oblique Audio", 2022);
    insertBullet(); insertText ("Fast maths approximations"); insertCopyright ("Paul Mineiro", 2011);
    insertBullet(); insertText ("rand31pmc white noise generator"); insertCopyright ("Robin Whittle", 2005);
    insertBullet(); insertText ("Pink noise filter - Paul Kellett");
    insertBullet(); insertText ("PolyBLEP/BLAMP - adapted from Tebjan Halm (vvvv.org)");
    insertBullet(); insertText ("MGA JS Limiter"); insertCopyright ("Michael Gruhn", 2008);
    insertBreak();

    insertTitle ("Software Disclaimer");
    insertText ("This software is provided \"as is\" and you use it at your own risk.", true);
    insertBreak();
    insertText ("The developers make no warranties as to performance, merchantability, fitness for a particular purpose, or any other warranties whether expressed or implied. ");
    insertText ("No oral or written communication from or information provided by the developers shall create a warranty.", true);
    insertBreak();
    insertText ("Under no circumstances shall the developers or contributors be liable for direct, indirect, special, incidental, or consequential damages resulting from the use, ");
    insertText ("misuse, or inability to use this software, even if the developers have been advised of the possibility of such damages.", true);
    insertBreak();
    insertText ("These exclusions and limitations may not apply in all jurisdictions. You may have additional rights and some of these limitations may not apply to you.", true);
    insertBreak (0.5f);

    txtEditor.setCaretPosition (0);
}
void AboutComponent::insertTitle (const String& title, const Colour textColour, const bool insertLineBefore)
{
    txtEditor.setColour (TextEditor::ColourIds::textColourId, textColour);
    if (insertLineBefore)
        insertBreak (1.2f);
    txtEditor.setFont (titleFont);
    txtEditor.insertTextAtCaret (title.toUpperCase() + "\n");
    insertBreak (0.3f);
}
void AboutComponent::insertSubtitle(const String & subtitle, const Colour textColour, const bool insertLineBefore)
{
    txtEditor.setColour (TextEditor::ColourIds::textColourId, textColour);
    if (insertLineBefore)
        insertBreak (0.5f);
    txtEditor.setFont (subtitleFont);
    txtEditor.insertTextAtCaret (subtitle.toUpperCase() + "\n");
    insertBreak (0.15f);
}
void AboutComponent::insertText (const String& text, const bool breakLine, const Colour textColour)
{
    txtEditor.setColour (TextEditor::ColourIds::textColourId, textColour);
    txtEditor.setFont (textFont);
    txtEditor.insertTextAtCaret (text);
    if (breakLine)
        txtEditor.insertTextAtCaret ("\n");
}
void AboutComponent::insertBreak (const float height)
{
    txtEditor.setFont (Font (GUI_SIZE_F(height)));  // NOLINT(clang-diagnostic-double-promotion)
    txtEditor.insertTextAtCaret ("\n");
}
void AboutComponent::insertBullet (const bool insertBreakBefore)
{
    if (insertBreakBefore)
        insertBreak();
    txtEditor.setFont (textFont);
    txtEditor.insertTextAtCaret (CharPointer_UTF8("\xE2\x80\xA2 "));
}
void AboutComponent::insertCopyright(const String& copyrightOwner, const int year)
{
    jassert (year > 1900);
    jassert (copyrightOwner.length() > 0);
    txtEditor.setFont (textFont);
    txtEditor.insertTextAtCaret (String(CharPointer_UTF8(" - copyright \xc2\xa9 ")) + String (year) + " " + copyrightOwner);
}