/*
  ==============================================================================

    SimpleLevelMeterComponent.cpp
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "SimpleLevelMeterComponent.h"

SimpleLevelMeterComponent::SimpleLevelMeterComponent()
{
    SimpleLevelMeterComponent (-100.0f, -15.0f, -3.0f, 0.0f);
}
SimpleLevelMeterComponent::SimpleLevelMeterComponent (const float minimumLevelDb, const float cautionLevelDb,
                                                      const float alertLevelDb, const float maximumLevelDb)
{
    setColour (meterBarNormalColourId,	Colour (0xff00ff00)); auto x = Colours::greenyellow;
	setColour (meterBarCautionColourId,	Colours::yellow);
	setColour (meterBarAlertColourId,	Colour (0xffff0000));
    
    minDb       = minimumLevelDb;
    cautionDb   = cautionLevelDb;
    alertDb     = alertLevelDb;
    maxDb       = maximumLevelDb;
    rangeDb     = maximumLevelDb - minimumLevelDb;
}
SimpleLevelMeterComponent::~SimpleLevelMeterComponent()
{
}
void SimpleLevelMeterComponent::paint (Graphics& g)
{
    // Paints a vertical meter bar
	// TODO - optimise with precalculated values
	const auto fullHeight = static_cast<float> (getHeight());
	const auto gradientHeight = fullHeight * 0.5f;
	const auto fullWidth = static_cast<float> (getWidth());
	const auto darkShadow = 0.2f;

	auto h = jmin (currentLevelY, cautionY);
	//g.setColour (findColour (meterBarNormalColourId));
	g.setGradientFill (	ColourGradient	(	findColour (meterBarNormalColourId), 0.0f, gradientHeight,
											findColour (meterBarNormalColourId).darker (darkShadow), fullWidth, gradientHeight,
											false) );
	g.fillRect(0.0f, fullHeight - h, fullWidth, h);
	
	if (currentLevelY > cautionY)
	{
		h = jmin (currentLevelY, alertY) - cautionY;
		//g.setColour (findColour (meterBarCautionColourId));
		g.setGradientFill (	ColourGradient	(	findColour(meterBarCautionColourId), 0.0f, gradientHeight,
												findColour(meterBarCautionColourId).darker(darkShadow), fullWidth, gradientHeight,
												false) );
		g.fillRect (0.0f, fullHeight - cautionY - h, fullWidth, h);
	}

	if (currentLevelY > alertY)
	{
		h = currentLevelY - alertY;
		//g.setColour (findColour (meterBarAlertColourId));
		g.setGradientFill (	ColourGradient	(	findColour (meterBarAlertColourId), 0.0f, gradientHeight,
												findColour (meterBarAlertColourId).darker (darkShadow), fullWidth, gradientHeight,
												false) );
		g.fillRect (0.0f, fullHeight - currentLevelY, fullWidth, h);
	}
}
void SimpleLevelMeterComponent::resized ()
{
    conversionFactor    = rangeDb / static_cast<float> (getHeight());
    cautionY            = dBtoPx (cautionDb);
    alertY              = dBtoPx (alertDb);
}
void SimpleLevelMeterComponent::setLevel (const float dB)
{
    currentLevelY = dBtoPx (dB);
    repaint();
}
float SimpleLevelMeterComponent::dBtoPx (const float dB) const
{
    return (dB - minDb) * conversionFactor;
}
