/*
  ==============================================================================

    SimpleLevelMeterComponent.cpp
    Created: 18 Jan 2018 4:40:20pm
    Author:  Andrew Jerrim

  ==============================================================================
*/

#include "SimpleLevelMeterComponent.h"

SimplePeakMeterComponent::SimplePeakMeterComponent()
{
    this->setPaintingIsUnclipped(true);
}

SimplePeakMeterComponent::SimplePeakMeterComponent (const float minimumLevelDb, const float cautionLevelDb,
                                                      const float alertLevelDb, const float maximumLevelDb)
{
    minDb       = minimumLevelDb;
    cautionDb   = cautionLevelDb;
    alertDb     = alertLevelDb;
    maxDb       = maximumLevelDb;
    rangeDb     = maximumLevelDb - minimumLevelDb;
}

SimplePeakMeterComponent::~SimplePeakMeterComponent()
= default;

void SimplePeakMeterComponent::paint (Graphics& g)
{
    auto colourNormal  = Colour (0xff00ff00);
	auto colourCaution = Colours::yellow;
	auto colourAlert   = Colour (0xffff0000);

    const auto fullHeight = static_cast<float> (getHeight());
	const auto gradientHeight = fullHeight * 0.5f;
	const auto fullWidth = static_cast<float> (getWidth());
	const auto darkShadow = 0.5f;

	auto h = jmin (currentLevelY, cautionY);
	//g.setColour (colourNormal);
	g.setGradientFill (	ColourGradient	(	colourNormal, 0.0f, gradientHeight,
											colourNormal.darker (darkShadow), fullWidth, gradientHeight,
											false) );
	g.fillRect(0.0f, fullHeight - h, fullWidth, h);
	
	if (currentLevelY > cautionY)
	{
		h = jmin (currentLevelY, alertY) - cautionY;
		//g.setColour (colourCaution);
		g.setGradientFill (	ColourGradient	(	colourCaution, 0.0f, gradientHeight,
												colourCaution.darker(darkShadow), fullWidth, gradientHeight,
												false) );
		g.fillRect (0.0f, fullHeight - cautionY - h, fullWidth, h);
	}

	if (currentLevelY > alertY)
	{
		h = currentLevelY - alertY;
		//g.setColour (colourAlert);
		g.setGradientFill (	ColourGradient	(	colourAlert, 0.0f, gradientHeight,
												colourAlert.darker (darkShadow), fullWidth, gradientHeight,
												false) );
		g.fillRect (0.0f, fullHeight - currentLevelY, fullWidth, h);
	}
}
void SimplePeakMeterComponent::resized ()
{
    rangeDb = maxDb - minDb;
    conversionFactor    = static_cast<float> (getHeight()) / rangeDb;
    currentLevelY       = dBtoPx (currentLevelDb);
    cautionY            = dBtoPx (cautionDb);
    alertY              = dBtoPx (alertDb);
}
void SimplePeakMeterComponent::setLevel (const float dB)
{
    currentLevelDb = dB;
    currentLevelY = dBtoPx (dB);
    repaint();
}
void SimplePeakMeterComponent::setMinDb (const float dB)
{
    minDb = dB;
    resized();
}
void SimplePeakMeterComponent::setMaxDb (const float dB)
{
    maxDb = dB;
    resized();
}
float SimplePeakMeterComponent::dBtoPx (const float dB) const
{
    if (dB <= minDb)
        return 0.0f;
    else
        return (jlimit (minDb, maxDb, dB) - minDb) * conversionFactor;
}
