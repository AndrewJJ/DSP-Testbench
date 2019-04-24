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

    /**	Defines typical colours to be used by our application. */
    struct ApplicationColours
    {
        static Colour componentBackground        ()            { return Colour (0xff323e44); }
        static Colour titleMenuBackground        ()            { return Colour (0xff263238); }
        static Colour sourceBackground           ()            { return Colours::darkgrey; }
        static Colour processorBackground        ()            { return Colour (0x300081ff); }
        static Colour meterBackground            ()            { return Colours::black; }
        static Colour audioThumbnailForeground   ()            { return Colours::white; }
        static Colour audioThumbnailBackground   ()            { return sourceBackground(); }
        static Colour audioThumbnailCursor       ()            { return Colours::black; }
        static Colour titleFontColour            ()            { return Colours::white; }
        static Colour normalFontColour           ()            { return Colour (0xffd0d0d0); }
        static Colour cpuMeterBarColour          ()            { return Colour (0xff705090); }
        static Colour benchmarkHeadingBackground ()            { return Colours::black; }
        static Colour benchmarkRow               ()            { return componentBackground().darker(0.2f); }
        static Colour benchmarkAlternateRow      ()            { return Colours::darkgrey; }
    };

    /**	Provides colours for our DrawableButtons. */
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

    /**	Sets up images for a DrawableButton from binary data for a single image. This will replace an original colour in
     *	the image with alternate colours the normal, over, down and disabled images. So naturally this works best with
     *	single colour SVG icons.
     */
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

    /** Translates an image stored as binary resource data into a bitmap then returns it as a MouseCursor (works for SVGs). */
    static MouseCursor getMouseCursorFromImageData (
        const void* rawData,                    /**< Pointer to raw binary data representing the image. */
        const size_t numBytesOfData,            /**< Size of the raw binary data. */
        const Colour originalColour,            /**< Colour in image that you want to replace (e.g. Colours::black). */
        const Colour replacementColour,         /**< Colour that replaces the original (e.g. Colours::white). */
        const int width,                        /**< Width of the cursor image (big images may be scaled down). */
        const int height,                       /**< Height of the cursor image (big images may be scaled down). */
        const int hotspotX,                     /**< X position of mouse cursor hotspot (zero based, so you might want to subract 1). */
        const int hotspotY                      /**< Y position of mouse cursor hotspot (zero based, so you might want to subract 1). */
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
