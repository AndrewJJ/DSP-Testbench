/*
  ==============================================================================

    NoiseGenerators.h
    Created: 15 Jan 2018 3:28:15pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace juce {
namespace dsp {

//////////////////////////////////////////////////////////////////////////////
//
//
// rand31pmc
//
// Robin Whittle  2005 September 20
//
// 31 bit pseudo-random number generator based on:
//
//   Lehmer (1951)
//   Lewis, Goodman & Miller (1969)
//   Park & Miller (1983)
//   
// implemented according to the optimisation suggested by David G. Carta
// in 1990 which uses 32 bit math and does not require division.  
// Park and Miller rejected Carta's approach in 1993.  Carta provided no 
// code examples.  Carta's approach produces identical results to Park 
// and Miller's code.
//
// Copyright public domain . . . *but*:
//
// * Please leave the comments intact so inquiring minds have a chance of 
// * understanding how this implementation works and chasing the 
// * references to see the strengths and limitations of this particular 
// * pseudo-random number generator.
//
// Output is a 31 bit unsigned integer.  The range of values output is
// 1 to 2,147,483,646 and the seed must be in this range too.  The
// output sequence repeats in a loop of this length = (2^31 - 2).
//
// The output stream has some predictable patterns.  For instance, after 
// a very low output, the next one or two outputs will be relatively low 
// (compared to the 2 billion range) because the multiplier is only 16,807.
// Linear congruential generators are not suitable for cryptography or 
// simulation work (such as Monte Carlo Method), but they are probably 
// fine for many uses where the output is sound or vision for human 
// perception.  
//
// The particular generator implemented here:
//
//   New-value = (old-value * 16807) mod 0x7FFFFFFF 
//
// is probably the best studied linear congruentual PRNG.  It is not the very
// best, but it is far from the worst.
//
// For the background on this implementation, and the Park Miller
// "Minimal Standard" linear congruential PRNG, please see:
//
//    http://www.firstpr.com.au/dsp/rand31/  
//
//    Stephen K. Park and Keith W. Miller 
//    Random Number Generators: Good Ones are Hard to Find
//    Communications of the ACM, Oct 1988, Vol 31 Number 10 1192-1201
//
//    David G. Carta
//    Two Fast Implementations of the "Minimal Standard" Random Number Generator
//    Communications of the ACM, Jan 1990, Vol 33 Number 1 87-88
//
//    George Marsaglia; Stephen J. Sullivan; Stephen K. Park, Keith W. Miller, 
//    Paul K. Stockmeyer
//    Remarks on Choosing and Implementing Random Number Generators 
//    Communications of the ACM, Jul 1993, Vol 36 Number 7 105-110
//
//    http://random.mat.sbg.ac.at has lots of material on PRNG quality. 
//
//
// The sequence of values this PRNG should produce includes:
// 
//      Result     Number of results after seed of 1
//
//       16807          1
//   282475249          2
//  1622650073          3
//   984943658          4
//  1144108930          5
//   470211272          6
//   101027544          7
//  1457850878          8
//  1458777923          9
//  2007237709         10
//
//   925166085       9998
//  1484786315       9999
//  1043618065      10000
//  1589873406      10001
//  2010798668      10002
//
//  1227283347    1000000
//  1808217256    2000000
//  1140279430    3000000
//   851767375    4000000
//  1885818104    5000000
//
//   168075678   99000000
//  1209575029  100000000
//   941596188  101000000
//
//  1207672015 2147483643
//  1475608308 2147483644
//  1407677000 2147483645
//           1 2147483646
//       16807 2147483647
//
// Carta refers to two registers p (15 bits) and q (31 bits) which
// together hold the 46 bit multiplication product:
//
//         |                   |                   |                   |
//          4444 4444 3333 3333 3322 2222 2222 1111 1111 11            
//          7654 3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210
//
//   q 31                        qqq qqqq qqqq qqqq qqqq qqqq qqqq qqqq
//   p 15     pp pppp pppp pppp p
//
// The maximum 46 bit result occurs 
// when the seed is at its highest
// allowable value: 0x7FFFFFFE.  
//
//    0x20D37FFF7CB2     
//
// which splits up like this    
//
//   q 31                        111 1111 1111 1111 0111 1100 1011 0010
//   p 15     10 0000 1101 0011 0
//          =  100 0001 1010 0110 
//
// In hex, these maxiumum values are:
//
//   q 31     7FFF7CB2  = 2^31 - (2 * 16807)
//   p 15         41A6  = 16807 - 1
//
//
// The task is to combine the two partial products p and q as if they were
// both parts of a 46 bit number, with the final result being modulo:
//
//                              0111 1111 1111 1111 1111 1111 1111 1111
//
// when we are actually only doing 32 bits at a time.  
//
// Here I explain David G. Carta's trick - in a different and much simpler 
// way than he does.
//
// We need to deal with the p bits "pp pppp pppp pppp p" shown above.  
// These bits carry weights of bits 45 to 31 in the multiplication product 
// of the usual Park Miller algorithm.
// 
// David Carta writes that in order to calculate mod(0x7FFFFFFF) of the
// complete multiplication product (taking into account the total value
// of p and q) we should simply add the bits of p into the bit positions 
// 14 to 0 of q and then do a mod(0x7FFFFFFF) on the result!  
//
//         |                   |                   |                   |
//          4444 4444 3333 3333 3322 2222 2222 1111 1111 11            
//          7654 3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210
//
//     31                        qqq qqqq qqqq qqqq qqqq qqqq qqqq qqqq
//     15                   +                        ppp pppp pppp pppp
//                          =   Cxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//                               
// Highest possible value,
// for q, with a value for
// p which would allow it:
//        
//                   7FFFFFFF    111 1111 1111 1111 1111 1111 1111 1111
//                +      41A5                        100 0001 1010 0101
//                = 8000041A4   1000 0000 0000 0000 0100 0001 1010 0100
//
// The result can't be larger than 2 * 0x7FFFFFFF = 0xFFFFFFFE.  So when we 
// do the modulus operation, we will have to subtract either nothing or just
// one 0x7FFFFFFF.  With this model of addition, the subtraction only 
// occurs very rarely.
//
// David Carta's explanation for why this produces the correct answer is too 
// long to repeat here.  Mine is easy to understand. 
//
// Lets define some labels:
//
//  Q = 31 bits 30 to 0. 
//  P = 15 bits 14 to 0.  
//
// If we were doing 46 bit math, the multiplication product (seed * 16807) 
// would be: 
//
//     Q
//  + (P * 0x80000000) 
//
// Observe that this is the same as:
//
//     Q
//  + (P * 0x7FFFFFFF) 
//  + (P * 0x00000001) 
//                                          
// However, we don't need or want a 46 bit result.  We only want that result
// mod(0x7FFFFFFF).  Therfore we can ignore the middle line above and use for 
// our result:
//  
//    Q
//  + P 
//
// This is a lot snappier than using a division, as the Schrage technique 
// requires.
//
// Methods:
//    
//    seedi    Set seed with a 31 bit unsigned integer between 1 and 
//             0x7FFFFFFE inclusive.  Don't use 0!
//
//    ranlui   Provides the next pseudorandom number as a long unsigned 
//             integer (31 bits).
//
//    ranf     Provides the next pseudorandom number as a float between
//             nearly 0 and nearly 1.0.
//
class rand31dc {
    
    // The sole item of state - a 32 bit integer.
    long unsigned int seed31;   

public:
    
    // Constructor sets seed31 to 1, in case  no seedi operation is used.
    rand31dc() {seed31 = 1;}                                    
                                    
    // Set the seed from a long unsigned integer.  If zero is used, then the seed will be set to 1.
    void rand31dc::seedi(long unsigned int seedin)
    {
        if (seedin == 0) seedin = 1;
        seed31 = seedin;
    }
                                    
    // Return next pseudo-random value as a long unsigned integer.
    long unsigned int rand31dc::ranlui(void)  
    {
        return nextrand();
    }

    // Return next pseudo-random value as a double value in the range 0.0f to 1.0f
    double rand31dc::rand(void)
    {
        // 1 / 2147483647 = 4.656612875245796924105750827168e-10
        return static_cast<double> (nextrand()) * 4.656612875245796924105750827168E-10;
    }    

    // Return next pseudo-random value as a double value in the range -1.0f to 1.0f
    double rand31dc::rand2(void)  
    {
        // 2 / 2147483647 = 9.31322574615478515625e-10
        return static_cast<double> (nextrand()) * 9.31322574615478515625E-10 - 1.0;
    }    

    // Return next pseudo-random value as a floating point value in the range 0.0f to 1.0f
    float rand31dc::ranf(void)  
    {
        return static_cast<float> (rand());
    }    

    // Return next pseudo-random value as a floating point value in the range -1.0f to 1.0f
    float rand31dc::ranf2(void)  
    {
        return static_cast<float> (rand2());
    }    

private:

    // Multiplier constant = 16807 = 7^5. This is 15 bits. Park and Miller in 1993 recommend 48271,
    // which they say produces a somewhat better quality of pseudo-random results. 48271 can't be
    // used with the following implementation of Carta's algorithm, since it is 16 bits and would
    // result in bit 31 potentially being set in lo in the first multiplication.  (A similar problem
    // occurs later with the higher bits of hi.)
    #define consta 16807

    // Modulus constant = 2^31 - 1 = 0x7FFFFFFF. We use this explicitly in the code, rather than define it
    // somewhere, because this is a value which must not be changed and should always be recognised as a zero 
    // followed by 31 ones.

    // Generate next pseudo-random number.
    long unsigned int nextrand()
    {
        // Two 32 bit integers for holding parts of the (seed31 * consta) multiplication product which would 
        // normally need a 46 bit word. 
        //   lo 31 bits   30  -  0 
        //   hi 30 bits   45  -  16  
        // These overlap in their value. Bit 0 of hi has the same weight in the result as bit 16 of lo.
                                    
        // lo = 31 bits: low 16 bits (15-0) of seed31 * 15 bit consta 
        long unsigned int lo = consta * (seed31 & 0xFFFF);
        
        // hi = 30 bits: high 15 bits (30-16) of seed31 * 15 bit consta 
        long unsigned int hi = consta * (seed31 >> 16);
        
        // The new pseudo-random number is the 46 bit product mod(0x7FFFFFF).  Our task is to calculate it with 32
        // bit words and maths, and without division.
        //
        // The first section is easy to understand.  We have a bunch of bits - bits 14 to 0 of hi -  which overlap
        // with and carry the same weight as bits 30 to 16 of lo.
        //
        // Add the low 15 bits of hi into bits 30-16 of lo.  
        lo += (hi & 0x7FFF) << 16;
        
        // The result may set bit 31 of lo, but it will not overflow lo. So far, we got some of our total result in
        // lo. The only other part of the result we need to deal with is bits 29 to 15 of hi. These bits carry
        // weights of bits 45 to 31 in the value of the multiplication product of the usual Park-Miller algorithm. 
        // David Carta writes that in order to get the mod(0x7FFFFFF) of the multiplication product we should
        // simply add these bits into the bit positions 14 to 0 of lo.
        lo += hi >> 15;

        // In order to be able to get away with this, and to perform the following simple mod(0x7FFFFFFF) operation,
        // we need to be sure that the result of the addition will not exceed: 2 * 0x7FFFFFFF = 0xFFFFFFFE
        // This is assured as per the diagrams above. Note that in the vast majority of cases, lo will be less than 
        // 0x7FFFFFFFF. 
        if (lo > 0x7FFFFFFF) lo -= 0x7FFFFFFF;          
        
        // lo contains the new pseudo-random number.  Store it to the seed31 and return it.
        return ( seed31 = static_cast<long> (lo));       
    }
};

/** White noise generator posted by Robin Whittle at http://www.firstpr.com.au/dsp/rand31/
 * Based on work by Park, Miller & Carta (see rand31pmc notes above.)
 */
class WhiteNoiseGenerator
{
public:

    WhiteNoiseGenerator ()
    { }

    ~WhiteNoiseGenerator()
    = default;

    /* Generates different noise on each channel. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        // this is an output-only processor
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));

        for (size_t ch = 0; ch <context.getOutputBlock().getNumChannels(); ++ch)
        {
            auto* dst = context.getOutputBlock().getChannelPointer(ch);

            for (size_t i = 0; i < context.getOutputBlock().getNumSamples(); i++)
                dst[i] = prng.ranf2();
        }
    }

    void reset()
    {
        prng.seedi (1);
    }

private:
    rand31dc prng;
};

 /**    Generates pink noise by applying a filter to white noise. Filter posted by Paul Kellett
  *     at http://www.musicdsp.org/files/pink.txt.
  *     
  *     Note that this generator may very occasionally produce samples outside the range of -1.0 to +1.0
  */
class PinkNoiseGenerator
{
public:

    PinkNoiseGenerator ()
    { }

    ~PinkNoiseGenerator()
    = default;

    /* Generates different noise on each channel. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        // this is an output-only processor
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));

        for (size_t ch = 0; ch <context.getOutputBlock().getNumChannels(); ++ch)
        {
            auto* dst = context.getOutputBlock().getChannelPointer(ch);

            for (size_t i = 0; i < context.getOutputBlock().getNumSamples(); i++)
            {
                const auto white = prng.rand2();
                
                // Pink noise filter posted by Paul Kellett: http://www.musicdsp.org/files/pink.txt
                //
                // This is an approximation to a -10dB/decade filter using a weighted sum of first order filters. It is accurate to within +/-0.05dB above 9.2Hz (44100Hz sampling rate). Unity gain is at Nyquist, but can be adjusted by scaling the numbers at the end of each line.
                // If 'white' consists of uniform random numbers, such as those generated by the rand() function, 'pink' will have an almost gaussian level distribution.
                b0 = 0.99886 * b0 + white * 0.0555179;
                b1 = 0.99332 * b1 + white * 0.0750759;
                b2 = 0.96900 * b2 + white * 0.1538520;
                b3 = 0.86650 * b3 + white * 0.3104856;
                b4 = 0.55000 * b4 + white * 0.5329522;
                b5 = -0.7616 * b5 - white * 0.0168980;
                dst[i] = static_cast<float> (b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362);
                b6 = white * 0.115926;

                //// 'Economy' version with accuracy of +/-0.5dB
                //b0 = 0.99765 * b0 + white * 0.0990460;
                //b1 = 0.96300 * b1 + white * 0.2965164;
                //b2 = 0.57000 * b2 + white * 1.0526913;
                //dst[i] = static_cast<float> (b0 + b1 + b2 + white * 0.1848);

                // Scaling factor empirically determined to lower risk of clipping
                 dst[i] *= 0.12348f;
            }
        }
    }

private:
    rand31dc    prng;
    double      b0 = 0.0;
    double      b1 = 0.0;
    double      b2 = 0.0;
    double      b3 = 0.0;
    double      b4 = 0.0;
    double      b5 = 0.0;
    double      b6 = 0.0;
};

} // namespace dsp
} // namespace juce