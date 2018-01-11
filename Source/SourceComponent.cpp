/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "SourceComponent.h"

SynthesisTab::SynthesisTab ()
{
    addAndMakeVisible (cmbWaveform = new ComboBox ("Select Waveform"));
    cmbWaveform->addItem ("Sine", Waveform::sine);
    cmbWaveform->addItem ("Saw", Waveform::saw);
    cmbWaveform->addItem ("Square", Waveform::square);
    cmbWaveform->addItem ("Impulse", Waveform::impulse);
    cmbWaveform->addItem ("Step", Waveform::step);
    cmbWaveform->addItem ("White Noise", Waveform::whiteNoise);
    cmbWaveform->addItem ("Pink Noise", Waveform::pinkNoise);
    cmbWaveform->onChange = [this] { updateWaveform(); };
    cmbWaveform->setSelectedId (Waveform::sine);

    addAndMakeVisible (sldFrequency = new Slider ("Frequency"));
    sldFrequency->setSliderStyle (Slider::ThreeValueHorizontal);
    sldFrequency->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldFrequency->setRange (1.0, 20000.0, 1.0);
    sldFrequency->setMinAndMaxValues (1.0, 20000.0, dontSendNotification);
    sldFrequency->setValue (440.0, dontSendNotification);
    sldFrequency->setSkewFactor (0.5);
    sldFrequency->addListener (this);

    addAndMakeVisible (sldSweepDuration = new Slider ("Sweep Duration"));
    sldSweepDuration->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldSweepDuration->addListener (this);

    addAndMakeVisible (btnSweepEnabled = new TextButton ("Sweep"));
    btnSweepEnabled->setTooltip ("Enable sweeping from start frequency to end frequency");
    btnSweepEnabled->setClickingTogglesState (true);
    //btnSweepEnabled->setToggleState (false, dontSendNotification);
    btnSweepEnabled->setColour (TextButton::buttonOnColourId, Colours::green);
    btnSweepEnabled->onStateChange = [this] { updateSweepEnablement(); };

    addAndMakeVisible (btnSweepReset = new TextButton ("Reset"));
    btnSweepReset->setTooltip ("Reset/restart the frequency sweep");
    btnSweepEnabled->onClick = [this] { resetSweep(); };

    // TODO - implement sweep modes: Hard Wrap, Hard Reverse, Soft Wrap, Soft Reverse
}
SynthesisTab::~SynthesisTab ()
{
    cmbWaveform = nullptr;
    sldFrequency = nullptr;
    sldSweepDuration = nullptr;
    btnSweepEnabled = nullptr;
}
void SynthesisTab::paint (Graphics& g)
{ }
void SynthesisTab::resized ()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr),
                            Track (1_fr),
                            Track (1_fr),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray ({  GridItem (cmbWaveform).withArea ({ }, GridItem::Span (2)),
                            GridItem (sldFrequency).withArea ({ }, GridItem::Span (2)),
                            GridItem (sldSweepDuration).withArea ({ }, GridItem::Span (2)),
                            GridItem (btnSweepEnabled),
                            GridItem (btnSweepReset)
                        });

    const auto marg = jmin (proportionOfWidth (0.05f), proportionOfHeight (0.05f));
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void SynthesisTab::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldFrequency)
    {
    }
}
void SynthesisTab::updateWaveform()
{
    // Modify controls according to waveform selection
    // TODO - hide or recolour disabled controls
    const auto id = cmbWaveform->getSelectedId();
    if (id == Waveform::sine || id == Waveform::saw || id == Waveform::square)
    {
        if (btnSweepEnabled->getToggleState())
            sldSweepDuration->setEnabled(true);
    }
    else
    {
        sldSweepDuration->setEnabled (false);
    }

    // TODO - notify audio engine of change

    //if (id == Waveforms::sine)
    //else if (id == Waveforms::saw)
    //else if (id == Waveforms::square)
    //else if (id == Waveforms::impulse)
    //else if (id == Waveforms::step)
    //else if (id == Waveforms::whiteNoise)
    //else if (id == Waveforms::pinkNoise)

}
void SynthesisTab::updateSweepEnablement ()
{
    // TODO - rename start frequency to just frequency if sweep is disabled
    // TODO - notify audio engine of change
}
void SynthesisTab::resetSweep ()
{
    // TODO - notify audio engine of change
}

SampleTab::SampleTab ()
{
    addAndMakeVisible (cmbSample = new ComboBox ("Select Sample"));
    cmbSample->addItem ("None", 1);
    cmbSample->setSelectedId (1, dontSendNotification);
    cmbSample->onChange = [this] { };

    addAndMakeVisible (btnLoopEnabled = new TextButton ("Loop Enabled"));
    btnLoopEnabled->setClickingTogglesState (true);
    //btnLoopEnabled->setToggleState (true, dontSendNotification);
    btnLoopEnabled->setColour (TextButton::buttonOnColourId, Colours::green);
    btnLoopEnabled->onClick = [this] { };

    // TODO - add delay control to prevent machine gunning of sample?
}
SampleTab::~SampleTab ()
{
    cmbSample = nullptr;
    btnLoopEnabled = nullptr;
}
void SampleTab::paint (Graphics& g)
{
}
void SampleTab::resized ()
{
    cmbSample->setBoundsRelative (0.1f, 0.2f, 0.8f, 0.2f);
    btnLoopEnabled->setBoundsRelative (0.1f, 0.5f, 0.8f, 0.2f);
}

WaveTab::WaveTab ()
{
}
WaveTab::~WaveTab ()
{
}
void WaveTab::paint (Graphics& g)
{
    g.setFont (25.0f);
    g.setColour (Colours::white);
    g.drawFittedText ("Wave playing not yet implemented", 0, 0, getWidth(), getHeight(), Justification::Flags::centred, 2);
}
void WaveTab::resized ()
{
}

AudioTab::AudioTab ()
{
}
AudioTab::~AudioTab ()
{
}
void AudioTab::paint (Graphics& g)
{
    g.setFont (25.0f);
    g.setColour (Colours::white);
    g.drawFittedText ("Audio input not yet implemented", 0, 0, getWidth(), getHeight(), Justification::Flags::centred, 2);
}
void AudioTab::resized ()
{
}

//==============================================================================

SourceComponent::SourceComponent (String sourceId)
{
    addAndMakeVisible (lblTitle = new Label ("Source label", TRANS("Source") + " " + String (sourceId)));
    lblTitle->setFont (Font (15.00f, Font::bold));
    lblTitle->setJustificationType (Justification::topLeft);
    lblTitle->setEditable (false, false, false);
    lblTitle->setColour (TextEditor::textColourId, Colours::black);
    lblTitle->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sldGain = new Slider ("Input gain slider"));
    sldGain->setTooltip (TRANS("Adjusts the gain of this source"));
    sldGain->setRange (-100, 50, 0.1);
    sldGain->setSliderStyle (Slider::LinearHorizontal);
    sldGain->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldGain->addListener (this);

    addAndMakeVisible (btnMute = new TextButton ("Mute Source button"));
    btnMute->setButtonText (TRANS("Mute"));
    btnMute->onClick = [this] { toggleMute(); };
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (30);
    tabbedComponent->addTab (TRANS("Synthesis"), Colours::darkgrey, new SynthesisTab(), true);
    tabbedComponent->addTab (TRANS("Sample"), Colours::darkgrey, new SampleTab(), true);
    tabbedComponent->addTab (TRANS("Wave File"), Colours::darkgrey, new WaveTab(), true);
    tabbedComponent->addTab (TRANS("Audio In"), Colours::darkgrey, new AudioTab(), true);
    tabbedComponent->setCurrentTabIndex (0);

    //setSize (600, 400);
}

SourceComponent::~SourceComponent()
{
    lblTitle = nullptr;
    sldGain = nullptr;
    btnMute = nullptr;
    tabbedComponent = nullptr;
}

void SourceComponent::paint (Graphics& g)
{
    g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), 10.000f);
}

void SourceComponent::resized()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr),
                            Track (4_fr)
                        };

    grid.templateColumns = { Track (2_fr), Track (6_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (sldGain),
                            GridItem (btnMute).withMargin (GridItem::Margin (0.0f, 0.0f, 0.0f, 10.0f)),
                            GridItem (tabbedComponent).withArea ({ }, GridItem::Span (3))
                        });
    
    const auto marg = 10;
    // .withTrimmedTop(proportionOfHeight(0.1f))
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}

void SourceComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldGain)
    {
    }
}

void SourceComponent::toggleMute()
{
    if (btnMute->getToggleState())
    {
        // TODO - notify audio engine that this source component should be muted
    }
    else
    {
        // TODO - notify audio engine that this source component should be unmuted
    }
}
