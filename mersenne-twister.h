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
 * This code has been designed as a drop-in replacement for libc rand and
 * srand().  If you need to mix them, you should encapsulate this code in a
 * namespace.
 *
 * Written by Christian Stigen Larsen
 * 2012-01-11 -- http://csl.sublevel3.org
 *
 * Distributed under the modified BSD license.
 */

#include <stdint.h>

/*
 * Maximum number you can get from rand().
 */
#define RAND_MAX INT32_MAX

/*
 * Initialize the number generator with given seed.
 * (LIBC REPLACEMENT FUNCTION)
 */
extern "C" void srand(unsigned seed);

/*
 * Extract a pseudo-random integer in the range 0 ... RAND_MAX.
 * (LIBC REPLACEMENT FUNCTION)
 */
extern "C" int rand();

/*
 * Extract a pseudo-random unsigned 32-bit integer in the range 0 ... UINT32_MAX
 */
extern "C" uint32_t rand_u32();

/*
 * Initialize with given seed value.
 */
extern "C" void initialize(uint32_t seed);
