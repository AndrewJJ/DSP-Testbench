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
    // Assume sample rate of 48K - this will be corrected when prepare() is called
    const auto nyquist = 24000.0;

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
    sldFrequency->setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldFrequency->setTooltip ("Sets the oscillator frequency in Hertz");
    sldFrequency->setRange (10.0, nyquist, 1.0);
    sldFrequency->setMinAndMaxValues (10.0, nyquist, dontSendNotification);
    sldFrequency->addListener (this);
    sldFrequency->setValue (440.0, sendNotificationSync);
    sldFrequency->setSkewFactor (0.5);

    addAndMakeVisible (sldSweepDuration = new Slider ("Sweep Duration"));
    sldSweepDuration->setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldSweepDuration->setTooltip ("Sets the duration of the logarithmic frequency sweep in seconds");
    sldSweepDuration->setRange (0.5, 5.0, 0.1);
    sldSweepDuration->addListener (this);
    sldSweepDuration->setValue (1.0, sendNotificationSync);
    
    addAndMakeVisible (cmbSweepMode = new ComboBox ("Select Sweep Mode"));
    cmbSweepMode->setTooltip ("Select whether the frequency sweep wraps or reverses when it reaches its maximum value");
    cmbSweepMode->addItem ("Reverse", SweepMode::Reverse);
    cmbSweepMode->addItem ("Wrap", SweepMode::Wrap);
    cmbSweepMode->onChange = [this] { currentSweepMode = static_cast<SweepMode> (cmbSweepMode->getSelectedId()); };
    cmbSweepMode->setSelectedId (SweepMode::Reverse);

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
void SynthesisTab::paint (Graphics&)
{ }
void SynthesisTab::resized ()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX),
                            Track (GUI_BASE_SIZE_PX),
                            Track (GUI_BASE_SIZE_PX),
                            Track (GUI_BASE_SIZE_PX)
                        };

    grid.templateColumns = { GUI_SIZE_PX(3.5), GUI_SIZE_PX(3.5), GUI_SIZE_PX(3.5), GUI_SIZE_PX(3.5) };

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray ({  GridItem (cmbWaveform).withArea ({ }, GridItem::Span (4)),
                            GridItem (sldFrequency).withArea ({ }, GridItem::Span (4)),
                            GridItem (sldSweepDuration).withArea ({ }, GridItem::Span (4)),
                            GridItem (cmbSweepMode),
                            GridItem (btnSweepEnabled),
                            GridItem (btnSweepReset),
                            GridItem (btnSynchWithOther)
                        });

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
float SynthesisTab::getMinimumWidth()
{
    // This is an exact calculation of the width in the grid layout in resized()
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemWidth = GUI_SIZE_F(3.5 * 4);
    const auto totalItemGaps = GUI_GAP_F(3);
    return innerMargin + totalItemWidth + totalItemGaps;
}
float SynthesisTab::getMinimumHeight()
{
    // This is an exact calculation of the height in the grid layout in resized()
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_SIZE_F(4);
    const auto totalItemGaps = GUI_GAP_F(3);
    return innerMargin + totalItemHeight + totalItemGaps;
}
void SynthesisTab::performSynch ()
{
    // Required to ensure synching with other source
    const ScopedLock sl (synthesiserCriticalSection);

    otherSource->getSynthesisTab()->syncAndResetOscillator( currentWaveform,
                                                            currentFrequency,
                                                            sweepStartFrequency,
                                                            sweepEndFrequency,
                                                            sweepDuration,
                                                            currentSweepMode,
                                                            isSweepEnabled
                                                          );
    this->reset();
}
void SynthesisTab::setOtherSource (SourceComponent* otherSourceComponent)
{
    otherSource = otherSourceComponent;
}
void SynthesisTab::syncAndResetOscillator (const Waveform waveform, const double freq,
                                           const double sweepStart, const double sweepEnd,
                                           const double newSweepDuration, SweepMode sweepMode, const bool sweepEnabled)
{
    cmbWaveform->setSelectedId (waveform, sendNotificationSync);
    sldFrequency->setMinAndMaxValues(sweepStart, sweepEnd, sendNotificationSync);
    sldFrequency->setValue(freq, sendNotificationSync);
    cmbSweepMode->setSelectedId (sweepMode, sendNotificationSync);
    btnSweepEnabled->setToggleState(sweepEnabled, sendNotificationSync);
    sldSweepDuration->setValue(newSweepDuration, sendNotificationSync);
    this->reset();
}
void SynthesisTab::prepare (const dsp::ProcessSpec& spec)
{
    // Assumes sliders are constructed before prepare is ever called and that audio is shutdown before sliders are destroyed
    jassert (sldFrequency != nullptr && sldSweepDuration != nullptr);

    for (auto&& oscillator : oscillators)
    {
        oscillator.setFrequency (static_cast<float> (currentFrequency));
        oscillator.prepare (spec);
    }

    sampleRate = spec.sampleRate;
    maxBlockSize = spec.maximumBlockSize;
    
    const auto nyquist = round (sampleRate / 2.0);
    sldFrequency->setRange (10.0, nyquist, 1.0);

    calculateNumSweepSteps();
    resetSweep();
}
void SynthesisTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (isSelectedWaveformOscillatorBased())
    {
        // Set oscillator frequency
        if (isSweepEnabled)
        {
            for (auto&& oscillator : oscillators)
                oscillator.setFrequency (static_cast<float> (getSweepFrequency()));
            
            if (currentSweepMode == SweepMode::Wrap)
            {
                if (sweepStepIndex >= numSweepSteps)
                    sweepStepIndex = 0;
                sweepStepIndex++;
            }
            else if (currentSweepMode == SweepMode::Reverse)
            {
                if (sweepStepIndex >= numSweepSteps)
                    sweepStepDelta = -1;
                else if (sweepStepIndex <= 0)
                    sweepStepDelta = 1;
                sweepStepIndex += sweepStepDelta;
            }
        }
        else
        {
            for (auto&& oscillator : oscillators)
                oscillator.setFrequency (static_cast<float> (currentFrequency));
        }

        // Process current oscillator (note we adjust 1-based index to 0-based index)
        oscillators[currentWaveform - 1].process (context);
    }
    else if (currentWaveform == Waveform::whiteNoise)
    {
        whiteNoise.process (context);
    }
    else if (currentWaveform == Waveform::pinkNoise)
    {
        pinkNoise.process (context);
    }
    else
    {
        // TODO - implement impulse and step functions with configurable pre-delay

        // TODO - delete once impulse and step functions have been implemented
        context.getOutputBlock().clear();
    }
}
void SynthesisTab::reset()
{
    // Required to ensure synching with other source
    const ScopedLock sl (synthesiserCriticalSection);

    for (auto&& oscillator : oscillators)
    {
        oscillator.reset();
        oscillator.setFrequency (static_cast<float> (currentFrequency), true);
    }

    resetSweep();
}
void SynthesisTab::timerCallback ()
{
    jassert (isSweepEnabled);
    sldFrequency->setValue (getSweepFrequency(), sendNotificationAsync);
}
void SynthesisTab::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldFrequency)
    {
        currentFrequency = sldFrequency->getValue();
        sweepStartFrequency = sldFrequency->getMinValue();
        sweepEndFrequency = sldFrequency->getMaxValue();
    }
    if (sliderThatWasMoved == sldSweepDuration)
    {
        sweepDuration = sldSweepDuration->getValue();
        calculateNumSweepSteps();
    }
}

bool SynthesisTab::isSelectedWaveformOscillatorBased() const
{
    return (    currentWaveform == Waveform::sine 
             || currentWaveform == Waveform::saw 
             || currentWaveform == Waveform::square 
             || currentWaveform == Waveform::triangle
           );
}
void SynthesisTab::waveformUpdated()
{
    // Store locally so audio routines can check value safely
    currentWaveform = static_cast<Waveform> (cmbWaveform->getSelectedId());

    // Only enable sweep controls for oscillator based waveforms
    sldSweepDuration->setEnabled (isSelectedWaveformOscillatorBased());
    btnSweepEnabled->setEnabled (isSelectedWaveformOscillatorBased());
    btnSweepReset->setEnabled (isSelectedWaveformOscillatorBased());
    sldFrequency->setEnabled (isSelectedWaveformOscillatorBased());
}
void SynthesisTab::updateSweepEnablement ()
{
    isSweepEnabled = btnSweepEnabled->getToggleState();
    
    sldSweepDuration->setEnabled (isSweepEnabled);
    
    if (isSweepEnabled)
        startTimerHz (50);
    else
        stopTimer();
}
void SynthesisTab::resetSweep ()
{
    sweepStepIndex = 0;
    sweepStepDelta = 1;
}
double SynthesisTab::getSweepFrequency() const
{
    //f(x) = 10^(log(span)/n*x) + fStart
    //where:
    //    x = the number of the sweep point
    //    n = total number of sweep points
    const auto span = sweepEndFrequency - sweepStartFrequency;
    return pow (10, log10 (span) / numSweepSteps * sweepStepIndex) + sweepStartFrequency;
}
void SynthesisTab::calculateNumSweepSteps()
{
    numSweepSteps = static_cast<long> (sweepDuration * sampleRate / static_cast<double> (maxBlockSize));
}

//SampleTab::SampleTab ()
//{
//    addAndMakeVisible (cmbSample = new ComboBox ("Select Sample"));
//    cmbSample->addItem ("None", 1);
//    cmbSample->setSelectedId (1, dontSendNotification);
//    cmbSample->onChange = [this] { selectedSampleChanged(); };
//
//    addAndMakeVisible (btnLoopEnabled = new TextButton ("Loop Enabled"));
//    btnLoopEnabled->setClickingTogglesState (true);
//    //btnLoopEnabled->setToggleState (true, dontSendNotification);
//    btnLoopEnabled->setColour (TextButton::buttonOnColourId, Colours::green);
//    btnLoopEnabled->onClick = [this] { loopEnablementToggled(); };
//
//    // Add delay control to prevent machine gunning of sample?
//}
//SampleTab::~SampleTab ()
//{
//    cmbSample = nullptr;
//    btnLoopEnabled = nullptr;
//}
//void SampleTab::paint (Graphics&)
//{
//}
//void SampleTab::resized ()
//{
//    cmbSample->setBoundsRelative (0.1f, 0.2f, 0.8f, 0.2f);
//    btnLoopEnabled->setBoundsRelative (0.1f, 0.5f, 0.8f, 0.2f);
//}
//void SampleTab::selectedSampleChanged()
//{ }
//void SampleTab::loopEnablementToggled()
//{ }
//void SampleTab::prepare (const dsp::ProcessSpec&)
//{ }
//void SampleTab::process (const dsp::ProcessContextReplacing<float>& context)
//{ }
//void SampleTab::reset()
//{ }

WaveTab::AudioThumbnailComponent::AudioThumbnailComponent(AudioDeviceManager* deviceManager, AudioFormatManager* formatManager)
    : audioDeviceManager (deviceManager),
      audioFormatManager (formatManager),
      thumbnailCache (5),
      thumbnail (128, *formatManager, thumbnailCache)
{
    thumbnail.addChangeListener (this);
}
WaveTab::AudioThumbnailComponent::~AudioThumbnailComponent()
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
void WaveTab::AudioThumbnailComponent::clear ()
{
    thumbnail.clear();
    fileLoaded = false;
}
void WaveTab::AudioThumbnailComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == reinterpret_cast<ChangeBroadcaster*> (&thumbnail) || source == this)
        repaint();
}
void WaveTab::AudioThumbnailComponent::timerCallback ()
{
    if (transportSource != nullptr && fileLoaded)
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
    fileLoaded = true;

    if (notify)
        sendChangeMessage();
}
void WaveTab::AudioThumbnailComponent::mouseDrag (const MouseEvent& e)
{
    if (transportSource != nullptr)
    {
        const ScopedLock sl (audioDeviceManager->getAudioCallbackLock());

        transportSource->setPosition ((jmax (static_cast<double> (e.x), 0.0) / getWidth())
                                        * thumbnail.getTotalLength());
    }
}
bool WaveTab::AudioThumbnailComponent::isFileLoaded() const
{
    return fileLoaded;
}

WaveTab::WaveTab(AudioDeviceManager* deviceManager)
    :   audioDeviceManager (deviceManager),
        sampleRate (0.0),
        maxBlockSize (0)
{
    formatManager.registerBasicFormats();

    addAndMakeVisible(audioThumbnailComponent = new AudioThumbnailComponent (audioDeviceManager, &formatManager));
    audioThumbnailComponent->addChangeListener (this);

    addAndMakeVisible (btnLoad = new TextButton ("Load"));
    btnLoad->setTooltip ("Load wave file");
    btnLoad->onClick = [this] { chooseFile(); };

    addAndMakeVisible (btnPlay = new TextButton ("Play"));
    btnPlay->setTooltip ("Play/pause (playing will loop once the end of the file is reached)");
    btnPlay->setClickingTogglesState (true);
    btnPlay->setColour (TextButton::buttonOnColourId, Colours::green);
    btnPlay->onClick = [this] {
        if (!audioThumbnailComponent->isFileLoaded())
            btnPlay->setToggleState(false, dontSendNotification);
        else if (btnPlay->getToggleState())
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
    btnLoop->onClick = [this] { 
        if (readerSource != nullptr)
            readerSource->setLooping (btnLoop->getToggleState());
    };

}
WaveTab::~WaveTab ()
{
}
void WaveTab::paint (Graphics&)
{ }
void WaveTab::resized ()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (1_fr),
                            Track (GUI_BASE_SIZE_PX)
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
    
    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
float WaveTab::getMinimumWidth ()
{
    // This is an estimate that allows us to use relative widths in grid layout in resized()
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemWidth = GUI_SIZE_F(3 * 4);
    const auto totalItemGaps = GUI_GAP_F(3);
    return innerMargin + totalItemWidth + totalItemGaps;
}
float WaveTab::getMinimumHeight ()
{
    // This is an estimate that allows us to use relative heights in grid layout in resized()
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_SIZE_F(3);
    const auto totalItemGaps = GUI_GAP_F(2);
    return innerMargin + totalItemHeight + totalItemGaps;
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
    btnPlay->setToggleState (false, dontSendNotification);
    audioThumbnailComponent->clear();
    transportSource.reset();
    readerSource.reset();
    reader.reset();
    fileReadBuffer.clear();
    init();
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

    reader = formatManager.createReaderFor (fileToPlay);
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
            if (auto* device = audioDeviceManager->getCurrentAudioDevice())
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

AudioTab::ChannelComponent::ChannelComponent (SimplePeakMeterProcessor* meterProcessorToQuery, const int numberOfOutputChannels, const int channelIndex)
    :   meterProcessor (meterProcessorToQuery),
        numOutputs (numberOfOutputChannels),
        channel (channelIndex)
{
    // Set opaque to reduce performance impact of meters redrawing
    this->setOpaque (true);

    // Initially map each input channel to the corresponding output channel (if it exists)
    if (channel < numberOfOutputChannels)
        selectedOutputChannels.setBit (static_cast<int> (channel));

    lblChannel.setText ("In " + String (channelIndex), dontSendNotification);
    lblChannel.setJustificationType (Justification::centred);
    lblChannel.setFont (Font(GUI_SIZE_F(0.5)).boldened());
    addAndMakeVisible (lblChannel);

    meterBar.setTooltip("Signal level for input channel " + String (channelIndex));
    addAndMakeVisible (meterBar);

    sldGain.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    sldGain.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    sldGain.setRange (-100.0, 15.0, 1.0);
    sldGain.setPopupDisplayEnabled (true, true, nullptr);
    sldGain.setTextValueSuffix (" dB");
    sldGain.setTooltip("Set gain for copying input channel " + String (channelIndex) + " to outputs (doesn't affect input meter)");
    sldGain.addListener (this);
    addAndMakeVisible (sldGain);

    btnOutputSelection.setButtonText("Outs");
    btnOutputSelection.setTooltip ("Assign input channel " + String (channelIndex) + " to different output channels");
    btnOutputSelection.setTriggeredOnMouseDown (true);
    btnOutputSelection.onClick  = [this] {
        auto options = PopupMenu::Options().withTargetComponent (&btnOutputSelection);
        getOutputMenu().showMenuAsync (options, new MenuCallback (this));
    };
    addAndMakeVisible (btnOutputSelection);
}
AudioTab::ChannelComponent::~ChannelComponent() = default;
void AudioTab::ChannelComponent::paint (Graphics& g)
{
    // Remember that this component is opaque, so it should be filled with colour
    g.fillAll (Colours::darkgrey.darker (0.3f));
}
void AudioTab::ChannelComponent::resized ()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX),
                            Track (GUI_SIZE_PX(0.75)),
                            Track (1_fr)
                        };

    grid.templateColumns = { Track (GUI_SIZE_PX(0.6)), Track (GUI_SIZE_PX(1.4)) };

    grid.autoFlow = Grid::AutoFlow::column;

    grid.items.addArray({   GridItem (meterBar).withArea (GridItem::Span (3), {}).withMargin(GridItem::Margin (0.0f, 2.0f, 0.0f, 0.0f)),
                            GridItem (lblChannel),
                            GridItem (btnOutputSelection),
                            GridItem (sldGain)
                        });
    grid.performLayout (getLocalBounds().reduced (GUI_BASE_GAP_I, GUI_BASE_GAP_I));
}
float AudioTab::ChannelComponent::getMinimumWidth()
{
    // This is an exact calculation of the width we want for a channel component
    const auto innerMargin = GUI_GAP_F(2);
    const auto totalItemWidth = GUI_SIZE_F(2);
    const auto totalItemGaps = GUI_BASE_GAP_F;
    return innerMargin + totalItemWidth + totalItemGaps;
}
float AudioTab::ChannelComponent::getMinimumHeight()
{
    // This is an approximate calculation of the minimum height we want for a channel component (the rotary slider can grow in height)
    const auto innerMargin = GUI_GAP_F(2);
    const auto totalItemHeight = GUI_SIZE_F(4);
    const auto totalItemGaps = GUI_GAP_F(3);
    return innerMargin + totalItemHeight + totalItemGaps;
}
void AudioTab::ChannelComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &sldGain)
    {
        const auto currentGain = static_cast<float> (sldGain.getValue());
        const auto minGain = static_cast<float> (sldGain.getMinimum());
        currentLinearGain = Decibels::decibelsToGain (currentGain, minGain);
    }
}
void AudioTab::ChannelComponent::setActive (bool shouldBeActive)
{
    active = shouldBeActive;
}
void AudioTab::ChannelComponent::setNumOutputChannels (const int numberOfOutputChannels)
{
    numOutputs = numberOfOutputChannels;
}
BigInteger AudioTab::ChannelComponent::getSelectedOutputs() const
{
    return selectedOutputChannels;
}
void AudioTab::ChannelComponent::toggleOutputSelection (const int channelNumber)
{
    if (selectedOutputChannels[channelNumber] == 1)
        selectedOutputChannels.clearBit (channelNumber);
    else
        selectedOutputChannels.setBit (channelNumber);
}
bool AudioTab::ChannelComponent::isOutputSelected (const int channelNumer) const
{
    return selectedOutputChannels[static_cast<int> (channelNumer)];
}
void AudioTab::ChannelComponent::reset()
{
    sldGain.setValue(0.0, sendNotificationSync);
}
void AudioTab::ChannelComponent::refresh ()
{
    auto dB = -100.0f;
    if (active)
        dB = meterProcessor->getLeveldB (static_cast<int> (channel));
    meterBar.setLevel (dB);
}
float AudioTab::ChannelComponent::getLinearGain() const
{
    return currentLinearGain.get();
}
AudioTab::ChannelComponent::MenuCallback::MenuCallback (ChannelComponent* parentComponent)
    : parent (parentComponent)
{
}
PopupMenu AudioTab::ChannelComponent::getOutputMenu() const
{
    PopupMenu menu;
    for (int ch = 0; ch < numOutputs; ++ch)
        menu.addItem (ch + 1, "Output " + String (ch), true, isOutputSelected (ch));
    return menu;
}
void AudioTab::ChannelComponent::MenuCallback::modalStateFinished (int returnValue)
{
    parent->toggleOutputSelection (returnValue - 1);
}

AudioTab::InputArrayComponent::InputArrayComponent (OwnedArray<ChannelComponent>* channelComponentsToReferTo)
    : channelComponents (channelComponentsToReferTo)
{ }
AudioTab::InputArrayComponent::~InputArrayComponent() = default;
void AudioTab::InputArrayComponent::paint (Graphics&)
{ }
void AudioTab::InputArrayComponent::resized ()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.autoColumns = Track (Grid::Px (ChannelComponent::getMinimumWidth()));
    grid.autoRows = Track (1_fr);
    grid.autoFlow = Grid::AutoFlow::column;

    for (auto channelComponent : *channelComponents)
        grid.items.add (GridItem (channelComponent));

    grid.performLayout (getLocalBounds().reduced (GUI_BASE_GAP_I, GUI_BASE_GAP_I));
}
float AudioTab::InputArrayComponent::getMinimumWidth() const
{
    const auto channelWidth = ChannelComponent::getMinimumWidth();
    const auto channelGap = GUI_BASE_GAP_I;    // Set in ChannelComponent resized()
    const auto margins = GUI_GAP_I(2);         // Set in InputArrayComponent::resized()
    const auto numInputs = channelComponents->size();
    return numInputs * channelWidth + jmax (0, numInputs - 1) * channelGap + margins;
}

AudioTab::AudioTab ()
    : inputArrayComponent (&channelComponents)
{
    viewport.setScrollBarsShown (false, true);
    viewport.setViewedComponent (&inputArrayComponent);
    addAndMakeVisible (viewport);
}
AudioTab::~AudioTab() = default;
void AudioTab::paint (Graphics&)
{ }
void AudioTab::resized ()
{
    viewport.setBounds (getLocalBounds());
}
float AudioTab::getMinimumWidth ()
{
    // We'll take whatever we get
    return 0.0f;
}
float AudioTab::getMinimumHeight ()
{
    // We'll take whatever we get
    return 0.0f;
}
void AudioTab::prepare (const dsp::ProcessSpec& spec)
{
    meterProcessor.prepare (spec);
    tempBuffer.setSize (spec.numChannels, spec.maximumBlockSize);
}
void AudioTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    meterProcessor.process (context);

    auto input = context.getInputBlock();
    auto output = context.getOutputBlock();
    auto temp = dsp::AudioBlock<float> (tempBuffer);
    const auto numInputChannels = static_cast<int> (input.getNumChannels());
    const auto numOutputChannels = static_cast<int> (output.getNumChannels());

    // Apply gains to input channels
    for (auto ch = 0; ch < numInputChannels; ++ch)
    {
        const auto linearGain = channelComponents[ch]->getLinearGain();
        for (auto i = 0; i < input.getNumSamples(); ++i)
            input.getChannelPointer (ch)[i] *= linearGain;
    }
    temp.copy (input);
    
    // Add inputs to assigned outputs
    output.clear();
    for (auto outCh = 0; outCh < numOutputChannels; ++outCh)
    {
        for (auto inCh = 0; inCh < numInputChannels; ++inCh)
        {
            auto outputChannel = output.getSingleChannelBlock (outCh);
            if (channelComponents[inCh]->isOutputSelected (outCh))
                outputChannel.add(temp.getSingleChannelBlock(inCh));
        }
    }
}
void AudioTab::reset ()
{
    meterProcessor.reset();

    for (auto ch : channelComponents)
        ch->reset();
}
void AudioTab::timerCallback ()
{
    for (auto ch : channelComponents)
        ch->refresh();
}
void AudioTab::setNumChannels (const int numberOfInputChannels, const int numberOfOutputChannels)
{
    channelComponents.clear();

    for (auto ch  = 0; ch < numberOfInputChannels; ++ ch)
        inputArrayComponent.addAndMakeVisible (channelComponents.add (new ChannelComponent (&meterProcessor, numberOfOutputChannels, ch)));
        
    // Use this code to test the case where there are more channels that can fit within the parent
    //for (auto ch = numInputs; ch < 32; ++ ch)
    //    inputArrayComponent.addAndMakeVisible (channelComponents.add (new ChannelComponent (&meterProcessor, numberOfOutputChannels, ch)));

    const auto viewWidth = inputArrayComponent.getMinimumWidth();
    auto viewHeight = getHeight();
    if (viewWidth>getWidth())
        viewHeight -= viewport.getLookAndFeel().getDefaultScrollbarWidth();
    inputArrayComponent.setSize (static_cast<int> (viewWidth), static_cast<int> (viewHeight));
    
    resized();
}
void AudioTab::setRefresh (const bool shouldRefresh)
{
    if (shouldRefresh)
    {
        for (auto ch : channelComponents)
            ch->setActive (true);
        startTimerHz (50);
    }
    else
    {
        stopTimer();
        for (auto ch : channelComponents)
        {
            ch->setActive (false);
            ch->refresh();
        }
    }
}

SourceComponent::SourceComponent (const String& sourceId, AudioDeviceManager* deviceManager)
    : audioDeviceManager (deviceManager),
      keyName ("Source" + sourceId)
{
    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config.reset (propertiesFile->getXmlValue (keyName));
    if (!config)
    {
        // Define default properties to be used if user settings not already saved
        config.reset(new XmlElement (keyName));
        config->setAttribute ("SourceGain", 0.0);
        config->setAttribute ("Invert", false);
        config->setAttribute ("Mute", false);
        config->setAttribute ("TabIndex", 0);
    }

    gain.setRampDurationSeconds (0.01);
    
    addAndMakeVisible (lblTitle = new Label ("Source label", TRANS("Source") + " " + String (sourceId)));
    lblTitle->setFont (Font (GUI_SIZE_F(0.7), Font::bold));
    lblTitle->setJustificationType (Justification::topLeft);
    lblTitle->setEditable (false, false, false);
    lblTitle->setColour (TextEditor::textColourId, Colours::black);
    lblTitle->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sldGain = new Slider ("Source gain slider"));
    sldGain->setTooltip (TRANS("Adjusts the gain of this source"));
    sldGain->setRange (-100, 50, 0.1);
    sldGain->setDoubleClickReturnValue (true, 0.0);
    sldGain->setSliderStyle (Slider::LinearHorizontal);
    sldGain->setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldGain->setValue (config->getDoubleAttribute ("SourceGain"));
    sldGain->addListener (this);

    addAndMakeVisible (btnInvert = new TextButton ("Invert Source button"));
    btnInvert->setButtonText (TRANS("Invert"));
    btnInvert->setClickingTogglesState (true);
    btnInvert->setColour(TextButton::buttonOnColourId, Colours::green);
    isInverted = config->getBoolAttribute ("Invert");
    btnInvert->setToggleState (isInverted, dontSendNotification);
    btnInvert->onClick = [this] { isInverted = btnInvert->getToggleState(); };

    addAndMakeVisible (btnMute = new TextButton ("Mute Source button"));
    btnMute->setButtonText (TRANS("Mute"));
    btnMute->setClickingTogglesState (true);
    btnMute->setColour(TextButton::buttonOnColourId, Colours::darkred);
    isMuted = config->getBoolAttribute ("Mute");
    btnMute->setToggleState (isMuted, dontSendNotification);
    btnMute->onClick = [this] {
        isMuted = btnMute->getToggleState();
        audioTab->setRefresh (!isMuted);
    };

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (GUI_BASE_SIZE_I);
    tabbedComponent->addTab (TRANS("Synthesis"), Colours::darkgrey, synthesisTab = new SynthesisTab(), false, Mode::Synthesis);
    //tabbedComponent->addTab (TRANS("Sample"), Colours::darkgrey, sampleTab = new SampleTab(), false, Mode::Sample);
    tabbedComponent->addTab (TRANS("Wave File"), Colours::darkgrey, waveTab = new WaveTab(audioDeviceManager), false, Mode:: WaveFile);
    tabbedComponent->addTab (TRANS("Audio In"), Colours::darkgrey, audioTab = new AudioTab(), false, Mode::AudioIn);
    tabbedComponent->setCurrentTabIndex (config->getIntAttribute("TabIndex"));
    tabbedComponent->getTabbedButtonBar().addChangeListener(this);
}
SourceComponent::~SourceComponent()
{
    // Update configuration from class state
    config->setAttribute ("SourceGain", sldGain->getValue());
    config->setAttribute ("Invert", isInverted);
    config->setAttribute ("Mute", isMuted);
    config->setAttribute ("TabIndex", tabbedComponent->getCurrentTabIndex());
    
    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue(keyName, config.get());
    propertiesFile->saveIfNeeded();

    lblTitle = nullptr;
    sldGain = nullptr;
    btnMute = nullptr;
    tabbedComponent = nullptr;
}
void SourceComponent::paint (Graphics& g)
{
    g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void SourceComponent::resized()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   
                            Track (GUI_BASE_SIZE_PX),
                            Track (Grid::Px (getDesiredTabComponentHeight()))
                        };

    grid.templateColumns = { GUI_SIZE_PX(3.0), 1_fr, GUI_SIZE_PX(2), GUI_SIZE_PX(1.7) };

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (sldGain).withMargin (GridItem::Margin (0.0f, GUI_GAP_F(3), 0.0f, 0.0f)),
                            GridItem (btnInvert),
                            GridItem (btnMute),
                            GridItem (tabbedComponent).withArea ({ }, GridItem::Span (4))
                        });
    
    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
float SourceComponent::getMinimumWidth() const
{
    // This is an exact calculation of the width we want
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemWidth = getDesiredTabComponentWidth();
    return innerMargin + totalItemWidth;
}
float SourceComponent::getMinimumHeight() const
{
    // This is an exact calculation of the height we want
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_BASE_SIZE_F + getDesiredTabComponentHeight();
    const auto totalItemGaps = GUI_BASE_GAP_F;
    return innerMargin + totalItemHeight + totalItemGaps;
}
void SourceComponent::sliderValueChanged (Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == sldGain)
    {
        gain.setGainDecibels (static_cast<float> (sldGain->getValue()));
    }
}
void SourceComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == dynamic_cast<ChangeBroadcaster*> (&tabbedComponent->getTabbedButtonBar()))
    {
        const auto shouldRefresh = tabbedComponent->getCurrentTabIndex() == Mode::AudioIn;
        audioTab->setRefresh (shouldRefresh);
    }
}
void SourceComponent::prepare (const dsp::ProcessSpec& spec)
{
    synthesisTab->prepare (spec);
    //sampleTab->prepare (spec);
    waveTab->prepare (spec);
    audioTab->prepare (spec);
    gain.prepare (spec);

    jassert (sldGain != nullptr); // If this is null then gain won't initialise and you won't hear a sound until the slider is moved
    if (sldGain != nullptr)
        gain.setGainDecibels (static_cast<float> (sldGain->getValue()));
}
void SourceComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (!isMuted)
    {
        // Process currently selected source
        const auto idx = static_cast<Mode> (tabbedComponent->getCurrentTabIndex()); // this should be safe to call from an audio routine
        switch (idx) {
            case Synthesis:
                synthesisTab->process (context);
                break;
            //case Sample:
                //sampleTab->process (context);
                //break;
            case WaveFile:
                waveTab->process (context);
                break;
            case AudioIn:
                audioTab->process (context);
                break;
            default: ; // Do nothing
        }

        // Apply gain
        gain.process (context);

        if (isInverted)
            context.getOutputBlock().multiply(-1.0f);
    }
    else
        context.getOutputBlock().clear();
}
void SourceComponent::reset ()
{
    synthesisTab->reset();
    //sampleTab->reset();
    waveTab->reset();
    audioTab->reset();
    gain.reset();
}
void SourceComponent::setNumChannels (int numInputChannels, int numOutputChannels)
{
    audioTab->setNumChannels (numInputChannels, numOutputChannels);
}
SourceComponent::Mode SourceComponent::getMode() const
{
    // This is just looking up a local variable inside the tabbed component, so should be safe enough for audio processing
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
void SourceComponent::mute()
{
    btnMute->setToggleState (true, sendNotificationSync);
}
float SourceComponent::getDesiredTabComponentWidth() const
{
    // Calculate size of tabbed component
    const auto tabInnerWidth = jmax (synthesisTab->getMinimumWidth(), jmax (waveTab->getMinimumWidth(), audioTab->getMinimumWidth()));
    const auto tabBorders = 2.0f;
    return tabInnerWidth + tabBorders;
}
float SourceComponent::getDesiredTabComponentHeight() const
{
    // Calculate size of tabbed component
    const auto tabInnerHeight = jmax (synthesisTab->getMinimumHeight(), jmax (waveTab->getMinimumHeight(), audioTab->getMinimumHeight()));
    const auto tabBorder = 2.0f;
    const auto tabMargin = GUI_GAP_F(4);
    const auto tabButtonBarDepth = GUI_BASE_GAP_F + tabBorder + 1.0f;
    return tabInnerHeight + tabBorder + tabMargin + tabButtonBarDepth;
}
