/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MeteringComponents.h"
#include "../Processing/PolyBLEP.h"
#include "../Processing/PulseFunctions.h"
#include "../Processing/NoiseGenerators.h"
#include "../Processing/MeteringProcessors.h"

// Forward declarations
class SourceComponent;
class ChannelSelectorPopup;

enum Waveform
{
    sine = 1,
    triangle,
    square,
    saw,
    impulse,
    step,
    whiteNoise,
    pinkNoise
};

enum SweepMode
{
    Reverse = 1,
    Wrap    
};

class SynthesisTab final : public Component, public dsp::ProcessorBase, public Timer
{
public:
    explicit SynthesisTab (String& sourceName /**< Specifies the name of the Source being used */);
    ~SynthesisTab() override;

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();

    void performSynch();
    void setOtherSource (SourceComponent* otherSourceComponent);
    void syncAndResetOscillator (const Waveform waveform, const double freq,
                                 const double sweepStart, const double sweepEnd,                                 
                                 const double newSweepDuration, SweepMode sweepMode,
                                 const bool sweepEnabled);

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;    
    void timerCallback() override;

private:

    String keyName;
    std::unique_ptr<XmlElement> config {};

    ComboBox cmbWaveform;
    Slider sldFrequency;
    Slider sldSweepDuration;
    ComboBox cmbSweepMode;
    TextButton btnSweepEnabled;
    TextButton btnSweepReset;
    TextButton btnSynchWithOther;
    Label lblPreDelay;
    Slider sldPreDelay;
    Label lblPulseWidth;
    Slider sldPulseWidth;
    TextButton btnPulsePolarity;
    
    SourceComponent* otherSource {};
    CriticalSection synthesiserCriticalSection;
    Waveform currentWaveform = Waveform::sine;
    double sampleRate = 0.0;
    uint32 maxBlockSize = 0;
    long numSweepSteps = 0;
    long sweepStepIndex = 0;
    int sweepStepDelta = 1;
    double currentFrequency = 0.0;
    double sweepStartFrequency = 0.0;
    double sweepEndFrequency = 0.0;
    double sweepDuration = 0.0;
    bool isSweepEnabled = false;
    SweepMode currentSweepMode = SweepMode::Wrap;

    bool isSelectedWaveformOscillatorBased() const;
    void waveformUpdated();
    void updateSweepEnablement();
    void resetSweep();
    double getSweepFrequency() const;
    void calculateNumSweepSteps();

    dsp::PolyBlepOscillator<float> oscillators[4]
    {
        dsp::PolyBlepOscillator<float>::sine,
        dsp::PolyBlepOscillator<float>::triangle,
        dsp::PolyBlepOscillator<float>::square,
        dsp::PolyBlepOscillator<float>::saw
    };

    dsp::WhiteNoiseGenerator whiteNoise {};
    dsp::PinkNoiseGenerator pinkNoise {};
    dsp::PulseFunctionBase<float> impulseFunction {};
    dsp::StepFunction<float> stepFunction {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesisTab)
};

//class SampleTab final : public Component, public dsp::ProcessorBase
//{
//public:
//    SampleTab();
//    ~SampleTab() = default;
//
//    void paint (Graphics& g) override;
//    void resized() override;
//
//    void prepare (const dsp::ProcessSpec& spec) override;
//    void process (const dsp::ProcessContextReplacing<float>& context) override;
//    void reset() override;
//
//private:
//
//    void selectedSampleChanged();
//    void loopEnablementToggled();
//
//    ComboBox cmbSample;
//    TextButton btnLoopEnabled;
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTab)
//};

class WaveTab final : public Component, public dsp::ProcessorBase, public ChangeListener, public Timer
{
public:
    explicit WaveTab (AudioDeviceManager* deviceManager, const String& initialFilePathFromConfig, const bool shouldPlayOnInitialise);
    ~WaveTab() override; 

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();
    
    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    void changeListenerCallback (ChangeBroadcaster* source) override;
    void timerCallback() override;
    String getFilePath() const;
    bool isPlaying() const;
    void storePlayState();
    void prepForSnapshot();
    void setSnapshotMode (const bool shouldPlayFromStart);
    bool getSnapshotMode() const;

    class AudioThumbnailComponent : public Component,
                                    public FileDragAndDropTarget,
                                    public ChangeBroadcaster,
                                    private ChangeListener,
                                    private Timer
    {
    public:
        AudioThumbnailComponent(AudioDeviceManager* deviceManager, AudioFormatManager* formatManager);
        ~AudioThumbnailComponent() override;

        void paint (Graphics& g) override;

        bool isInterestedInFileDrag (const StringArray& files) override;
        void filesDropped (const StringArray& files, int x, int y) override;
        
        void setCurrentFile (const File& f);
        File getCurrentFile() const;
        void setTransportSource (AudioTransportSource* newSource);
        void clear();
        bool isFileLoaded() const;

    private:
        AudioDeviceManager* audioDeviceManager = nullptr;
        AudioFormatManager* audioFormatManager = nullptr;
        AudioThumbnailCache thumbnailCache;
        AudioThumbnail thumbnail;
        AudioTransportSource* transportSource = nullptr;

        File currentFile;
        double currentPosition = 0.0;
        bool fileLoaded = false;

        void changeListenerCallback (ChangeBroadcaster* source) override;
        void timerCallback() override;

        void reset();
        void loadFile (const File& f, bool notify = false);
        void mouseDown (const MouseEvent& e) override;
        void mouseDrag (const MouseEvent& e) override;
        void setPositionFromMouse (const MouseEvent& e);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnailComponent)
    };

private:

    AudioDeviceManager* audioDeviceManager = nullptr;
    std::unique_ptr<AudioThumbnailComponent> audioThumbnailComponent{};
    TextButton btnLoad;
    TextButton btnPlay;
    TextButton btnStop;
    TextButton btnLoop;
    TextButton btnSnapshotMode;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReader> reader{};
    std::unique_ptr<AudioFormatReaderSource> readerSource{};
    std::unique_ptr<AudioTransportSource> transportSource{};

    bool loadFile (const File& fileToPlay);
    void chooseFile();
    void init();
    void play();
    void pause();
    void stop();

    AudioBuffer<float> fileReadBuffer;
    double sampleRate;
    uint32 maxBlockSize;
    String initialFilePath;
    bool playOnInitialise;
    bool playFromStartOnSnapshot = false;
    bool snapShotPlayStateResume = false;
    double snapShotPlayStatePosition = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTab)
};


/**
 * NOTE that properties have not been implemented to retain settings for the AudioTab because:
 * - the number of channels can vary between devices on same machine
 * - things will get a bit messy with devices with dozens of channels
 * - this feature isn't expected to be in high demand :)
 */
class AudioTab final : public Component, public dsp::ProcessorBase, public Timer
{
public:
    AudioTab (AudioDeviceManager* deviceManager);
    ~AudioTab() override;

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    void timerCallback () override;

    void setRefresh (const bool shouldRefresh);

private:

    class ChannelComponent final : public Component, public Slider::Listener
    {
    public:
        ChannelComponent (PeakMeterProcessor* meterProcessorToQuery, const int numberOfOutputChannels, const int channelIndex);;
        ~ChannelComponent() override;

        void paint (Graphics& g) override;
        void resized() override;
        static float getMinimumWidth();
        static float getMinimumHeight();

        void sliderValueChanged (Slider* slider) override;

        void setActive (bool shouldBeActive);
        BigInteger getSelectedOutputs() const;
        bool isOutputSelected (const int channelNumber) const;
        // Resets
        void reset();
        // Queries meter processor to update meter value
        void refresh();
        float getLinearGain() const;

    private:

        Label lblChannel;
        MeterBar meterBar{};
        Slider sldGain;
        TextButton btnOutputSelection;

        bool active = true;
        PeakMeterProcessor* meterProcessor;
        int numOutputs = 0;
        BigInteger selectedOutputChannels = 0;
        int channel = 0;
        Atomic<float> currentLinearGain = 1.0f;
        std::unique_ptr<ChannelSelectorPopup> channelSelectorPopup {};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelComponent)
    };

    class InputArrayComponent final : public Component
    {
    public:
        explicit InputArrayComponent (OwnedArray<ChannelComponent>* channelComponentsToReferTo);
        ~InputArrayComponent() override;

        void paint (Graphics& g) override;
        void resized() override;
        float getMinimumWidth() const;

    private:
        OwnedArray<ChannelComponent>* channelComponents {};
    };

    void channelsChanged();

    PeakMeterProcessor meterProcessor;
    Viewport viewport;
    OwnedArray <ChannelComponent> channelComponents {};
    InputArrayComponent inputArrayComponent;
    AudioBuffer<float> tempBuffer;
    AudioDeviceManager* audioDeviceManager;
    int numInputs = 0;
    int numOutputs = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTab)
};

//==============================================================================

class SourceComponent final : public Component, public ChangeListener, public dsp::ProcessorBase
{
public:

    enum Mode
    {
        Synthesis = 0,
        //Sample,
        WaveFile,
        AudioIn
    };

    SourceComponent (const String& sourceId, AudioDeviceManager* deviceManager);
    ~SourceComponent() override;

    void paint (Graphics& g) override;
    void resized() override;
    float getMinimumWidth() const;
    float getMinimumHeight() const;

    void changeListenerCallback (ChangeBroadcaster* source) override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    void storeWavePlayerState() const;
    void prepForSnapShot();

    Mode getMode() const;
    void setOtherSource (SourceComponent* otherSourceComponent);
    SynthesisTab* getSynthesisTab() const;
    void mute();

private:

    AudioDeviceManager* audioDeviceManager;
    String sourceName;
    std::unique_ptr<XmlElement> config {};

    float getDesiredTabComponentWidth() const;
    float getDesiredTabComponentHeight() const;

    Label lblTitle;
    Slider sldGain;
    TextButton btnOutputSelection;
    TextButton btnInvert;
    TextButton btnMute;
    std::unique_ptr<TabbedComponent> tabbedComponent{};
    std::unique_ptr<SynthesisTab> synthesisTab{};
    //std::unique_ptr<SampleTab> sampleTab;
    std::unique_ptr<WaveTab> waveTab{};
    std::unique_ptr<AudioTab> audioTab{};
    std::unique_ptr<ChannelSelectorPopup> channelSelectorPopup {};

    SourceComponent* otherSource = nullptr;
    bool isInverted = false;
    bool isMuted = false;
    BigInteger selectedOutputChannels = 0;
    int numOutputs = -1;

    dsp::Gain<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};

class ChannelSelectorPopup : public Component, public KeyListener
{
public:
    /** Creates a channel selector popup which you must call show() on.
     *  onClose will be called when the popup is closed, with the channel mask passed as an argument.
     */
    explicit ChannelSelectorPopup (const int numberOfChannels, const String& channelLabelPrefix, const BigInteger& initialSelection, const Component* componentToPositionNear);
    ~ChannelSelectorPopup() override = default;

    void paint (Graphics& g) override;
    void resized() override;
    bool keyPressed (const KeyPress & key, Component * originatingComponent) override;
    void show();
    void dismiss();
    /** If you set the owner, then we'll delete ourselves when we finish. */
    void setOwner (std::unique_ptr<ChannelSelectorPopup>* owner);

    std::function<void (BigInteger&)> onClose;

private:

    class ChannelArrayComponent : public Component
    {
    public:
        ChannelArrayComponent() = default;
        ~ChannelArrayComponent() override = default;
        void paint (Graphics& g) override;
        void resized() override;
        OwnedArray<ToggleButton> channelButtons {};
    };

    int calculateHeight() const;
    int getMinimumHeight() const;
    int getMaximumHeight() const;
    int getHeightExcludingViewport() const;
    int getViewportInternalHeight() const;

    int numChannels = 0;
    BigInteger channelMask {};
    const Component* anchorComponent;
    TextButton btnAll;
    TextButton btnNone;
    TextButton btnDone;
    TextButton btnCancel;
    Viewport viewport;
    ChannelArrayComponent channelArrayComponent;
    std::unique_ptr<ChannelSelectorPopup>* myOwner = nullptr;
};