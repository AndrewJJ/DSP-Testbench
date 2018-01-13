/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "SourceComponent.h"

SynthesisTab::SynthesisTab ()
    :   sampleRate (0.0),
        maxBlockSize (0),
        numSweepSteps (0.0),
        sweepStepIndex (0),
        sweepStepDelta (1)
{
    addAndMakeVisible (cmbWaveform = new ComboBox ("Select Waveform"));
    cmbWaveform->addItem ("Sine", Waveform::sine);
    cmbWaveform->addItem ("Saw", Waveform::saw);
    cmbWaveform->addItem ("Square", Waveform::square);
    cmbWaveform->addItem ("Impulse", Waveform::impulse);
    cmbWaveform->addItem ("Step", Waveform::step);
    cmbWaveform->addItem ("White Noise", Waveform::whiteNoise);
    cmbWaveform->addItem ("Pink Noise", Waveform::pinkNoise);
    cmbWaveform->onChange = [this] { waveformUpdated(); };
    cmbWaveform->setSelectedId (Waveform::sine);

    addAndMakeVisible (sldFrequency = new Slider ("Frequency"));
    sldFrequency->setSliderStyle (Slider::ThreeValueHorizontal);
    sldFrequency->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldFrequency->setTooltip ("Sets the oscillator frequency in Hertz");
    sldFrequency->setRange (1.0, 24000.0, 1.0);
    sldFrequency->setMinAndMaxValues (1.0, 24000.0, dontSendNotification);
    sldFrequency->setValue (440.0, dontSendNotification);
    sldFrequency->setSkewFactor (0.5);

    addAndMakeVisible (sldSweepDuration = new Slider ("Sweep Duration"));
    sldSweepDuration->setTextBoxStyle (Slider::TextBoxRight, false, 80, 20);
    sldSweepDuration->setTooltip ("Sets the duration of the logarithmic frequency sweep in seconds");
    sldSweepDuration->setRange (0.5, 5.0, 0.1);
    sldSweepDuration->addListener (this);
    sldSweepDuration->setValue (1.0, sendNotificationAsync);
    
    addAndMakeVisible (cmbSweepMode = new ComboBox ("Select Sweep Mode"));
    cmbSweepMode->addItem ("Wrap", SweepMode::Wrap);
    cmbSweepMode->addItem ("Reverse", SweepMode::Reverse);
    cmbSweepMode->onChange = [this] { resetSweep(); };
    cmbSweepMode->setSelectedId (SweepMode::Wrap);

    addAndMakeVisible (btnSweepEnabled = new TextButton ("Sweep"));
    btnSweepEnabled->setTooltip ("Enable sweeping from start frequency to end frequency");
    btnSweepEnabled->setClickingTogglesState (true);
    btnSweepEnabled->setColour (TextButton::buttonOnColourId, Colours::green);
    btnSweepEnabled->onStateChange = [this] { updateSweepEnablement(); };

    addAndMakeVisible (btnSweepReset = new TextButton ("Reset"));
    btnSweepReset->setTooltip ("Reset/restart the frequency sweep");
    btnSweepReset->onClick = [this] { resetSweep(); };

    addAndMakeVisible (btnSynchWithOther = new TextButton ("Synch"));
    btnSynchWithOther->setTooltip ("Synch other source oscillator with this");
    btnSynchWithOther->onClick = [this] { performSynch(); };
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

    grid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray ({  GridItem (cmbWaveform).withArea ({ }, GridItem::Span (4)),
                            GridItem (sldFrequency).withArea ({ }, GridItem::Span (4)),
                            GridItem (sldSweepDuration).withArea ({ }, GridItem::Span (4)),
                            GridItem (cmbSweepMode),
                            GridItem (btnSweepEnabled),
                            GridItem (btnSweepReset),
                            GridItem (btnSynchWithOther)
                        });

    const auto marg = jmin (proportionOfWidth (0.05f), proportionOfHeight (0.05f));
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void SynthesisTab::performSynch ()
{
    ScopedLock sl (synthesiserCriticalSection);

    otherSource->getSynthesisTab()->syncAndResetOscillator( getSelectedWaveform(),
                                                            sldFrequency->getValue(),
                                                            sldFrequency->getMinValue(),
                                                            sldFrequency->getMaxValue(),
                                                            sldSweepDuration->getValue(),
                                                            getSelectedSweepMode(),
                                                            btnSweepEnabled->getToggleState()
                                                          );
    this->reset();
}
void SynthesisTab::setOtherSource (SourceComponent* otherSourceComponent)
{
    otherSource = otherSourceComponent;
}
void SynthesisTab::syncAndResetOscillator (const Waveform waveform, const double freq,
                                           const double sweepStart, const double sweepEnd,
                                           const double sweepDuration, SweepMode sweepMode, const bool sweepEnabled)
{
    cmbWaveform->setSelectedId (waveform, sendNotificationSync);
    sldFrequency->setMinAndMaxValues(sweepStart, sweepEnd, sendNotificationSync);
    sldFrequency->setValue(freq, sendNotificationSync);
    cmbSweepMode->setSelectedId (sweepMode, sendNotificationSync);
    btnSweepEnabled->setToggleState(sweepEnabled, sendNotificationSync);
    sldSweepDuration->setValue(sweepDuration, sendNotificationSync);
    this->reset();
}
void SynthesisTab::prepare (const dsp::ProcessSpec& spec)
{
    // Assumes sliders are constructed before prepare is ever called and that audio is shutdown before sliders are destroyed
    jassert (sldFrequency != nullptr && sldSweepDuration != nullptr);

    for (auto&& oscillator : oscillators)
    {
        oscillator.setFrequency (static_cast<float> (sldFrequency->getValue()));
        oscillator.prepare (spec);
    }
    
    sampleRate = spec.sampleRate;
    maxBlockSize = spec.maximumBlockSize;

    calculateNumSweepSteps();
    resetSweep();
}
void SynthesisTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (isSelectedWaveformOscillatorBased())
    {
        if (btnSweepEnabled->getToggleState())
        {
            oscillators[getSelectedWaveformIndex()].setFrequency (getSweepFrequency());
            
            if (getSelectedSweepMode() == SweepMode::Wrap)
            {
                if (sweepStepIndex >= numSweepSteps)
                    sweepStepIndex = 0;
                sweepStepIndex++;
            }
            else if (getSelectedSweepMode() == SweepMode::Reverse)
            {
                if (sweepStepIndex >= numSweepSteps)
                    sweepStepDelta = -1;
                else if (sweepStepIndex <= 0)
                    sweepStepDelta = 1;
                sweepStepIndex += sweepStepDelta;
            }
        }
        else
            oscillators[getSelectedWaveformIndex()].setFrequency (static_cast<float> (sldFrequency->getValue()));

        oscillators[getSelectedWaveformIndex()].process (context);
    }
}
void SynthesisTab::reset()
{
    ScopedLock sl (synthesiserCriticalSection);

    if (isSelectedWaveformOscillatorBased())
    {
        oscillators[getSelectedWaveformIndex()].reset();
        oscillators[getSelectedWaveformIndex()].setFrequency (static_cast<float> (sldFrequency->getValue()), true);
        resetSweep();
    }
}
void SynthesisTab::timerCallback ()
{
    jassert (btnSweepEnabled->getToggleState());
    sldFrequency->setValue (getSweepFrequency(),dontSendNotification);
}
void SynthesisTab::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldSweepDuration)
        calculateNumSweepSteps();
}

Waveform SynthesisTab::getSelectedWaveform() const
{
    return static_cast<Waveform> (cmbWaveform->getSelectedId());
}
int SynthesisTab::getSelectedWaveformIndex() const
{
    return cmbWaveform->getSelectedId()-1;
}
bool SynthesisTab::isSelectedWaveformOscillatorBased() const
{
    return (getSelectedWaveform() == Waveform::sine || getSelectedWaveform() == Waveform::saw || getSelectedWaveform() == Waveform::square);
}
SweepMode SynthesisTab::getSelectedSweepMode () const
{
    return static_cast<SweepMode> (cmbSweepMode->getSelectedId());
}
void SynthesisTab::waveformUpdated()
{
    // Modify controls according to waveform selection
    // TODO - hide or recolour disabled controls
    sldSweepDuration->setEnabled (isSelectedWaveformOscillatorBased() && btnSweepEnabled->getToggleState());
}
void SynthesisTab::updateSweepEnablement ()
{
    sldSweepDuration->setEnabled (btnSweepEnabled->getToggleState());
    
    if (btnSweepEnabled->getToggleState())
        startTimer (20.0);
    else
        stopTimer();
}
void SynthesisTab::resetSweep ()
{
    sweepStepIndex = 0;
    sweepStepDelta = 1;
}
double SynthesisTab::getSweepFrequency ()
{
    //f(x) = 10^(log(Span)/n*x) + fStart
    //where:
    //    x = the number of the sweep point
    //    n = total number of sweep points
    double duration = sldSweepDuration->getValue();
    double mn = sldFrequency->getMinValue();
    double mx = sldFrequency->getMaxValue();
    double span = mx - mn;

    return pow(10, log10(span)/numSweepSteps * sweepStepIndex) + mn;
}
void SynthesisTab::calculateNumSweepSteps ()
{
    numSweepSteps = static_cast<long> (sldSweepDuration->getValue() * sampleRate / static_cast<double> (maxBlockSize));
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

    addAndMakeVisible (btnInvert = new TextButton ("Invert Source button"));
    btnInvert->setButtonText (TRANS("Invert"));
    //btnInvert->onClick = [this] { };
    btnInvert->setClickingTogglesState (true);
    btnInvert->setColour(TextButton::buttonOnColourId, Colours::green);

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

    grid.templateColumns = { Track (3_fr), Track (8_fr), Track (2_fr), Track (2_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (sldGain).withMargin (GridItem::Margin (0.0f, 10.0f, 0.0f, 0.0f)),
                            GridItem (btnInvert),
                            GridItem (btnMute),
                            GridItem (tabbedComponent).withArea ({ }, GridItem::Span (4))
                        });
    
    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void SourceComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldGain)
    {
        gain.setGainDecibels (static_cast<float> (sldGain->getValue()));
    }
}
double SourceComponent::getGain () const
{
    return sldGain->getValue();
}
bool SourceComponent::isInverted () const
{
    return btnInvert->getToggleState();
}
bool SourceComponent::isMuted () const
{
    return btnMute->getToggleState();
}
SourceComponent::Mode SourceComponent::getMode() const
{
    return static_cast<Mode> (tabbedComponent->getCurrentTabIndex());
}
void SourceComponent::setOtherSource (SourceComponent* otherSourceComponent)
{
    otherSource = otherSourceComponent;
    synthesisTab->setOtherSource (otherSourceComponent);
}
SynthesisTab* SourceComponent::getSynthesisTab ()
{
    return synthesisTab;
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
        gain.setGainDecibels (static_cast<float> (sldGain->getValue()));
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

        if (isInverted())
            context.getOutputBlock().multiply(-1.0f);
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
