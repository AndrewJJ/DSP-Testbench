/*
  ==============================================================================

    MonitoringComponent.cpp
    Created: 10 Jan 2018
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "MonitoringComponent.h"
#include "Main.h"

MonitoringComponent::MonitoringComponent (AudioDeviceManager* audioDeviceManager)
    : deviceManager (audioDeviceManager),
    keyName ("Monitoring")
{
    // Read configuration from application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    config.reset (propertiesFile->getXmlValue (keyName));
    if (!config)
    {
        // Define default properties to be used if user settings not already saved
        config.reset(new XmlElement (keyName));
        config->setAttribute ("OutputGain", 0.0);
        config->setAttribute ("OutputLimiter", true);
        config->setAttribute ("OutputMute", false);
    }

    gain.setRampDurationSeconds (0.01);

    addAndMakeVisible (lblTitle);
    lblTitle.setText (TRANS("Monitoring"), dontSendNotification);
    lblTitle.setFont (Font (GUI_SIZE_F(0.7), Font::bold));
    lblTitle.setJustificationType (Justification::topLeft);
    lblTitle.setEditable (false, false, false);
    lblTitle.setColour (TextEditor::textColourId, Colours::black);
    lblTitle.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sldGain);
    sldGain.setTooltip (TRANS("Allows gain adjustment of the output to your audio device"));
    sldGain.setRange (-100, 0, 0.1);
    sldGain.setDoubleClickReturnValue (true, 0.0);
    sldGain.setSliderStyle (Slider::LinearHorizontal);
    sldGain.setTextBoxStyle (Slider::TextBoxRight, false, GUI_SIZE_I(2.5), GUI_SIZE_I(0.7));
    sldGain.setValue (config->getDoubleAttribute ("OutputGain"));
    sldGain.onValueChange = [this] { gain.setGainDecibels (static_cast<float> (sldGain.getValue())); };

    addAndMakeVisible (btnLimiter);
    btnLimiter.setTooltip (TRANS("Activate limiter on output"));
    btnLimiter.setButtonText (TRANS("Limiter"));
    btnLimiter.setClickingTogglesState (true);
    btnLimiter.setColour (TextButton::buttonOnColourId, Colours::darkorange);
    statusLimiter = config->getBoolAttribute ("OutputLimiter");
    btnLimiter.setToggleState (statusLimiter, dontSendNotification);
    btnLimiter.onClick = [this] { statusLimiter = btnLimiter.getToggleState(); };
    
    addAndMakeVisible (btnMute);
    btnMute.setButtonText ("Mute");
    btnMute.setClickingTogglesState (true);
    btnMute.setColour (TextButton::buttonOnColourId, Colours::darkred);
    statusMute = config->getBoolAttribute ("OutputMute");
    btnMute.setToggleState (statusMute, dontSendNotification);
    btnMute.onClick = [this] { statusMute = btnMute.getToggleState(); };
}
MonitoringComponent::~MonitoringComponent()
{
    // Update configuration from class state
    config->setAttribute ("OutputGain", sldGain.getValue());
    config->setAttribute ("OutputLimiter", statusLimiter);
    config->setAttribute ("OutputMute", statusMute);

    // Save configuration to application properties
    auto* propertiesFile = DSPTestbenchApplication::getApp().appProperties.getUserSettings();
    propertiesFile->setValue (keyName, config.get());
    propertiesFile->saveIfNeeded();
}
void MonitoringComponent::paint (Graphics& g)
{
    g.setColour (Colours::darkgrey);
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), GUI_GAP_F(2));
}
void MonitoringComponent::resized()
{
    Grid grid;
    grid.rowGap = GUI_BASE_GAP_PX;
    grid.columnGap = GUI_BASE_GAP_PX;

    using Track = Grid::TrackInfo;

    grid.templateRows = {   Track (GUI_BASE_SIZE_PX)
                        };

    grid.templateColumns = { Track (GUI_SIZE_PX(4)), Track (1_fr), Track (GUI_SIZE_PX(2.0)), Track (GUI_SIZE_PX(1.7)) };

    grid.autoFlow = Grid::AutoFlow::row;

    grid.items.addArray({   GridItem (lblTitle),
                            GridItem (sldGain).withMargin (GridItem::Margin (0.0f, GUI_GAP_F(3), 0.0f, 0.0f)),
                            GridItem (btnLimiter),
                            GridItem (btnMute)
                        });

    grid.performLayout (getLocalBounds().reduced (GUI_GAP_I(2), GUI_GAP_I(2)));
}
float MonitoringComponent::getMinimumWidth()
{
    // This is an arbitrary minimum that should look OK
    return 400.0f;
}
float MonitoringComponent::getMinimumHeight()
{
    // This is an exact calculation of the height we want
    const auto innerMargin = GUI_GAP_F(4);
    const auto totalItemHeight = GUI_BASE_SIZE_F;
    return innerMargin + totalItemHeight;
}
void MonitoringComponent::prepare (const dsp::ProcessSpec& spec)
{
    gain.prepare (spec);
    gain.setGainDecibels (static_cast<float> (sldGain.getValue()));
}
void MonitoringComponent::process (const dsp::ProcessContextReplacing<float>& context)
{
    dsp::AudioBlock<float> inputBlock;

    if (!isMuted()) // Probably not necessary, because main component doesn't call MonitoringComponent::process if monitoring is muted anyway!
    {
        // Apply gain
        gain.process (context);

        if (isLimited())
        {
            // TODO - implement limiting function
        }
    }
    else
        context.getOutputBlock().clear();
}
void MonitoringComponent::reset ()
{
    gain.reset();
}
bool MonitoringComponent::isMuted() const
{
    // For safe audio processing, we use local variable rather than accessing button toggle state
    return statusMute;
}
bool MonitoringComponent::isLimited() const
{
    // For safe audio processing, we use local variable rather than accessing button toggle state
    return statusLimiter;
}
