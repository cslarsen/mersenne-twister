/* 
 * The Mersenne Twister pseudo-random number generator (PRNG)
 *
 * This is an implementation of fast PRNG called MT19937,
 * meaning it has a period of 2^19937-1, which is a Mersenne
 * prime.
 *
 * This PRNG is fast and suitable for non-cryptographic code.
 * For instance, it would be perfect for Monte Carlo simulations,
 * etc.
 *
 * Written by Christian Stigen Larsen
 * 2012-01-11 -- http://csl.sublevel3.org
 *
 * Distributed under the modified BSD license.
 */

#include <stdio.h>
#include <stdint.h>
#include "mersenne-twister.h"

static const uint16_t SIZE = 624;
static uint32_t MT[SIZE];
static uint16_t index = 0;

static void generate_numbers()
{
  for ( uint16_t i=0; i<SIZE; ++i ) {
    uint32_t y = (0x80000000 & MT[i]) |           // 32nd MSB
                 (0x7FFFFFFF & MT[(i+1) % SIZE]); // 31 LSB

    MT[i] = MT[(i + 397) % SIZE] ^ (y>>1);

    if ( y & 1 ) // odd?
      MT[i] ^= 0x9908b0df;
  }
}

extern "C" void srand(unsigned seed)
{
  MT[0] = static_cast<uint32_t>(seed);

  /*
   * For an explanation about the magic number 0x6c078965, see
   * Knuth's THE ART OF COMPUTER PROGRAMMING, volume 2, page 106.
   *
   */

  for ( uint16_t i=1; i<SIZE; ++i )
    MT[i] = 0xFFFFFFFF & (0x6c078965*(MT[i-1] ^ MT[i-1]>>30) + i);

}

extern "C" uint32_t rand_u32()
{
  if ( !index )
    generate_numbers();

  uint32_t y = MT[index];

  // Tempering
  y ^=  y>>11;
  y ^= (y<< 7) & 0x9d2c5680;
  y ^= (y<<15) & 0xefc60000;
  y ^=  y>>18;

  index = (index+1) % SIZE;
  return y;
}

extern "C" int rand()
{
  /*
   * PORTABILITY WARNING:
   *
   * rand_u32() uses all 32-bits for the pseudo-random number,
   * but rand() must return a number from 0 ... RAND_MAX.
   *
   * We'll just assume that (1) rand() only uses 31-bits worth of data, and
   * that (2) we're on a two's complement system.  Therefore we'll just chop
   * off the MSB from rand_u32().
   *
   */
  return 0x7FFFFFFF & rand_u32();
}
