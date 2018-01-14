/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "SourceComponent.h"
#include "Main.h"

SynthesisTab::SynthesisTab ()
{
    addAndMakeVisible (cmbWaveform = new ComboBox ("Select Waveform"));
    cmbWaveform->setTooltip ("Select a waveform");
    cmbWaveform->addItem ("Sine", Waveform::sine);
    cmbWaveform->addItem ("Triangle", Waveform::triangle);
    cmbWaveform->addItem ("Square", Waveform::square);
    cmbWaveform->addItem ("Saw", Waveform::saw);
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
    cmbSweepMode->setTooltip ("Select whether the frequency sweep wraps or reverses when it reaches its maximum value");
    cmbSweepMode->addItem ("Wrap", SweepMode::Wrap);
    cmbSweepMode->addItem ("Reverse", SweepMode::Reverse);
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
            oscillators[getSelectedWaveformIndex()].setFrequency (static_cast<float> (getSweepFrequency()));
            
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
    else
    {
        // TODO - implement other waveforms
        // see here for white and pink noise: https://github.com/tebjan/VVVV.Audio/blob/master/Source/VVVV.Audio.Signals/Sources/OscSignal.cs

        // TODO - delete
        context.getOutputBlock().clear();
    }
}
void SynthesisTab::reset()
{
    ScopedLock sl (synthesiserCriticalSection);

    for (auto&& oscillator : oscillators)
    {
        oscillator.reset();
        oscillator.setFrequency (static_cast<float> (sldFrequency->getValue()), true);
    }

    resetSweep();
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
    return (    getSelectedWaveform() == Waveform::sine 
             || getSelectedWaveform() == Waveform::saw 
             || getSelectedWaveform() == Waveform::square 
             || getSelectedWaveform() == Waveform::triangle
           );
}
SweepMode SynthesisTab::getSelectedSweepMode () const
{
    return static_cast<SweepMode> (cmbSweepMode->getSelectedId());
}
void SynthesisTab::waveformUpdated()
{
    // Only enable sweep controls for oscillator based waveforms
    sldSweepDuration->setEnabled (isSelectedWaveformOscillatorBased());
    btnSweepEnabled->setEnabled (isSelectedWaveformOscillatorBased());
    btnSweepReset->setEnabled (isSelectedWaveformOscillatorBased());
}
void SynthesisTab::updateSweepEnablement ()
{
    sldSweepDuration->setEnabled (btnSweepEnabled->getToggleState());
    
    if (btnSweepEnabled->getToggleState())
        startTimerHz (50);
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
    const auto mn = sldFrequency->getMinValue();
    const auto mx = sldFrequency->getMaxValue();
    const auto span = mx - mn;

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
void SampleTab::selectedSampleChanged()
{
    // TODO - SampleTab::selectedSampleChanged()
}
void SampleTab::loopEnablementToggled()
{
    // TODO - SampleTab::loopEnablementToggled()
}
void SampleTab::prepare (const dsp::ProcessSpec& spec)
{
    // TODO - SampleTab::prepare()
}
void SampleTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    // TODO - SampleTab::process()
    context.getOutputBlock().clear();
}
void SampleTab::reset()
{
    // TODO - SampleTab::reset()
}

WaveTab::AudioThumbnailComponent::AudioThumbnailComponent()
    : thumbnailCache (5),
      thumbnail (128, DSPTestbenchApplication::getApp().getFormatManager(), thumbnailCache)
{
    thumbnail.addChangeListener (this);
}
WaveTab::AudioThumbnailComponent::~AudioThumbnailComponent ()
{
    thumbnail.removeChangeListener (this);
}
void WaveTab::AudioThumbnailComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xff495358));

    g.setColour (Colours::white);

    if (thumbnail.getTotalLength() > 0.0)
    {
        thumbnail.drawChannels (g, getLocalBounds().reduced (2),
                                0.0, thumbnail.getTotalLength(), 1.0f);

        g.setColour (Colours::black);
        g.fillRect (static_cast<float> (currentPosition * getWidth()), 0.0f,
                    1.0f, static_cast<float> (getHeight()));
    }
    else
    {
        g.drawFittedText ("No audio file loaded.\nDrop a file here or click the \"Load\" button.", getLocalBounds(),
                            Justification::centred, 2);
    }
}
bool WaveTab::AudioThumbnailComponent::isInterestedInFileDrag (const StringArray&)
{
    return true;
}
void WaveTab::AudioThumbnailComponent::filesDropped (const StringArray& files, int, int)
{
    loadFile (File (files[0]), true);
}
void WaveTab::AudioThumbnailComponent::setCurrentFile (const File& f)
{
    if (currentFile == f)
        return;

    loadFile (f);
}
File WaveTab::AudioThumbnailComponent::getCurrentFile() const
{
    return currentFile;
}
void WaveTab::AudioThumbnailComponent::setTransportSource (AudioTransportSource* newSource)
{
    transportSource = newSource;

    struct ResetCallback  : public CallbackMessage
    {
        ResetCallback (AudioThumbnailComponent& o) : owner (o) {}
        void messageCallback() override    { owner.reset(); }

        AudioThumbnailComponent& owner;
    };

    (new ResetCallback (*this))->post();
}
void WaveTab::AudioThumbnailComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == reinterpret_cast<ChangeBroadcaster*> (&thumbnail) || source == this)
        repaint();
}
void WaveTab::AudioThumbnailComponent::timerCallback ()
{
    if (transportSource != nullptr)
    {
        currentPosition = transportSource->getCurrentPosition() / thumbnail.getTotalLength();
        repaint();
    }
}
void WaveTab::AudioThumbnailComponent::reset ()
{
    currentPosition = 0.0;
    repaint();

    if (transportSource == nullptr)
        stopTimer();
    else
        startTimerHz (25);
}
void WaveTab::AudioThumbnailComponent::loadFile (const File& f, bool notify)
{
    if (currentFile == f || ! f.existsAsFile())
        return;

    currentFile = f;
    thumbnail.setSource (new FileInputSource (f));

    if (notify)
        sendChangeMessage();
}
void WaveTab::AudioThumbnailComponent::mouseDrag (const MouseEvent& e)
{
    if (transportSource != nullptr)
    {
        AudioDeviceManager* adm = &DSPTestbenchApplication::getApp().getDeviceManager();
        const ScopedLock sl (adm->getAudioCallbackLock());

        transportSource->setPosition ((jmax (static_cast<double> (e.x), 0.0) / getWidth())
                                        * thumbnail.getTotalLength());
    }
}

WaveTab::WaveTab()
    :   sampleRate(0.0),
        maxBlockSize(0)
{
    addAndMakeVisible(audioThumbnailComponent = new AudioThumbnailComponent());
    audioThumbnailComponent->addChangeListener (this);

    addAndMakeVisible (btnLoad = new TextButton ("Load"));
    btnLoad->setTooltip ("Load wave file");
    btnLoad->onClick = [this] { chooseFile(); };

    addAndMakeVisible (btnPlay = new TextButton ("Play"));
    btnPlay->setTooltip ("Play/pause (playing will loop once the end of the file is reached)");
    btnPlay->setClickingTogglesState (true);
    btnPlay->setColour (TextButton::buttonOnColourId, Colours::green);
    btnPlay->onClick = [this] {
        if (btnPlay->getToggleState())
            play();
        else
            pause();
    };

    addAndMakeVisible (btnStop = new TextButton ("Stop"));
    btnStop->onClick = [this] { stop(); };

    addAndMakeVisible (btnLoop = new TextButton ("Loop"));
    btnLoop->setClickingTogglesState (true);
    btnLoop->setToggleState(true, dontSendNotification);
    btnLoop->setColour (TextButton::buttonOnColourId, Colours::green);
    btnLoop->onStateChange = [this] { 
        readerSource->setLooping (btnLoop->getToggleState());
    };

}
WaveTab::~WaveTab ()
{
}
void WaveTab::paint (Graphics& g)
{
}
void WaveTab::resized ()
{
    Grid grid;
    grid.rowGap = 5_px;
    grid.columnGap = 5_px;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (2_fr),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr) };

    grid.autoColumns = Track (1_fr);
    grid.autoRows = Track (1_fr);

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (audioThumbnailComponent).withArea ({ }, GridItem::Span (4)),
                            GridItem (btnLoad),
                            GridItem (btnPlay),
                            GridItem (btnStop),
                            GridItem (btnLoop)
                        });
    
    const auto marg = 10;
    grid.performLayout (getLocalBounds().reduced (marg, marg));
}
void WaveTab::prepare (const dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxBlockSize = spec.maximumBlockSize;
    fileReadBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    init();
}
void WaveTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (transportSource != nullptr)
    {
        AudioSourceChannelInfo info (fileReadBuffer);

        // Always read next audio block so pause & stop methods doesn't have a one second time out
        transportSource->getNextAudioBlock (info);

        // But only output the block if currently playing
        if (transportSource->isPlaying())
            context.getOutputBlock().copy(fileReadBuffer);
        else
            context.getOutputBlock().clear();
    }
    else
        context.getOutputBlock().clear();
}
void WaveTab::reset()
{
    // TODO - check this code (is it actually needed?)
    stop();
    transportSource = nullptr;
    readerSource = nullptr;
    reader = nullptr;
    fileReadBuffer.clear();
}
void WaveTab::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == audioThumbnailComponent)
    {
        if (btnPlay->getToggleState())
            stop();
        loadFile (audioThumbnailComponent->getCurrentFile());
    }
    if (transportSource->hasStreamFinished())
        stop();
}
bool WaveTab::loadFile (const File& fileToPlay)
{
    stop();

    audioThumbnailComponent->setTransportSource (nullptr);
    transportSource.reset();
    readerSource.reset();

    reader = DSPTestbenchApplication::getApp().getFormatManager().createReaderFor (fileToPlay);
    if (reader != nullptr)
    {
        readerSource = new AudioFormatReaderSource (reader, false);
        init();
        return true;
    }
    return false;
}
void WaveTab::chooseFile()
{
    stop();

    FileChooser fc ("Select an audio file...", File(), "*.wav;*.mp3;*.aif;");

    if (fc.browseForFileToOpen())
    {
        auto f = fc.getResult();

        if (!loadFile (f))
            NativeMessageBox::showOkCancelBox (AlertWindow::WarningIcon, "Error loading file", "Unable to load audio file", nullptr, nullptr);
        else
            audioThumbnailComponent->setCurrentFile (f);
    }
}
void WaveTab::init()
{
    if (transportSource == nullptr)
    {
        transportSource = new AudioTransportSource();
        transportSource->prepareToPlay(maxBlockSize, sampleRate);
        transportSource->addChangeListener (this);

        if (readerSource != nullptr)
        {
            if (auto* device = DSPTestbenchApplication::getApp().getDeviceManager().getCurrentAudioDevice())
            {
                transportSource->setSource (readerSource, roundToInt (device->getCurrentSampleRate()), &DSPTestbenchApplication::getApp(), reader->sampleRate);
                // tell the main window about this so that it can do the seeking behaviour...
                audioThumbnailComponent->setTransportSource (transportSource);
            }
        }
    }
}
void WaveTab::play()
{
    if (readerSource == nullptr)
        return;

    if (transportSource->getCurrentPosition() >= transportSource->getLengthInSeconds()
        || transportSource->getCurrentPosition() < 0)
    {
        transportSource->setPosition (0);
    }
    transportSource->start();
}
void WaveTab::pause ()
{
    if (transportSource != nullptr)
        transportSource->stop();
}
void WaveTab::stop ()
{
    btnPlay->setToggleState (false, dontSendNotification);
    
    if (transportSource != nullptr)
    {
        transportSource->stop();
        transportSource->setPosition (0);
    }
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
    // TODO - AudioTab::prepare()
}
void AudioTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    // TODO - AudioTab::process()
    context.getOutputBlock().clear();
}
void AudioTab::reset()
{
    // TODO - AudioTab::reset()
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
