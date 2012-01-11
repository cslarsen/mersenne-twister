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

static const unsigned SIZE   = 624;
static const unsigned PERIOD = 397;
static const unsigned DIFF   = SIZE-PERIOD;

static uint32_t MT[SIZE];
static unsigned index = 0;

#define M32(x) (0x80000000 & x) // 32nd Most Significant Bit
#define L31(x) (0x7FFFFFFF & x) // 31 Least Significant Bits
#define ODD(x) (x & 1) // Check if number is odd

static inline void generate_numbers()
{
  /*
   * Originally, we had one loop with i going from [0, SIZE) and two
   * modulues operations:
   *
   * for ( register unsigned i=0; i<SIZE; ++i ) {
   *   register uint32_t y = M32(MT[i]) | L31(MT[(i+1) % SIZE]);
   *   MT[i] = MT[(i + PERIOD) % SIZE] ^ (y>>1);
   *   if ( ODD(y) ) MT[i] ^= 0x9908b0df;
   * }
   *
   * For performance reasons, we've unrolled the loop three times, thus
   * mitigating the need for any modulus operations.
   */

  register uint32_t y;
  register unsigned i;

  // i = [0 ... 226]
  for ( i=0; i<DIFF; ++i ) {
    y = M32(MT[i]) | L31(MT[i+1]);
    MT[i] = MT[i + PERIOD] ^ (y>>1);
    if ( ODD(y) ) MT[i] ^= 0x9908b0df;
  }

  // i = [227 ... 622]
  for ( i=DIFF; i<(SIZE-1); ++i ) {
    y = M32(MT[i]) | L31(MT[i+1]);
    MT[i] = MT[i-DIFF] ^ (y>>1);
    if ( ODD(y) ) MT[i] ^= 0x9908b0df;
  }

  // i = [623]
  y = M32(MT[SIZE-1]) | L31(MT[SIZE]);
  MT[SIZE-1] = MT[PERIOD-1] ^ (y>>1);
  if ( ODD(y) ) MT[SIZE-1] ^= 0x9908b0df;
}

extern "C" void initialize(uint32_t seed)
{
  MT[0] = seed;

  /*
   * For an explanation about the magic number 0x6c078965, see
   * Knuth's THE ART OF COMPUTER PROGRAMMING, volume 2, page 106.
   *
   */

  for ( register unsigned i=1; i<SIZE; ++i )
    MT[i] = 0xFFFFFFFF & (0x6c078965*(MT[i-1] ^ MT[i-1]>>30) + i);

}

extern "C" uint32_t rand_u32()
{
  if ( !index )
    generate_numbers();

  register uint32_t y = MT[index];

  // Tempering
  y ^= y>>11;
  y ^= y<<7 & 0x9d2c5680;
  y ^= y<<15 & 0xefc60000;
  y ^= y>>18;

  if ( ++index == SIZE )
    index = 0;

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
   * off the M32 from rand_u32().
   *
   */
  return 0x7FFFFFFF & rand_u32();
}

extern "C" void srand(unsigned seed)
{
  initialize(static_cast<uint32_t>(seed));
}
