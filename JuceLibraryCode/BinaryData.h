/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   about_svg;
    const int            about_svgSize = 1108;

    extern const char*   audio_settings_svg;
    const int            audio_settings_svgSize = 2610;

    extern const char*   configure_svg;
    const int            configure_svgSize = 1998;

    extern const char*   dashboard_gauge_svg;
    const int            dashboard_gauge_svgSize = 1781;

    extern const char*   expand_svg;
    const int            expand_svgSize = 1044;

    extern const char*   mute_svg;
    const int            mute_svgSize = 1263;

    extern const char*   pause_svg;
    const int            pause_svgSize = 811;

    extern const char*   play_svg;
    const int            play_svgSize = 797;

    extern const char*   phase_invert_svg;
    const int            phase_invert_svgSize = 1297;

    extern const char*   screenshot_svg;
    const int            screenshot_svgSize = 2475;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 10;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
