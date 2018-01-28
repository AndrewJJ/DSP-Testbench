/*
  ==============================================================================

    AnalyserComponent.h
    Created: 11 Jan 2018 4:37:59pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Analysis.h"
#include "FastApproximations.h"

class FftScope : public Component, public AudioProbe <FftProcessor <12>::FftFrame>::Listener
{
public:

    FftScope ();
    ~FftScope ();

    void paint (Graphics& g) override;
    void resized() override;
    void mouseMove(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;

    void assignFftMult (FftProcessor<12>* fftMultPtr);
    void audioProbeUpdated (AudioProbe<FftProcessor<12>::FftFrame>* audioProbe) override;
    void prepare (const dsp::ProcessSpec& spec);

private:
    
    class Background : public Component
    {
    public:
        Background (FftScope* parentFftScope);
        void paint (Graphics& g) override;
    private:
        FftScope* parentScope;
    };

    class Foreground : public Component
    {
    public:
        explicit Foreground (FftScope* parentFftScope);
        void paint (Graphics& g) override;
    private:
        FftScope* parentScope;
    };

    void paintFft (Graphics& g) const;
    void paintFftScale (Graphics& g) const;

    inline float convertToDbVFromLinear (const float linear) const;
    inline float convertToPxFromLinear (const float linear) const;
    inline float convertToPxFromDbV (const float dB) const;
    inline float convertToDbVFromPx (const float yInPixels) const;
    inline float convertToHzFromPx (const float xInPixels) const;
    inline float convertToPxFromHz (const float xInHz) const;

    String hertzToString (const double frequencyInHz, const int numDecimals, const bool appendHz, const bool includeSpace) const;
    static Colour getColourForChannel (const int channel);
    void initialise();

    Background background;
    Foreground foreground;
	FftProcessor<12>* fftProcessor;
    HeapBlock<float> x, y;
	double samplingFreq = 48000; // will be set correctly in prepare()
    float dBmax = 0.0f;         // TODO - make max dB a property
    float dBmin = -80.0f;       // TODO - make min dB a property
    float minFreq = 10.0f;      // TODO - make min frequency a property
    float maxFreq = 24000.0f;   // TODO - make the max frequency a property (limited to nyquist)
    float minLogFreq = 0.0f;
    float logFreqSpan = 0.0f;
    float xRatio = 1.0f;
    float yRatio = 1.0f;
    int currentX = -1;
    int currentY = -1;
    
    // Candidate frequencies for drawing the grid on the background
    Array<float> gridFrequencies = { 20.0f, 50.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f, 32000.0f, 64000.0f };
};

class AnalyserComponent  :  public Component, public dsp::ProcessorBase
{
public:

    AnalyserComponent();
    ~AnalyserComponent();

    void paint (Graphics& g) override;
    void resized() override;

    void prepare (const dsp::ProcessSpec& spec) override;
    void process (const dsp::ProcessContextReplacing<float>& context) override;
    void reset () override;

    bool isActive() const noexcept;

    // TODO - add waveform display
    // TODO - add detailed metering (peak, RMS, VU)
    // TODO - add phase scope

private:

    Label lblTitle;
    TextButton btnDisable;

    FftProcessor<12> fftMult;
    FftScope fftScope;

    Atomic<bool> statusActive = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyserComponent)
};