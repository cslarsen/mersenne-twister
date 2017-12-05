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
 * http://csl.name
 *
 * Distributed under the modified BSD license.
 *
 * 2015-02-17
 */

#include <stdio.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace mt {
  #include "mersenne-twister.h"
}

namespace reference {
  #include "reference/mt19937ar.c"
}

int main()
{
  printf("Testing Mersenne Twister with reference implementation\n");

  const uint32_t seeds = 10000;
  const uint32_t start = 0;
  const uint32_t stop = 10000;
  const uint32_t passes = 2;

  for ( uint32_t pass=0; pass < passes; ++pass) {
    for ( uint32_t seed = 0; seed < seeds; ++seed ) {
      mt::seed(seed);
      reference::init_genrand(seed);

      if ( (seed % 100) == 0 ) {
        printf("\rPass %d/%d %4" PRIu64 "%%", 1 + pass, passes,
            100ULL * uint64_t(seed)/uint64_t(seeds));
        fflush(stdout);
      }

      for ( uint32_t n = start; n < stop; ++n ) {
        uint32_t a = mt::rand_u32();
        uint32_t b = reference::genrand_int32();

        if ( a != b ) {
          printf("\rError: For seed=%" PRIu32
                 " and n=%" PRIu32
                 " got %" PRIu32
                 " but expected %" PRIu32 "\n", seed, n, a, b);
          return 1;
        }
      }
    }

    printf("\rPass %d/%d OK     \n", 1 + pass, passes);
  }

  return 0;
}
