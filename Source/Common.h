/*
  ==============================================================================

    Common.h
    Created: 11 Jan 2018 10:37:15am
    Author:  Andrew Jerrim

  ==============================================================================
*/

#pragma once

enum Waveform
{
    sine = 1,
    saw,
    square,
    impulse,
    step,
    whiteNoise,
    pinkNoise
};

//String WaveformToString (const Waveform waveForm)
//{
//    switch (waveForm)
//    {
//        case sine: return "Sine"; break;
//        case saw: return "Saw"; break;
//        case Waveform::square: return "Square"; break;
//        case impulse: return "Impulse"; break;
//        case step: return "Step"; break;
//        case whiteNoise: return "White Noise"; break;
//        case pinkNoise: return "Pink Noise"; break;
//    }
//}


struct ParameterBase : public ChangeBroadcaster
{
    ParameterBase (const String& labelName) : name (labelName) {}
    virtual ~ParameterBase() {}

    virtual Component* getComponent() = 0;

    virtual int getPreferredHeight()  = 0;
    virtual int getPreferredWidth()   = 0;

    String name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterBase)
};

class SliderParameter : public ParameterBase, private Slider::Listener
{
public:
    SliderParameter (Range<double> range, double skew, double initialValue, const String& labelName, const String& suffix = {})
        : ParameterBase (labelName)
    {
        slider.setRange (range.getStart(), range.getEnd(), 0.01);
        slider.setSkewFactor (skew);
        slider.setValue (initialValue);

        if (suffix.isNotEmpty())
            slider.setTextValueSuffix (suffix);

        slider.addListener (this);
    }

    Component* getComponent() override    { return &slider; }

    int getPreferredHeight() override     { return 40; }
    int getPreferredWidth()  override     { return 500; }

    double getCurrentValue() const        { return slider.getValue(); }

private:
    Slider slider;

    void sliderValueChanged (Slider*) override    { sendChangeMessage(); }
};

class ChoiceParameter : public ParameterBase, private ComboBox::Listener
{
public:
    ChoiceParameter (const StringArray& options, int initialId, const String& labelName)
        : ParameterBase (labelName)
    {
        parameterBox.addItemList (options, 1);
        parameterBox.addListener (this);

        parameterBox.setSelectedId (initialId);
    }

    Component* getComponent() override    { return &parameterBox; }

    int getPreferredHeight() override     { return 25; }
    int getPreferredWidth()  override     { return 250; }

    int getCurrentSelectedID() const      { return parameterBox.getSelectedId(); }

private:
    ComboBox parameterBox;

    void comboBoxChanged (ComboBox*) override    { sendChangeMessage(); }
};

class ToggleParameter : public ParameterBase, private TextButton::Listener
{
public:
    ToggleParameter (bool initialValue, const String& labelName)
        : ParameterBase (labelName)
    {
        button.setToggleState (initialValue, sendNotificationAsync);
        button.addListener (this);
    }

    Component* getComponent() override    { return &button; }

    int getPreferredHeight() override     { return 40; }
    int getPreferredWidth()  override     { return 500; }

    double getCurrentValue() const        { return button.getToggleState(); }

private:
    TextButton button;

    void buttonClicked(Button*) override { sendChangeMessage(); }
};

// This is a base class for signal sources. It exposes them as an AudioSource with an array of parameters.
struct SignalSource : public AudioSource
{
    // TODO - why the "&"? is it really necessary?
    virtual const std::vector<ParameterBase*>& getParameters() = 0;
    
    // TODO - the following would only be required for audio input
    //AudioSource* inputSource = nullptr;
};
