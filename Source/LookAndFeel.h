/*
  ==============================================================================

    GuiHelpers.h
    Created: 10 Mar 2019 11:51:59am
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class DspTestBenchLnF final : public LookAndFeel_V4
{
public:
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;
    
    int getDefaultMenuBarHeight() override;
    
    Font getTitleFont() const;

    void drawDrawableButton (Graphics&, DrawableButton&,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
        
    /** Draws a StretchableLayoutResizerBar. */
	void drawStretchableLayoutResizerBar (Graphics&, int w, int h, bool isVerticalBar, bool isMouseOver, bool isMouseDragging) override;

    /**	Sets up images for a DrawableButton from binary data for a single image. This will replace an original colour in
     *	the image with alternate colours the normal, over, down and disabled images. So naturally this works best with
     *	single colour SVG icons.
     */
    struct DrawableButtonColours
    {
        static Colour normal     ()                            { return Colour (0xFFE0E0E0); }
        static Colour over       ()                            { return Colours::white; }
        static Colour disabled   ()                            { return Colours::darkgrey; }
        static Colour down       ()                            { return Colour (0xFFD090B0); }
        static Colour normalOn   (const Colour normalOn)       { return normalOn; }
        static Colour overOn     (const Colour normalOn)       { return normalOn.darker (0.2f); }
        static Colour downOn     (const Colour normalOn)       { return normalOn.brighter (0.3f).withRotatedHue (0.08f); }
        static Colour disabledOn (const Colour /*normalOn*/)   { return disabled(); }
    };
    static void setImagesForDrawableButton (
        DrawableButton* button,                 /**< DrawableButton to setup. */
        const void* imageData,			    	/**< Binary image data (typically SVG, but can be anything that juce::ImageFileFormat understands. */
        const size_t imageDataSize, 			/**< Size of the binary image data (in bytes). */
        const Colour original                   /**< Colour to be replaced in the original image. */
    );
    static void setImagesForDrawableButton (
        DrawableButton* button,                 /**< DrawableButton to setup. */
        const void* imageData,			    	/**< Binary image data (typically SVG, but can be anything that juce::ImageFileFormat understands. */
        const size_t imageDataSize, 			/**< Size of the binary image data (in bytes). */
        const Colour original,                  /**< Colour to be replaced in the original image. */
        const Colour toggleOnColour             /**< Set base colour for "on" state for toggle button. */
    );

private:

    /** Draws a set of dots in a horizontal line. */
    void drawDots ( Graphics& g,						/**< Graphics context on which to draw. */
				    juce::Rectangle<int> bounds,		/**< Bounds of the item. */
					const Colour dotColour,		    	/**< Colour for the dot. */
					const int numDots					/**< Number of dots to draw. */
				  ) const;

    const double titleMenuScalingFactor = 0.75;
};
