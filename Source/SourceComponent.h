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
//#include "Main.h"
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
    Wrap= 1,
    Reverse
};

class SynthesisTab : public Component, public dsp::ProcessorBase, public Timer, public Slider::Listener
{
public:
    SynthesisTab();
    ~SynthesisTab();

    void paint (Graphics& g) override;
    void resized() override;

    void performSynch();
    void setOtherSource (SourceComponent* otherSourceComponent);
    void syncAndResetOscillator (const Waveform waveform, const double freq,
                                 const double sweepStart, const double sweepEnd,                                 
                                 const double sweepDuration, SweepMode sweepMode, const bool sweepEnabled);

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

    SourceComponent* otherSource;
    CriticalSection synthesiserCriticalSection;
    Waveform currentWaveform;
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

    dsp::PolyBlepOscillator<float> oscillators[4] =
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

class SampleTab : public Component, public dsp::ProcessorBase
{
public:
    SampleTab();
    ~SampleTab();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

private:

    void selectedSampleChanged();
    void loopEnablementToggled();

    ScopedPointer<ComboBox> cmbSample;
    ScopedPointer<TextButton> btnLoopEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTab)
};

class WaveTab : public Component, public dsp::ProcessorBase, public ChangeListener
{
public:
    WaveTab();
    ~WaveTab();

    void paint (Graphics& g) override;
    void resized() override;

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
        AudioThumbnailComponent();
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
    ScopedPointer<AudioThumbnailComponent> audioThumbnailComponent;
    ScopedPointer<TextButton> btnLoad;
    ScopedPointer<TextButton> btnPlay;
    ScopedPointer<TextButton> btnStop;
    ScopedPointer<TextButton> btnLoop;

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

class AudioTab : public Component, public dsp::ProcessorBase
{
public:
    AudioTab();
    ~AudioTab();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    class ChannelComponent : public Component, public Timer, public Slider::Listener
    {
    public:
        ChannelComponent (SimpleLevelMeterProcessor* meterProcessorToQuery, size_t channelIndex);;
        ~ChannelComponent();

        void paint (Graphics& g) override;
        void resized() override;
        void timerCallback () override;
        void sliderValueChanged (Slider* slider) override;

        // Should be called every time prepare is called on the parent object
        void setNumOutputChannels (const size_t numberOfOutputChannels);
        void getSelectedOutputs(); // TODO - figure out how to do this
        void reset();
        double getGain();

    private:
        
        ScopedPointer<SimpleLevelMeterComponent> meterBar;
        ScopedPointer<Slider> sldGain;
        ScopedPointer<GroupComponent> grpOutputs;
        OwnedArray<ToggleButton> toggleButtons;

        SimpleLevelMeterProcessor* meterProcessor;
        size_t channel = 0;
        String label;
        float currentLinearGain = 0.0f;
        size_t numOutputs = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelComponent)
    };

private:

    SimpleLevelMeterProcessor meterProcessor;
    OwnedArray <ChannelComponent> channelComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTab)
};

//==============================================================================

class SourceComponent  : public Component, public Slider::Listener, public dsp::ProcessorBase
{
public:

    enum Mode
    {
        Synthesis = 0,
        Sample,
        WaveFile,
        AudioIn
    };

    SourceComponent (String sourceId);
    ~SourceComponent();

    void paint (Graphics& g) override;
    void resized() override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    Mode getMode() const;
    void setOtherSource (SourceComponent* otherSourceComponent);
    SynthesisTab* getSynthesisTab();
    void mute();

    private:

    ScopedPointer<Label> lblTitle;
    ScopedPointer<Slider> sldGain;
    ScopedPointer<TextButton> btnInvert;
    ScopedPointer<TextButton> btnMute;
    ScopedPointer<TabbedComponent> tabbedComponent;
    ScopedPointer<SynthesisTab> synthesisTab;
    ScopedPointer<SampleTab> sampleTab;
    ScopedPointer<WaveTab> waveTab;
    ScopedPointer<AudioTab> audioTab;

    SourceComponent* otherSource;
    bool isInverted = false;
    bool isMuted = false;

    // TODO - consider synch to other for sample, wave and audio tabs also

    dsp::Gain<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceComponent)
};