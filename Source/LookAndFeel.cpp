/*
  ==============================================================================

    GuiHelpers.cpp
    Created: 10 Mar 2019 11:51:59am
    Author:  Andrew

  ==============================================================================
*/

#include "LookAndFeel.h"

void DspTestBenchLnF::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
	const auto radius = static_cast<float> (jmin (width, height)) * 0.48f;
	const auto centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
	const auto centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const auto rx = centreX - radius;
    const auto ry = centreY - radius;
    const auto rw = radius * 2.0f;
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
	const auto isEnabled = slider.isEnabled();
    const auto isMouseOver = slider.isMouseOverOrDragging() && isEnabled;

	const auto fillColour = isEnabled ? Colours::black : Colours::black.brighter();
	const auto outlineColour = isEnabled ? (isMouseOver ? Colours::white.withAlpha (0.7f) : Colours::black) : Colours::grey.darker (0.6f);
    const auto indicatorColour = isEnabled ? Colours::white : Colours::grey;

	// Draw knob body
	{
		Path  p;
		p.addEllipse (rx, ry, rw, rw);
        g.setColour (fillColour);
		g.fillPath (p);
		g.setColour (outlineColour);
		g.strokePath (p, PathStrokeType (radius * 0.075f));
	}

	// Draw rotating pointer
	{
		Path l;
		l.startNewSubPath (0.0f, radius * -0.95f);
		l.lineTo (0.0f, radius * -0.50f);
		g.setColour (indicatorColour);
		g.strokePath (l, PathStrokeType (2.5f), AffineTransform::rotation (angle).translated (centreX, centreY));
	}
}

int DspTestBenchLnF::getDefaultMenuBarHeight()
{
    return GUI_SIZE_I (titleMenuScalingFactor) + GUI_GAP_I (2.0 * titleMenuScalingFactor);
}

Font DspTestBenchLnF::getTitleFont() const
{
    return Font (GUI_SIZE_F (titleMenuScalingFactor));
}

void DspTestBenchLnF::drawDrawableButton (Graphics& g, DrawableButton& button,
                                         bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    const auto toggleState = button.getToggleState();

    g.fillAll (button.findColour (DrawableButton::backgroundColourId));

    const auto textH = (button.getStyle() == DrawableButton::ImageAboveTextLabel)
                        ? jmin (16, button.proportionOfHeight (0.25f)) : 0;

    if (textH > 0)
    {
        g.setFont (static_cast<float> (textH));
        g.setColour (button.findColour (toggleState ? DrawableButton::textColourOnId : DrawableButton::textColourId)
                        .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.4f));
        g.drawFittedText (button.getButtonText(),
                          2, button.getHeight() - textH - 1,
                          button.getWidth() - 4, textH,
                          Justification::centred, 1);
    }
}

void DspTestBenchLnF::drawStretchableLayoutResizerBar (Graphics& g, int w, int h, bool, bool isMouseOver, bool isMouseDragging)
{
	const auto fillColour = (isMouseOver || isMouseDragging) ? Colour (0xff705090).darker (0.15f) : Colour (0xff705090);
	g.fillAll (fillColour);
	drawDots (g, juce::Rectangle<int> (0, 0, w, h), fillColour.darker (), 4);
}

void DspTestBenchLnF::setImagesForDrawableButton(DrawableButton * button, const void * imageData, const size_t imageDataSize, const Colour original)
{
    OwnedArray<Drawable> drawableArray;
    drawableArray.add (Drawable::createFromImageData (imageData, imageDataSize));
    for (auto i = 0; i < 3; i++)
        drawableArray.add (drawableArray[0]->createCopy ());
    
    using cols = DrawableButtonColours;
    drawableArray[0]->replaceColour (original, cols::normal());
    drawableArray[1]->replaceColour (original, cols::over());
    drawableArray[2]->replaceColour (original, cols::down());
    drawableArray[3]->replaceColour (original, cols::disabled());
    
    button->setImages (drawableArray[0], drawableArray[1], drawableArray[2], drawableArray[3]);
}

void DspTestBenchLnF::setImagesForDrawableButton(DrawableButton * button, const void * imageData, const size_t imageDataSize, const Colour original, const Colour toggleOnColour)
{
    OwnedArray<Drawable> drawableArray;
    drawableArray.add (Drawable::createFromImageData (imageData, imageDataSize));
    for (auto i = 0; i < 7; i++)
        drawableArray.add (drawableArray[0]->createCopy ());
    
    using cols = DrawableButtonColours;
    drawableArray[0]->replaceColour (original, cols::normal());
    drawableArray[1]->replaceColour (original, cols::over());
    drawableArray[2]->replaceColour (original, cols::down());
    drawableArray[3]->replaceColour (original, cols::disabled());
    drawableArray[4]->replaceColour (original, cols::normalOn (toggleOnColour));
    drawableArray[5]->replaceColour (original, cols::overOn (toggleOnColour));
    drawableArray[6]->replaceColour (original, cols::downOn (toggleOnColour));
    drawableArray[7]->replaceColour (original, cols::disabledOn (toggleOnColour));
    
    button->setImages (drawableArray[0], drawableArray[1], drawableArray[2], drawableArray[3], drawableArray[4], drawableArray[5], drawableArray[6], drawableArray[7]);
}

void DspTestBenchLnF::drawDots (Graphics& g, const juce::Rectangle<int> bounds, const Colour dotColour, const int numDots) const
{
	auto b = bounds.toFloat();
	const auto marginRatio = 0.2f;
	const auto margin = marginRatio * b.getHeight();
	const auto dotSize = b.getHeight() * (1.0f - marginRatio);
	const auto adjNumDots = jmin (numDots, bounds.getWidth() / static_cast<int> (dotSize + margin));
	b = b.withSizeKeepingCentre (dotSize * adjNumDots + margin * static_cast<float> (adjNumDots - 1), dotSize);
	b.setWidth (dotSize);
	for (auto i = 0; i < adjNumDots; ++i)
	{
		if (i>0)
			b.translate (dotSize + margin, 0.0f);

		// Draw shadow
		g.setColour (Colours::white.withAlpha (0.4f));
		g.fillEllipse (b);

		// Draw fill
		g.setColour (dotColour);
		const auto reduction = margin * 0.2f;
		g.fillEllipse (b.reduced (reduction).translated (-reduction, -reduction));
	}
}