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
    sldFrequency->setRange (1.0, 24000.0, 1.0);
    sldFrequency->setMinAndMaxValues (1.0, 24000.0, dontSendNotification);
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
void SynthesisTab::prepare (const dsp::ProcessSpec& spec)
{
    for (auto&& oscillator : oscillators)
    {
        // TODO - get initial value from Gui? but only if Gui is already loaded
        oscillator.setFrequency (sldFrequency->getValue());
        oscillator.prepare (spec);
    }
}
void SynthesisTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    auto idx = cmbWaveform->getSelectedId();
    switch (static_cast<Waveform> (idx)) {
    case sine:
    case saw:
    case Waveform::square:
        oscillators[idx-1].setFrequency (sldFrequency->getValue());
        oscillators[idx-1].process (context);
        break;
    default: ;  // Do nothing
    }
}
void SynthesisTab::reset()
{
    auto idx = cmbWaveform->getSelectedId();
    switch (static_cast<Waveform> (idx)) {
    case sine:
    case saw:
    case Waveform::square:
        oscillators[idx].reset();
        break;
    default: ;  // Do nothing
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
    cmbSample->onChange = [this] { selectedSampleChanged(); };

    addAndMakeVisible (btnLoopEnabled = new TextButton ("Loop Enabled"));
    btnLoopEnabled->setClickingTogglesState (true);
    //btnLoopEnabled->setToggleState (true, dontSendNotification);
    btnLoopEnabled->setColour (TextButton::buttonOnColourId, Colours::green);
    btnLoopEnabled->onClick = [this] { loopEnablementToggled(); };

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
void SampleTab::selectedSampleChanged ()
{
}
void SampleTab::loopEnablementToggled ()
{
}
void SampleTab::prepare (const dsp::ProcessSpec& spec)
{
    // TODO
}
void SampleTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    // TODO
}
void SampleTab::reset()
{
    // TODO
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
void WaveTab::prepare (const dsp::ProcessSpec& spec)
{
    // TODO
}
void WaveTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    // TODO
}
void WaveTab::reset()
{
    // TODO
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
void AudioTab::prepare (const dsp::ProcessSpec& spec)
{
    // TODO
}
void AudioTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    // TODO
}
void AudioTab::reset()
{
    // TODO
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
    //btnMute->onClick = [this] { toggleMute(); };
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (30);
    tabbedComponent->addTab (TRANS("Synthesis"), Colours::darkgrey, synthesisTab = new SynthesisTab(), false, Mode::Synthesis);
    tabbedComponent->addTab (TRANS("Sample"), Colours::darkgrey, sampleTab = new SampleTab(), false, Mode::Sample);
    tabbedComponent->addTab (TRANS("Wave File"), Colours::darkgrey, waveTab = new WaveTab(), false, Mode:: WaveFile);
    tabbedComponent->addTab (TRANS("Audio In"), Colours::darkgrey, audioTab = new AudioTab(), false, Mode::AudioIn);
    tabbedComponent->setCurrentTabIndex (0);

    //setSize (600, 400);

    gain.setRampDurationSeconds (0.01);
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

    grid.templateColumns = { Track (3_fr), Track (10_fr), Track (2_fr) };

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
        gain.setGainDecibels(sldGain->getValue());
    }
}
double SourceComponent::getGain () const
{
    return sldGain->getValue();
}
bool SourceComponent::isMuted () const
{
    return btnMute->getToggleState();
}
SourceComponent::Mode SourceComponent::getMode() const
{
    return static_cast<Mode> (tabbedComponent->getCurrentTabIndex());
}
void SourceComponent::prepare (const dsp::ProcessSpec& spec)
{
    synthesisTab->prepare (spec);
    sampleTab->prepare (spec);
    waveTab->prepare (spec);
    audioTab->prepare (spec);
    gain.prepare (spec);
    jassert (sldGain != nullptr); // If this is null then gain won't initialise and you won't hear a sound until the slider is moved
    if (sldGain != nullptr)
        gain.setGainDecibels(sldGain->getValue());
}
void SourceComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (!isMuted())
    {
        // Process currently selected source
        const auto idx = tabbedComponent->getCurrentTabIndex();
        if (idx == Mode::Synthesis)
            synthesisTab->process (context);
        else if (idx == Mode::Sample)
            sampleTab->process (context);
        else if (idx == Mode::WaveFile)
            waveTab->process (context);
        else if (idx == Mode::AudioIn)
            audioTab->process (context);
        
        // Apply gain
        gain.process (context);
    }
    else
        context.getOutputBlock().clear();
}
void SourceComponent::reset ()
{
    synthesisTab->reset();
    sampleTab->reset();
    waveTab->reset();
    audioTab->reset();
    gain.reset();
}
