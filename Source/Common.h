/*
  ==============================================================================

    Common.h
    Created: 11 Jan 2018 10:37:15am
    Author:  Andrew

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
