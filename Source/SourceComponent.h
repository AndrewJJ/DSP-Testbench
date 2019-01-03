/*
  ==============================================================================

    SourceComponent.h
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PolyBLEP.h"
#include "NoiseGenerators.h"
#include "MeteringProcessors.h"
#include "SimpleLevelMeterComponent.h"

// Forward declaration
class SourceComponent;

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

class SynthesisTab : public Component, public dsp::ProcessorBase, public Timer, public Slider::Listener
{
public:
    SynthesisTab();
    ~SynthesisTab();

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
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:
    ScopedPointer<ComboBox> cmbWaveform;
    ScopedPointer<Slider> sldFrequency;
    ScopedPointer<Slider> sldSweepDuration;
    ScopedPointer<ComboBox> cmbSweepMode;
    ScopedPointer<TextButton> btnSweepEnabled;
    ScopedPointer<TextButton> btnSweepReset;
    ScopedPointer<TextButton> btnSynchWithOther;

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

    dsp::WhiteNoiseGenerator whiteNoise;
    dsp::PinkNoiseGenerator pinkNoise;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesisTab)
};

//class SampleTab : public Component, public dsp::ProcessorBase
//{
//public:
//    SampleTab();
//    ~SampleTab();
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
//    ScopedPointer<ComboBox> cmbSample;
//    ScopedPointer<TextButton> btnLoopEnabled;
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTab)
//};

class WaveTab : public Component, public dsp::ProcessorBase, public ChangeListener
{
public:
    WaveTab(AudioDeviceManager* deviceManager);
    ~WaveTab();

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();


    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    void changeListenerCallback (ChangeBroadcaster* source) override;

    class AudioThumbnailComponent : public Component,
                                    public FileDragAndDropTarget,
                                    public ChangeBroadcaster,
                                    private ChangeListener,
                                    private Timer
    {
    public:
        AudioThumbnailComponent(AudioDeviceManager* deviceManager, AudioFormatManager* formatManager);
        ~AudioThumbnailComponent();

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
        void mouseDrag (const MouseEvent& e) override;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnailComponent)
    };

private:
    AudioDeviceManager* audioDeviceManager = nullptr;
    ScopedPointer<AudioThumbnailComponent> audioThumbnailComponent;
    ScopedPointer<TextButton> btnLoad;
    ScopedPointer<TextButton> btnPlay;
    ScopedPointer<TextButton> btnStop;
    ScopedPointer<TextButton> btnLoop;

    AudioFormatManager formatManager;
    ScopedPointer<AudioFormatReader> reader;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    ScopedPointer<AudioTransportSource> transportSource;

    bool loadFile (const File& fileToPlay);
    void chooseFile();
    void init();
    void play();
    void pause();
    void stop();

    AudioBuffer<float> fileReadBuffer;
    double sampleRate;
    uint32 maxBlockSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTab)
};

class AudioTab : public Component, public dsp::ProcessorBase, public Timer
{
public:
    AudioTab();
    ~AudioTab();

    void paint (Graphics& g) override;
    void resized() override;
    static float getMinimumWidth();
    static float getMinimumHeight();

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;
    void timerCallback () override;
    
    void setNumChannels (const int  numberOfInputChannels, const int numberOfOutputChannels);
    void setRefresh (const bool shouldRefresh);

private:

    class ChannelComponent : public Component, public Slider::Listener
    {
    public:
        ChannelComponent (SimplePeakMeterProcessor* meterProcessorToQuery, const int numberOfOutputChannels, const int channelIndex);;
        ~ChannelComponent();

        void paint (Graphics& g) override;
        void resized() override;
        static float getMinimumWidth();
        static float getMinimumHeight();

        void sliderValueChanged (Slider* slider) override;

        void setActive (bool shouldBeActive);
        void setNumOutputChannels (const int numberOfOutputChannels);
        BigInteger getSelectedOutputs() const;
        bool isOutputSelected (const int channelNumer) const;
        // Resets
        void reset();
        // Queries meter processor to update meter value
        void refresh();
        float getLinearGain() const;

    private:

        void toggleOutputSelection (const int channelNumber);
        PopupMenu getOutputMenu() const;

        class MenuCallback : public ModalComponentManager::Callback
        {
        public:
            MenuCallback (ChannelComponent* parentComponent);
            void modalStateFinished (int returnValue) override;
        private:
            ChannelComponent* parent;
        };

        Label lblChannel;
        SimplePeakMeterComponent meterBar{};
        Slider sldGain;
        TextButton btnOutputSelection;

        bool active = true;
        SimplePeakMeterProcessor* meterProcessor;
        int numOutputs = 0;
        BigInteger selectedOutputChannels = 0;
        int channel = 0;
        Atomic<float> currentLinearGain = 1.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelComponent)
    };

    class InputArrayComponent : public Component
    {
    public:
        InputArrayComponent (OwnedArray<ChannelComponent>* channelComponentsToReferTo);
        ~InputArrayComponent();

        void paint (Graphics& g) override;
        void resized() override;
        float getMinimumWidth() const;

    private:
        OwnedArray<ChannelComponent>* channelComponents {};
    };

    SimplePeakMeterProcessor meterProcessor;
    Viewport viewport;
    OwnedArray <ChannelComponent> channelComponents {};
    InputArrayComponent inputArrayComponent;
    AudioBuffer<float> tempBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTab)
};

//==============================================================================

class SourceComponent  : public Component, public Slider::Listener, public ChangeListener, public dsp::ProcessorBase
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
    ~SourceComponent();

    void paint (Graphics& g) override;
    void resized() override;
    float getMinimumWidth() const;
    float getMinimumHeight() const;

    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void changeListenerCallback (ChangeBroadcaster* source) override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    // Should be called before prepare is called
    void setNumChannels (int numInputChannels, int numOutputChannels);

    Mode getMode() const;
    void setOtherSource (SourceComponent* otherSourceComponent);
    SynthesisTab* getSynthesisTab();
    void mute();

private:

    AudioDeviceManager* audioDeviceManager;
    String keyName;
    std::unique_ptr<XmlElement> config{};

    float getDesiredTabComponentWidth() const;
    float getDesiredTabComponentHeight() const;

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldGain;
    ScopedPointer<TextButton> btnInvert;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TabbedComponent> tabbedComponent;
    ScopedPointer<SynthesisTab> synthesisTab;
    //ScopedPointer<SampleTab> sampleTab;
    ScopedPointer<WaveTab> waveTab;
    ScopedPointer<AudioTab> audioTab;

    SourceComponent* otherSource = nullptr;
    bool isInverted = false;
    bool isMuted = false;

    // TODO - consider synch to other for sample, wave and audio tabs also

    dsp::Gain<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};