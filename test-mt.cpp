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

#include <sys/resource.h>

namespace mt {
  #include "mersenne-twister.h"
}

namespace reference {
  #include "reference/mt19937ar.c"
}

struct Timer {
  double mark_;

  Timer() : mark_(rusage_self())
  {
  }

  double rusage_self() const
  {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
  }

  double elapsed_secs() const
  {
    return rusage_self() - mark_;
  }

  void reset()
  {
    mark_ = rusage_self();
  }
};

template<class X, class Y>
uint32_t benchmark(
    uint32_t seed,
    uint64_t iterations,
    X set_seed,
    Y draw_u32)
{
  uint32_t hash = 0xffffffff;

  set_seed(seed);

  for ( uint64_t n = 0; n < iterations; ++n ) {
    hash ^= draw_u32();
  }

  return hash;
}

int benchmark(const uint64_t iterations = 200000000ULL)
{
  const int passes = 10;
  double reftime=9999, ourtime=9999;

  printf("Benchmarking against reference implementation ... please wait\n");
  printf("Will take the *best* times over %d runs for each\n", passes);

  // Calculate hashes so that the optimizer won't optimize away
  uint32_t refhash, ourhash;

  for ( int pass = 0; pass < passes; ++pass ) {
    {
      Timer t;
      refhash = benchmark(0, iterations, reference::init_genrand, reference::genrand_int32);
      const double elapsed = t.elapsed_secs();
      if ( elapsed < reftime ) {
        reftime = elapsed;
        printf("  * %9.7f secs (mt19937ar.c)\n", reftime);
      } else {
        printf("  * no improvement (mt19937ar.c)\n");
      }
    }

    {
      Timer t;
      ourhash = benchmark(0, iterations, mt::seed, mt::rand_u32);
      const double elapsed = t.elapsed_secs();
      if ( elapsed < ourtime ) {
        ourtime = elapsed;
        printf("  * %9.7f secs (our mt)\n", ourtime);
      } else {
        printf("  * no improvement (our mt)\n");
      }
    }

    if ( refhash != ourhash ) {
      printf("Hashes do not match\n");
      return 1;
    }
  }

  const double ratio = reftime / ourtime;
  printf("  * %9.7f x %s (higher is better)\n", ratio,
      ratio > 1 ? "faster" : "slower");
  return 0;
}

int main()
{
  printf("Testing Mersenne Twister with reference implementation\n");

  const uint32_t passes = 2;
  const uint32_t seeds  = 5000;
  const uint32_t start  = 0;
  const uint32_t stop   = 5000;

  for ( uint32_t pass=0; pass < passes; ++pass) {
    for ( uint32_t seed = 0; seed < seeds; ++seed ) {
      mt::seed(seed);
      reference::init_genrand(seed);

      if ( (seed % 100) == 0 ) {
        printf("\r  * Pass %d/%d %4" PRIu64 "%%", 1 + pass, passes,
            100ULL * uint64_t(seed)/uint64_t(seeds));
        fflush(stdout);
      }

      for ( uint32_t n = start; n < stop; ++n ) {
        uint32_t a = mt::rand_u32();
        uint32_t b = reference::genrand_int32();

        if ( a != b ) {
          printf("\r  * Pass %d/%d ERROR\n", 1 + pass, passes);
          printf("\r    seed=%" PRIu32
                         " n=%" PRIu32
                  " expected %" PRIu32
                  " got %" PRIu32 "\n", seed, n, b, a);
          return 1;
        }
      }
    }

    printf("\r  * Pass %d/%d  OK       \n", 1 + pass, passes);
  }

  benchmark();
  return 0;
}
