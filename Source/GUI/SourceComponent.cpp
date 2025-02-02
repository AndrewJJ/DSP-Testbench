/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "SourceComponent.h"
#include <utility>
#include "../Main.h"

SynthesisTab::SynthesisTab (String& sourceName)
    : keyName (sourceName + "_Synthesis")
{
    // Assume sample rate of 48K - this will be corrected when prepare() is called
    const auto nyquist = 24000.0;

    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config = propertiesFile->getXmlValue (keyName);
    if (!config)
        config = std::make_unique<XmlElement> (keyName);

    addAndMakeVisible (cmbWaveform);
    cmbWaveform.setTooltip ("Select a waveform");
    cmbWaveform.addItem ("Sine", static_cast<int> (Waveform::Sine));
    cmbWaveform.addItem ("Triangle", static_cast<int> (Waveform::Triangle));
    cmbWaveform.addItem ("Square", static_cast<int> (Waveform::Square));
    cmbWaveform.addItem ("Saw", static_cast<int> (Waveform::Saw));
    cmbWaveform.addItem ("Impulse", static_cast<int> (Waveform::Impulse));
    cmbWaveform.addItem ("Step", static_cast<int> (Waveform::Step));
    cmbWaveform.addItem ("White Noise", static_cast<int> (Waveform::WhiteNoise));
    cmbWaveform.addItem ("Pink Noise", static_cast<int> (Waveform::PinkNoise));
    cmbWaveform.onChange = [this] { waveformUpdated(); };
    cmbWaveform.setSelectedId (config->getIntAttribute ("WaveForm", static_cast<int> (Waveform::Sine)), sendNotificationSync);

    addAndMakeVisible (sldFrequency);
    sldFrequency.setSliderStyle (Slider::ThreeValueHorizontal);
    sldFrequency.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldFrequency.setTooltip ("Sets the oscillator frequency in Hertz");
    sldFrequency.setRange (10.0, nyquist, 1.0);
    sldFrequency.setSkewFactor (0.5);
    sldFrequency.onValueChange = [this]
    {
        currentFrequency = sldFrequency.getValue();
        sweepStartFrequency = sldFrequency.getMinValue();
        sweepEndFrequency = sldFrequency.getMaxValue();
    };
    sldFrequency.setMinAndMaxValues (config->getDoubleAttribute ("SweepMin", 10.0), config->getDoubleAttribute ("SweepMax", nyquist), dontSendNotification);
    sldFrequency.setValue (config->getDoubleAttribute ("Frequency", 440.0), sendNotificationSync);

    addAndMakeVisible (sldSweepDuration);
    sldSweepDuration.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldSweepDuration.setTooltip ("Sets the duration of the logarithmic frequency sweep in seconds");
    sldSweepDuration.setRange (0.5, 5.0, 0.1);
    sldSweepDuration.onValueChange = [this]
    {
        sweepDuration = sldSweepDuration.getValue();
        calculateNumSweepSteps();
    };
    sldSweepDuration.setValue (config->getDoubleAttribute ("SweepDuration", 1.0), sendNotificationSync);
    
    addAndMakeVisible (cmbSweepMode);
    cmbSweepMode.setTooltip ("Select whether the frequency sweep wraps or reverses when it reaches its maximum value");
    cmbSweepMode.addItem ("Reverse", static_cast<int> (SweepMode::Reverse));
    cmbSweepMode.addItem ("Wrap", static_cast<int> (SweepMode::Wrap));
    cmbSweepMode.onChange = [this] { currentSweepMode = static_cast<SweepMode> (cmbSweepMode.getSelectedId()); };
    cmbSweepMode.setSelectedId (config->getIntAttribute ("SweepMode", static_cast<int> (SweepMode::Reverse)), sendNotificationSync);

    addAndMakeVisible (btnSweepEnabled);
    btnSweepEnabled.setButtonText ("Sweep");
    btnSweepEnabled.setTooltip ("Enable sweeping from start frequency to end frequency");
    btnSweepEnabled.setClickingTogglesState (true);
    btnSweepEnabled.setColour (TextButton::buttonOnColourId, Colours::green);
    btnSweepEnabled.onStateChange = [this] { updateSweepEnablement(); };
    btnSweepEnabled.setToggleState (config->getBoolAttribute ("SweepEnabled"), sendNotificationSync);

    addAndMakeVisible (btnSweepReset);
    btnSweepReset.setButtonText ("Reset");
    btnSweepReset.setTooltip ("Reset/restart the frequency sweep");
    btnSweepReset.onClick = [this] { resetSweep(); };

    addAndMakeVisible (btnSynchWithOther);
    btnSynchWithOther.setButtonText ("Synch");
    btnSynchWithOther.setTooltip ("Synch other source oscillator with this");
    btnSynchWithOther.onClick = [this] { performSynch(); };
    
    addAndMakeVisible (lblPreDelay);
    lblPreDelay.setText("Pre Delay", dontSendNotification);
    lblPreDelay.setJustificationType (Justification::centredRight);

    addAndMakeVisible (sldPreDelay);
    sldPreDelay.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldPreDelay.setTooltip ("Sets the pre-delay for pulse step/impulse functions in samples.\n\nNote that step function has it's minimum pre-delay clamped to 1 so that the first sample is zero.");
    sldPreDelay.setRange (0.0, 1000.0, 1.0);
    sldPreDelay.onValueChange = [this]
    {
        impulseFunction.setPreDelay (static_cast<size_t> (sldPreDelay.getValue()));
        stepFunction.setPreDelay (static_cast<size_t> (sldPreDelay.getValue()));
    };
    sldPreDelay.setValue (static_cast<double> (config->getIntAttribute ("PreDelay", 100)), sendNotificationSync);
    
    addAndMakeVisible (lblPulseWidth);
    lblPulseWidth.setText("Pulse Width", dontSendNotification);
    lblPulseWidth.setJustificationType (Justification::centredRight);
    
    addAndMakeVisible (sldPulseWidth);
    sldPulseWidth.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldPulseWidth.setTooltip ("Sets the pulse width for the step function in samples");
    sldPulseWidth.setRange (1.0, 1000.0, 1.0);
    sldPulseWidth.onValueChange = [this] {
        impulseFunction.setPulseWidth (static_cast<size_t> (sldPulseWidth.getValue()));
    };
    sldPulseWidth.setValue (static_cast<double> (config->getIntAttribute ("PulseWidth", 1)), sendNotificationSync);
    
    addAndMakeVisible (btnPulsePolarity);
    btnPulsePolarity.setTooltip ("Set leading edge of pulse to transition from zero to either full scale positive or negative");
    btnPulsePolarity.setClickingTogglesState (true);
    btnPulsePolarity.setColour (TextButton::buttonOnColourId, Colours::green);
    btnPulsePolarity.setColour (TextButton::buttonColourId, Colours::darkred);
    btnPulsePolarity.onStateChange = [this] {
        stepFunction.setPositivePolarity (btnPulsePolarity.getToggleState());
        impulseFunction.setPositivePolarity (btnPulsePolarity.getToggleState());
        if (btnPulsePolarity.getToggleState())
            btnPulsePolarity.setButtonText ("+ve Polarity");
        else
            btnPulsePolarity.setButtonText ("-ve Polarity");
    };
    btnPulsePolarity.setToggleState (config->getBoolAttribute ("PulsePolarity", true), sendNotificationSync);
}
SynthesisTab::~SynthesisTab ()
{
    // Update configuration from class state
    config->setAttribute ("WaveForm", cmbWaveform.getSelectedId());
    config->setAttribute ("Frequency", sldFrequency.getValue());
    config->setAttribute ("SweepMin", sldFrequency.getMinValue());
    config->setAttribute ("SweepMax", sldFrequency.getMaxValue());
    config->setAttribute ("SweepDuration", sldSweepDuration.getValue());
    config->setAttribute ("SweepMode",cmbSweepMode.getSelectedId());
    config->setAttribute ("SweepEnabled", btnSweepEnabled.getToggleState());
    config->setAttribute ("PreDelay", static_cast<int> (sldPreDelay.getValue()));
    config->setAttribute ("PulseWidth", static_cast<int> (sldPulseWidth.getValue()));
    config->setAttribute ("PulsePolarity", btnPulsePolarity.getToggleState());
    
    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue(keyName, config.get());
    propertiesFile->saveIfNeeded();
}
void SynthesisTab::paint (Graphics&)
{ }
void SynthesisTab::resized ()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.templateRows = { GUI_BASE_SIZE_PX, GUI_BASE_SIZE_PX, GUI_BASE_SIZE_PX, GUI_BASE_SIZE_PX };
    grid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr) };
    if (isSelectedWaveformOscillatorBased())
    {
        grid.items.addArray ({  GridItem (cmbWaveform).withArea ({ }, GridItem::Span (4)),
                                GridItem (sldFrequency).withArea ({ }, GridItem::Span (4)),
                                GridItem (sldSweepDuration).withArea ({ }, GridItem::Span (4)),
                                GridItem (cmbSweepMode), GridItem (btnSweepEnabled), GridItem (btnSweepReset), GridItem (btnSynchWithOther)
                            });
    }
    else
    {
        grid.items.addArray ({  GridItem (cmbWaveform).withArea ({ }, GridItem::Span (4)),
                                GridItem (lblPreDelay), GridItem (sldPreDelay).withArea ({ }, GridItem::Span (3)),
                                GridItem (lblPulseWidth), GridItem (sldPulseWidth).withArea ({ }, GridItem::Span (3)),
                                GridItem (btnPulsePolarity).withArea ({ }, GridItem::Span (3)), GridItem (btnSynchWithOther)
                            });        
    }
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
                                           const double newSweepDuration, const SweepMode sweepMode, const bool sweepEnabled)
{
    cmbWaveform.setSelectedId (static_cast<int> (waveform), sendNotificationSync);
    sldFrequency.setMinAndMaxValues(sweepStart, sweepEnd, sendNotificationSync);
    sldFrequency.setValue(freq, sendNotificationSync);
    cmbSweepMode.setSelectedId (static_cast<int> (sweepMode), sendNotificationSync);
    btnSweepEnabled.setToggleState(sweepEnabled, sendNotificationSync);
    sldSweepDuration.setValue(newSweepDuration, sendNotificationSync);
    this->reset();
}
void SynthesisTab::prepare (const dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxBlockSize = spec.maximumBlockSize;
    const auto nyquist = round (sampleRate / 2.0);
    sldFrequency.setRange (10.0, nyquist, 1.0);

    calculateNumSweepSteps();
    sweepStepIndex = 0;
    sweepStepDelta = 1;

    for (auto&& oscillator : oscillators)
    {
        oscillator.setFrequency (static_cast<float> (currentFrequency));
        oscillator.prepare (spec);
    }

    impulseFunction.prepare (spec);
    stepFunction.prepare (spec);
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
        oscillators[static_cast<int> (currentWaveform) - 1].process (context);
        return;
    }
    if (currentWaveform == Waveform::WhiteNoise)
    {
        whiteNoise.process (context);
        return;
    }
    if (currentWaveform == Waveform::PinkNoise)
    {
        pinkNoise.process (context);
        return;
    }
    if (currentWaveform == Waveform::Impulse)
    {
        impulseFunction.process (context);
        return;
    }
    if (currentWaveform == Waveform::Step)
    {
        stepFunction.process (context);
        return;
    }
    // Catch all in case waveform undefined at some point
    context.getOutputBlock().clear();
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

    impulseFunction.reset();
    stepFunction.reset();
    resetSweep();
}
void SynthesisTab::timerCallback ()
{
    jassert (isSweepEnabled);
    sldFrequency.setValue (getSweepFrequency(), sendNotificationAsync);
}
bool SynthesisTab::isSelectedWaveformOscillatorBased() const
{
    return (    currentWaveform == Waveform::Sine 
             || currentWaveform == Waveform::Saw 
             || currentWaveform == Waveform::Square 
             || currentWaveform == Waveform::Triangle
           );
}
void SynthesisTab::waveformUpdated()
{
    // Store locally so audio routines can check value safely
    currentWaveform = static_cast<Waveform> (cmbWaveform.getSelectedId());

    // Set control enablement based on waveform type
    cmbSweepMode.setEnabled (isSelectedWaveformOscillatorBased());
    sldSweepDuration.setEnabled (isSelectedWaveformOscillatorBased());
    btnSweepEnabled.setEnabled (isSelectedWaveformOscillatorBased());
    btnSweepReset.setEnabled (isSelectedWaveformOscillatorBased());
    sldFrequency.setEnabled (isSelectedWaveformOscillatorBased());
    sldPreDelay.setEnabled (!isSelectedWaveformOscillatorBased());
    sldPulseWidth.setEnabled (currentWaveform == Waveform::Impulse);
    btnPulsePolarity.setEnabled (!isSelectedWaveformOscillatorBased());

    // Set control visibility based on waveform type
    cmbSweepMode.setVisible (isSelectedWaveformOscillatorBased());
    sldSweepDuration.setVisible (isSelectedWaveformOscillatorBased());
    btnSweepEnabled.setVisible (isSelectedWaveformOscillatorBased());
    btnSweepReset.setVisible (isSelectedWaveformOscillatorBased());
    sldFrequency.setVisible (isSelectedWaveformOscillatorBased());
    lblPreDelay.setVisible (currentWaveform == Waveform::Impulse || currentWaveform == Waveform::Step);
    sldPreDelay.setVisible (currentWaveform == Waveform::Impulse || currentWaveform == Waveform::Step);
    lblPulseWidth.setVisible (currentWaveform == Waveform::Impulse);
    sldPulseWidth.setVisible (currentWaveform == Waveform::Impulse);
    btnPulsePolarity.setVisible (!isSelectedWaveformOscillatorBased());

    if (currentWaveform == Waveform::Impulse)
        sldPreDelay.setValue (static_cast<double> (impulseFunction.getPreDelay()), dontSendNotification);
    else if (currentWaveform == Waveform::Step)
        sldPreDelay.setValue (static_cast<double> (stepFunction.getPreDelay()), dontSendNotification);

    // Trigger resized so we redraw the layout grid with different controls
    resized();
}
void SynthesisTab::updateSweepEnablement ()
{
    isSweepEnabled = btnSweepEnabled.getToggleState();
    
    sldSweepDuration.setEnabled (isSweepEnabled);
    
    if (isSweepEnabled)
        startTimerHz (50);
    else
        stopTimer();
}
void SynthesisTab::resetSweep()
{
    sweepStepIndex = 0;
    sweepStepDelta = 1;
    if (isSweepEnabled)
        currentFrequency = sweepStartFrequency;
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
//    addAndMakeVisible (cmbSample);
//    cmbSample.addItem ("None", 1);
//    cmbSample.setSelectedId (1, dontSendNotification);
//    cmbSample.onChange = [this] { selectedSampleChanged(); };
//
//    addAndMakeVisible (btnLoopEnabled);
//    btnLoopEnabled.setButtonText ("Loop Enabled");
//    btnLoopEnabled.setClickingTogglesState (true);
//    //btnLoopEnabled.setToggleState (true, dontSendNotification);
//    btnLoopEnabled.setColour (TextButton::buttonOnColourId, Colours::green);
//    btnLoopEnabled.onClick = [this] { loopEnablementToggled(); };
//
//    // Add delay control to prevent machine gunning of sample?
//}
//void SampleTab::paint (Graphics&)
//{ }
//void SampleTab::resized ()
//{
//    cmbSample.setBoundsRelative (0.1f, 0.2f, 0.8f, 0.2f);
//    btnLoopEnabled.setBoundsRelative (0.1f, 0.5f, 0.8f, 0.2f);
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

WaveTab::AudioThumbnailComponent::AudioThumbnailComponent (AudioDeviceManager* deviceManager, AudioFormatManager* formatManager)
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
    using cols = DspTestBenchLnF::ApplicationColours;

    g.fillAll (cols::audioThumbnailBackground()); 

    g.setColour (cols::audioThumbnailForeground());

    if (thumbnail.getTotalLength() > 0.0)
    {
        thumbnail.drawChannels (g, getLocalBounds().reduced (2),
                                0.0, thumbnail.getTotalLength(), 1.0f);

        g.setColour (cols::audioThumbnailCursor());
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
    // Call reset from the message thread sometime after this method exits
    MessageManager::callAsync ([this] { reset(); });
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
    if (currentFile == f || !f.existsAsFile())
        return;

    currentFile = f;
    thumbnail.setSource (new FileInputSource (f));
    fileLoaded = true;

    if (notify)
        sendChangeMessage();
}
void WaveTab::AudioThumbnailComponent::mouseDown (const MouseEvent & e)
{
    setPositionFromMouse (e);
}
void WaveTab::AudioThumbnailComponent::mouseDrag (const MouseEvent& e)
{
    setPositionFromMouse (e);
}
void WaveTab::AudioThumbnailComponent::setPositionFromMouse (const MouseEvent & e)
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

WaveTab::WaveTab (AudioDeviceManager* deviceManager, const String& initialFilePathFromConfig, const bool shouldPlayOnInitialise_)
    :   audioDeviceManager (deviceManager),
        sampleRate (0.0),
        maxBlockSize (0),
        initialFilePath (std::move(initialFilePathFromConfig)),
        playOnInitialise (shouldPlayOnInitialise_)
{
    formatManager.registerBasicFormats();

    audioThumbnailComponent = std::make_unique<AudioThumbnailComponent> (audioDeviceManager, &formatManager);
    addAndMakeVisible (audioThumbnailComponent.get());
    audioThumbnailComponent->addChangeListener (this);

    addAndMakeVisible (btnLoad);
    btnLoad.setButtonText ("Load");
    btnLoad.onClick = [this] { chooseFile(); };

    addAndMakeVisible (btnPlay);
    btnPlay.setButtonText ("Play");
    btnPlay.setTooltip ("Play/pause");
    btnPlay.setClickingTogglesState (true);
    btnPlay.setColour (TextButton::buttonOnColourId, Colours::green);
    btnPlay.onClick = [this] {
        if (!audioThumbnailComponent->isFileLoaded())
            btnPlay.setToggleState (false, dontSendNotification);
        else if (btnPlay.getToggleState())
            play();
        else
            pause();
    };

    addAndMakeVisible (btnStop);
    btnStop.setButtonText ("Stop");
    btnStop.onClick = [this] { stop(); };

    addAndMakeVisible (btnLoop);
    btnLoop.setButtonText ("Loop");
    btnLoop.setClickingTogglesState (true);
    btnLoop.setToggleState (true, dontSendNotification);
    btnLoop.setColour (TextButton::buttonOnColourId, Colours::green);
    btnLoop.onClick = [this] { 
        readerSource->setLooping (btnLoop.getToggleState());
    };

    addAndMakeVisible (btnSnapshotMode);
    btnSnapshotMode.setButtonText ("|<");
    btnSnapshotMode.setTooltip ("Play from start when snapshot triggered");
    btnSnapshotMode.setClickingTogglesState (true);
    btnSnapshotMode.setToggleState (true, dontSendNotification);
    btnSnapshotMode.setColour (TextButton::buttonOnColourId, Colours::green);
    btnSnapshotMode.onClick = [this] { 
        playFromStartOnSnapshot = btnSnapshotMode.getToggleState();
    };

    // Delay load of initial file using timer so that audio device is set up
    startTimer (20);
}
WaveTab::~WaveTab()
= default;
void WaveTab::paint (Graphics&)
{ }
void WaveTab::resized ()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.templateRows = { Track (1_fr), Track (GUI_BASE_SIZE_PX) };
    grid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr), Track (GUI_SIZE_PX(1)) };
    grid.items.addArray({   GridItem (audioThumbnailComponent.get()).withArea ({ }, GridItem::Span (5)),
                            GridItem (btnLoad), GridItem (btnPlay), GridItem (btnStop), GridItem (btnLoop), GridItem (btnSnapshotMode)
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
    fileReadBuffer.setSize (static_cast<int> (spec.numChannels), static_cast<int> (spec.maximumBlockSize), true, true, false);
    init();
}
void WaveTab::process (const dsp::ProcessContextReplacing<float>& context)
{
    const AudioSourceChannelInfo info (fileReadBuffer);

    if (transportSource)
        // Always read next audio block so pause & stop methods doesn't have a one second time out
        transportSource->getNextAudioBlock (info);

    // But only output the block if currently playing
    if (transportSource && transportSource->isPlaying())
        context.getOutputBlock().copyFrom (fileReadBuffer);
    else
        context.getOutputBlock().clear();
}
void WaveTab::reset()
{
    btnPlay.setToggleState (false, dontSendNotification);
    audioThumbnailComponent->clear();
    transportSource->releaseResources();
    readerSource->releaseResources();
    reader.reset();
    fileReadBuffer.clear();
    init();
}
void WaveTab::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == audioThumbnailComponent.get())
    {
        if (btnPlay.getToggleState())
            stop();
        loadFile (audioThumbnailComponent->getCurrentFile());
    }
    if (transportSource->hasStreamFinished())
        stop();
}
void WaveTab::timerCallback ()
{
    // Load initial file from config
    if (initialFilePath.isNotEmpty())
    {
        const File file (initialFilePath);
        if (loadFile (file))
            audioThumbnailComponent->setCurrentFile (file);
        if (playOnInitialise)
        {
            btnPlay.setToggleState(true, dontSendNotification);
            play();
        }
    }
    stopTimer();
}
String WaveTab::getFilePath() const
{
    return audioThumbnailComponent->getCurrentFile().getFullPathName();
}
bool WaveTab::isPlaying() const
{
    return btnPlay.getToggleState();
}
void WaveTab::storePlayState()
{
    if (transportSource)
    {
        // Store playing state so we can resume after snapshot if play from start is configured
        snapShotPlayStateResume = isPlaying();
        // Store current play position so we can start the snapshot from here if play from start is not configured
        snapShotPlayStatePosition = transportSource->getCurrentPosition();
    }
    else
    {
        snapShotPlayStateResume = false;
        snapShotPlayStatePosition = 0.0;
    }
}
void WaveTab::prepForSnapshot()
{
    if (playFromStartOnSnapshot)
    {
        stop();
        play();
    }
}
void WaveTab::setSnapshotMode(const bool shouldPlayFromStart)
{
    playFromStartOnSnapshot = shouldPlayFromStart;
    btnSnapshotMode.setToggleState (shouldPlayFromStart, sendNotificationSync);
}
bool WaveTab::getSnapshotMode() const
{
    return playFromStartOnSnapshot;
}
bool WaveTab::loadFile (const File& fileToPlay)
{
    stop();

    audioThumbnailComponent->setTransportSource (nullptr);
    transportSource.reset();
    readerSource.reset();

    reader.reset (formatManager.createReaderFor (fileToPlay));
    if (reader)
    {
        readerSource = std::make_unique<AudioFormatReaderSource> (reader.get(), false);
        init();
        return true;
    }
    return false;
}
void WaveTab::chooseFile()
{
    stop();

    fileChooser = std::make_unique<FileChooser> ("Select an audio file...", File::getSpecialLocation (File::userHomeDirectory), "*.wav;*.mp3;*.aif;");
    
    fileChooser->launchAsync (FileBrowserComponent::openMode, [this] (const FileChooser& chooser)
    {
        const auto file (chooser.getResult());

        if (!loadFile (file))
            NativeMessageBox::showOkCancelBox (AlertWindow::WarningIcon, "Error loading file", "Unable to load audio file", nullptr, nullptr);
        else
            audioThumbnailComponent->setCurrentFile (file);
    });
}
void WaveTab::init()
{
    if (!transportSource)
        transportSource = std::make_unique<AudioTransportSource>();

    transportSource->prepareToPlay(maxBlockSize, sampleRate);
    transportSource->addChangeListener (this);
    
    if (readerSource)
    {
        readerSource->setLooping (btnLoop.getToggleState());
        if (auto* device = audioDeviceManager->getCurrentAudioDevice())
        {
            transportSource->setSource (readerSource.get(), roundToInt (device->getCurrentSampleRate()), &DSPTestbenchApplication::getApp(), reader->sampleRate);
            // tell the main window about this so that it can do the seeking behaviour...
            audioThumbnailComponent->setTransportSource (transportSource.get());
            
            if (btnPlay.getToggleState())
                transportSource->start();
            
            if (playFromStartOnSnapshot && snapShotPlayStateResume)
            {
                btnPlay.setToggleState(true, sendNotificationSync);
                snapShotPlayStateResume = false;
            }
        }
    }
    
    if (!playFromStartOnSnapshot)
    {
        transportSource->setPosition (snapShotPlayStatePosition);
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
void WaveTab::pause()
{
    if (transportSource != nullptr)
        transportSource->stop();
}
void WaveTab::stop()
{
    btnPlay.setToggleState (false, dontSendNotification);
    
    if (transportSource)
    {
        transportSource->stop();
        transportSource->setPosition (0);
    }
}

AudioTab::ChannelComponent::ChannelComponent (PeakMeterProcessor* meterProcessorToQuery, const int numberOfOutputChannels, const int channelIndex)
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
    lblChannel.setFont (Font (FontOptions (GUI_SIZE_F(0.5))).boldened());
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
        channelSelectorPopup = std::make_unique<ChannelSelectorPopup> (numOutputs, "Output", selectedOutputChannels, &btnOutputSelection);
        channelSelectorPopup->onClose = [this] (BigInteger& channelMask) { selectedOutputChannels = channelMask; };
        channelSelectorPopup->setOwner (&channelSelectorPopup);
        channelSelectorPopup->show();
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
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.templateRows = { Track (GUI_BASE_SIZE_PX), Track (GUI_SIZE_PX(0.75)), Track (1_fr) };
    grid.templateColumns = { Track (GUI_SIZE_PX(0.6)), Track (GUI_SIZE_PX(1.4)) };
    grid.autoFlow = Grid::AutoFlow::column;
    grid.items.addArray({   GridItem (meterBar).withArea (GridItem::Span (3), {}).withMargin(GridItem::Margin (0.0f, 2.0f, 0.0f, 0.0f)),
                            GridItem (lblChannel), GridItem (btnOutputSelection), GridItem (sldGain)
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
BigInteger AudioTab::ChannelComponent::getSelectedOutputs() const
{
    return selectedOutputChannels;
}
bool AudioTab::ChannelComponent::isOutputSelected (const int channelNumber) const
{
    return selectedOutputChannels[static_cast<int> (channelNumber)];
}
void AudioTab::ChannelComponent::reset()
{
    sldGain.setValue(0.0, sendNotificationSync);
}
void AudioTab::ChannelComponent::refresh ()
{
    auto dB = -100.0f;
    if (active)
        dB = meterProcessor->getLevelDb (static_cast<int> (channel));
    meterBar.setLevel (dB);
}
float AudioTab::ChannelComponent::getLinearGain() const
{
    return currentLinearGain.get();
}

AudioTab::InputArrayComponent::InputArrayComponent (OwnedArray<ChannelComponent>* channelComponentsToReferTo)
    : channelComponents (channelComponentsToReferTo)
{ }
AudioTab::InputArrayComponent::~InputArrayComponent() = default;
void AudioTab::InputArrayComponent::paint (Graphics&)
{ }
void AudioTab::InputArrayComponent::resized ()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
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
    const auto numIns = channelComponents->size();
    return numIns * channelWidth + jmax (0, numIns - 1) * channelGap + margins;
}

AudioTab::AudioTab (AudioDeviceManager* deviceManager)
    : inputArrayComponent (&channelComponents),
      audioDeviceManager (deviceManager)
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
    channelsChanged();
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
        auto linearGain = 0.0f;
        if (channelComponents[ch] != nullptr)
            linearGain = channelComponents[ch]->getLinearGain();
        for (auto i = 0; i < input.getNumSamples(); ++i)
            output.getChannelPointer (ch)[i] = input.getChannelPointer (ch)[i] * linearGain;
    }
    temp.copyFrom (input);
    
    // Add inputs to assigned outputs
    output.clear();
    for (auto outCh = 0; outCh < numOutputChannels; ++outCh)
    {
        for (auto inCh = 0; inCh < numInputChannels; ++inCh)
        {
            auto outputChannel = output.getSingleChannelBlock (outCh);
            if (channelComponents[inCh] != nullptr && channelComponents[inCh]->isOutputSelected (outCh))
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
void AudioTab::channelsChanged()
{
    const auto currentDevice = audioDeviceManager->getCurrentAudioDevice();
    if (!currentDevice)
        return;

    const auto numInputChannels = static_cast<uint32> (currentDevice->getActiveInputChannels().countNumberOfSetBits());
    const auto numOutputChannels = static_cast<uint32> (currentDevice->getActiveOutputChannels().countNumberOfSetBits());

    if (static_cast<int> (numInputChannels) != numInputs || static_cast<int> (numOutputChannels) != numOutputs)
    {
        numInputs = static_cast<int> (numInputChannels);
        numOutputs = static_cast<int> (numOutputChannels);

        inputArrayComponent.removeAllChildren();
        channelComponents.clear();

        for (auto ch  = 0; ch < numInputs; ++ ch)
            inputArrayComponent.addAndMakeVisible (channelComponents.add (new ChannelComponent (&meterProcessor, numOutputChannels, ch)));
            
        // Use this code to test the case where there are more channels that can fit within the parent
        //for (auto ch = numInputs; ch < 32; ++ ch)
        //    inputArrayComponent.addAndMakeVisible (channelComponents.add (new ChannelComponent (&meterProcessor, numOutputChannels, ch)));

        const auto viewWidth = inputArrayComponent.getMinimumWidth();
        auto viewHeight = getHeight();
        if (viewWidth>getWidth())
            viewHeight -= viewport.getLookAndFeel().getDefaultScrollbarWidth();
        inputArrayComponent.setSize (static_cast<int> (viewWidth), static_cast<int> (viewHeight));
        
        resized();
    }
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
      sourceName ("Source" + sourceId)
{
    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config = propertiesFile->getXmlValue (sourceName);
    if (!config)
        config = std::make_unique<XmlElement> (sourceName);

    // Restore output channel mask from config
    const auto mask = config->getStringAttribute ("OutputChannelMask");
    selectedOutputChannels.parseString (mask, 2);

    gain.setRampDurationSeconds (0.01);
    
    addAndMakeVisible (lblTitle);
    lblTitle.setText (TRANS("Source") + " " + String (sourceId), dontSendNotification);
    lblTitle.setFont (Font (FontOptions (GUI_SIZE_F(0.7), Font::bold)));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);

    addAndMakeVisible (sldGain);
    sldGain.setTooltip (TRANS("Adjusts the gain of this source"));
    sldGain.setRange (-100, 50, 0.1);
    sldGain.setDoubleClickReturnValue (true, 0.0);
    sldGain.setSliderStyle (Slider::LinearHorizontal);
    sldGain.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.0), GUI_SIZE_I(0.7));
    sldGain.setValue (config->getDoubleAttribute ("SourceGain"));
    sldGain.onValueChange = [this] { gain.setGainDecibels (static_cast<float> (sldGain.getValue())); };

    addAndMakeVisible (btnOutputSelection);
    btnOutputSelection.setButtonText (TRANS("Outs"));
    btnOutputSelection.setTooltip (TRANS("Select which output channels are active (others are muted)"));
    btnOutputSelection.onClick = [this]
    {
        if (audioDeviceManager)
        {
            if (const auto* currentDevice = audioDeviceManager->getCurrentAudioDevice())
            {
                const auto newNumOutputs = currentDevice->getActiveOutputChannels().countNumberOfSetBits();
                if (numOutputs == -1)
                {
                    // If the output channel mask has been set
                    if (selectedOutputChannels.countNumberOfSetBits() > 0)
                    {
                        // We won't change the output channel mappings from what is saved
                        numOutputs = newNumOutputs;
                    }
                    else
                    {
                        // Enable each new output channel
                        const auto firstNewChannel = jmax (0, numOutputs - 1);
                        for (auto ch = firstNewChannel; ch < newNumOutputs; ++ch)
                            selectedOutputChannels.setBit(ch);
                        numOutputs = newNumOutputs;
                    }
                }
            }
        }
        channelSelectorPopup = std::make_unique<ChannelSelectorPopup> (numOutputs, "Output", selectedOutputChannels, &btnOutputSelection);
        channelSelectorPopup->onClose = [this] (BigInteger& channelMask) { selectedOutputChannels = channelMask; };
        channelSelectorPopup->setOwner (&channelSelectorPopup);
        channelSelectorPopup->show();
    };

    addAndMakeVisible (btnInvert);
    btnInvert.setButtonText (TRANS("Invert"));
    btnInvert.setClickingTogglesState (true);
    btnInvert.setColour (TextButton::buttonOnColourId, Colours::green);
    isInverted = config->getBoolAttribute ("Invert");
    btnInvert.setToggleState (isInverted, dontSendNotification);
    btnInvert.onClick = [this] { isInverted = btnInvert.getToggleState(); };

    addAndMakeVisible (btnMute);
    btnMute.setButtonText (TRANS("Mute"));
    btnMute.setClickingTogglesState (true);
    btnMute.setColour (TextButton::buttonOnColourId, Colours::darkred);
    isMuted = config->getBoolAttribute ("Mute");
    btnMute.setToggleState (isMuted, dontSendNotification);
    btnMute.onClick = [this] {
        isMuted = btnMute.getToggleState();
        audioTab->setRefresh (!isMuted);
    };

    tabbedComponent = std::make_unique<TabbedComponent> (TabbedButtonBar::TabsAtTop);
    synthesisTab = std::make_unique<SynthesisTab> (sourceName);
    //sampleTab = std::make_unique<SampleTab>();
    waveTab = std::make_unique<WaveTab> (audioDeviceManager, config->getStringAttribute ("WaveFilePath"), config->getBoolAttribute ("WaveFilePlaying"));
    waveTab->setSnapshotMode (config->getBoolAttribute ("WaveFileShouldPlayFromStartOnSnapshot"));
    audioTab = std::make_unique<AudioTab> (audioDeviceManager);
    addAndMakeVisible (tabbedComponent.get());
    tabbedComponent->setTabBarDepth (GUI_BASE_SIZE_I);
    tabbedComponent->addTab (TRANS("Synthesis"), Colours::darkgrey, synthesisTab.get(), false, Mode::Synthesis);
    //tabbedComponent->addTab (TRANS("Sample"), Colours::darkgrey, sampleTab.get(), false, Mode::Sample);
    tabbedComponent->addTab (TRANS("Wave File"), Colours::darkgrey, waveTab.get(), false, Mode:: WaveFile);
    tabbedComponent->addTab (TRANS("Audio In"), Colours::darkgrey, audioTab.get(), false, Mode::AudioIn);
    tabbedComponent->getTabbedButtonBar().addChangeListener(this);
    tabbedComponent->setCurrentTabIndex (config->getIntAttribute("TabIndex")); // Need to set tab after change listener added
}
SourceComponent::~SourceComponent()
{
    // Update configuration from class state
    config->setAttribute ("SourceGain", sldGain.getValue());
    config->setAttribute ("Invert", isInverted);
    config->setAttribute ("Mute", isMuted);
    config->setAttribute ("TabIndex", tabbedComponent->getCurrentTabIndex());
    config->setAttribute ("WaveFilePath", waveTab->getFilePath());
    config->setAttribute ("WaveFilePlaying", waveTab->isPlaying());
    config->setAttribute ("WaveFileShouldPlayFromStartOnSnapshot", waveTab->getSnapshotMode());
    config->setAttribute ("OutputChannelMask", selectedOutputChannels.toString (2));
    
    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue (sourceName, config.get());
    propertiesFile->saveIfNeeded();
}
void SourceComponent::paint (Graphics& g)
{
    g.setColour (DspTestBenchLnF::ApplicationColours::sourceBackground());
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void SourceComponent::resized()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.templateRows = { Track (GUI_BASE_SIZE_PX), Track (Grid::Px (getDesiredTabComponentHeight())) };
    grid.templateColumns = { GUI_SIZE_PX(3.0), 1_fr, GUI_SIZE_PX(1.7), GUI_SIZE_PX(2), GUI_SIZE_PX(1.7) };
    grid.autoFlow = Grid::AutoFlow::row;
    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (sldGain).withMargin (GridItem::Margin (0.0f, GUI_GAP_F(3), 0.0f, 0.0f)),
                            GridItem (btnOutputSelection), GridItem (btnInvert), GridItem (btnMute),
                            GridItem (tabbedComponent.get()).withArea ({ }, GridItem::Span (5))
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
    gain.setGainDecibels (static_cast<float> (sldGain.getValue()));
}
void SourceComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    if (!isMuted)
    {
        // Process currently selected source
        const auto idx = static_cast<Mode> (tabbedComponent->getCurrentTabIndex()); // this should be safe to call from an audio routine
        switch (idx) {
            case Synthesis:  // NOLINT(bugprone-branch-clone)
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
            context.getOutputBlock().multiplyBy (-1.0f);

        // Mute disabled output channels
        for (auto ch = 0; ch < numOutputs; ++ch)
        {
            if (!selectedOutputChannels[ch])
            {
                context.getOutputBlock().getSingleChannelBlock (static_cast<size_t> (ch)).clear();
            }
        }
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
void SourceComponent::storeWavePlayerState() const
{
    waveTab->storePlayState();
}
void SourceComponent::prepForSnapShot()
{
    synthesisTab->reset();
    //sampleTab->reset();
    waveTab->prepForSnapshot();
    audioTab->reset();
    gain.reset();
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
SynthesisTab* SourceComponent::getSynthesisTab() const
{
    return synthesisTab.get();
}
void SourceComponent::mute()
{
    btnMute.setToggleState (true, sendNotificationSync);
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

ChannelSelectorPopup::ChannelSelectorPopup (const int numberOfChannels, const String& channelLabelPrefix, const BigInteger& initialSelection, const Component* componentToPositionNear)
    : numChannels (numberOfChannels),
      channelMask (std::move(initialSelection)),
      anchorComponent (componentToPositionNear)
{
    addKeyListener (this);

    btnAll.setButtonText ("All");
    btnAll.setTooltip ("Select all channels");
    btnAll.onClick = [this]
    {
        // Toggle all channel buttons on
        for (auto b : channelArrayComponent.channelButtons)
        {
            b->setToggleState (true, sendNotificationSync);
        }
    };
    addAndMakeVisible (btnAll);

    btnNone.setButtonText ("None");
    btnNone.setTooltip ("Deselect all channels");
    btnNone.onClick = [this]
    {
        // Toggle all channel buttons off
        for (auto b : channelArrayComponent.channelButtons)
        {
            b->setToggleState (false, sendNotificationSync);
        }
    };
    addAndMakeVisible (btnNone);

    btnDone.setButtonText ("OK");
    btnDone.setColour (TextButton::ColourIds::buttonColourId, Colours::green);
    btnDone.onClick = [this]
    {
        onClose (channelMask);
        dismiss();
    };
    addAndMakeVisible (btnDone);

    btnCancel.setButtonText ("Cancel");
    btnCancel.setColour (TextButton::ColourIds::buttonColourId, Colours::darkred);
    btnCancel.onClick = [this]
    {
        dismiss();
    };
    addAndMakeVisible (btnCancel);

    // Create array of channel toggles
    const auto prefix = (channelLabelPrefix != "") ? channelLabelPrefix : "Channel";
    for (auto ch = 0; ch < numberOfChannels; ++ch)
    {
        auto* t = channelArrayComponent.channelButtons.add (new ToggleButton());
        t->setButtonText (prefix + " " + String (ch));
        t->setToggleState (channelMask[ch], dontSendNotification);
        t->onStateChange = [this, ch, t] { channelMask.setBit (ch, t->getToggleState()); };
        channelArrayComponent.addAndMakeVisible (t);
    }

    viewport.setScrollBarsShown (true, false);
    viewport.setViewedComponent (&channelArrayComponent);
    addAndMakeVisible (viewport);

    // Size and position relative to anchor component
    const auto popupWidth = GUI_SIZE_I (5.5);
    const auto anchorRight = anchorComponent->getScreenX() + componentToPositionNear->getWidth() + GUI_BASE_GAP_I;
    const auto anchorTop = anchorComponent->getScreenY();
    if (const auto* anchorDisplay = Desktop::getInstance().getDisplays().getDisplayForRect (anchorComponent->getScreenBounds()))
    {
        const auto displayWidth = anchorDisplay->userArea.getWidth();
        const auto displayHeight = anchorDisplay->userArea.getHeight();
        const auto popupHeight = calculateHeight();
        const auto left = jmin (anchorRight, displayWidth - popupWidth);
        const auto top = jmin (anchorTop, displayHeight - popupHeight);
        channelArrayComponent.setSize (popupWidth - GUI_GAP_I(4), getViewportInternalHeight());
        setTopLeftPosition (left, top);
        setSize (popupWidth, popupHeight);
    }
}
void ChannelSelectorPopup::paint (Graphics & g)
{
    g.fillAll (DspTestBenchLnF::ApplicationColours::componentBackground());
    g.setColour (DspTestBenchLnF::ApplicationColours::componentOutline());
    g.drawRect (getLocalBounds());
}
void ChannelSelectorPopup::resized()
{
    const auto viewportHeight = getHeight() - getHeightExcludingViewport();
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.templateRows = { 
        Track (GUI_BASE_SIZE_PX),
        Track (Grid::Px (viewportHeight)),
        Track (GUI_BASE_SIZE_PX)
    };
    grid.templateColumns = { Track (1_fr), Track (1_fr) };
    grid.items.addArray( {
            GridItem (btnAll), GridItem (btnNone),
            GridItem (viewport).withArea ({}, GridItem::Span (2)),
            GridItem (btnDone), GridItem (btnCancel)
        });
    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
bool ChannelSelectorPopup::keyPressed (const KeyPress& key, Component* /*originatingComponent*/)
{
    if (key == KeyPress::escapeKey)
        dismiss();
    return true;
}
void ChannelSelectorPopup::show()
{
    setVisible (true);
    addToDesktop (ComponentPeer::StyleFlags::windowIsTemporary | ComponentPeer::StyleFlags::windowHasDropShadow);
    this->toFront (true);
}
void ChannelSelectorPopup::setOwner(std::unique_ptr<ChannelSelectorPopup>* owner)
{
    myOwner = owner;
}
void ChannelSelectorPopup::dismiss()
{
    this->removeFromDesktop();
    if (myOwner)
        myOwner->reset();
}
int ChannelSelectorPopup::calculateHeight() const
{
    const auto desiredHeight = getHeightExcludingViewport() + getViewportInternalHeight();
    return jlimit (getMinimumHeight(), getMaximumHeight(), desiredHeight);
}
int ChannelSelectorPopup::getMinimumHeight() const
{
    const auto minViewPortHeight = GUI_BASE_SIZE_I + GUI_BASE_GAP_I; // 1 channel with a small gap
    return getHeightExcludingViewport() + minViewPortHeight;
}
int ChannelSelectorPopup::getMaximumHeight() const
{
    if (const auto* anchorDisplay = Desktop::getInstance().getDisplays().getDisplayForRect (anchorComponent->getScreenBounds()))
    {
        return anchorDisplay->userArea.getHeight();
    }
    return 100; // Arbitrary value
}
int ChannelSelectorPopup::getHeightExcludingViewport() const
{
    return GUI_SIZE_I(2) + GUI_GAP_I(2) + GUI_GAP_I(4);
}
int ChannelSelectorPopup::getViewportInternalHeight() const
{
    return GUI_SIZE_I (numChannels) + GUI_GAP_I (numChannels - 1);
}
void ChannelSelectorPopup::ChannelArrayComponent::paint (Graphics& g)
{
    g.fillAll (DspTestBenchLnF::ApplicationColours::componentBackground());
}
void ChannelSelectorPopup::ChannelArrayComponent::resized()
{
    using Track = Grid::TrackInfo;
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;
    grid.autoRows = Track (GUI_BASE_SIZE_PX);
    grid.templateColumns = { Track (1_fr) };
    for (auto c : getChildren())
        grid.items.add (GridItem (c));
    grid.performLayout (getLocalBounds().withTrimmedRight (GUI_BASE_GAP_I));
}
