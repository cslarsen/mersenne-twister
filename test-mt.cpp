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
#include <math.h>
#include <float.h>

#include <sys/resource.h>

#include <vector>

namespace mt {
  #undef RAND_MAX
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
#if defined(__clang__)
  [[clang::optnone]]
#endif
static uint32_t benchmark_hash(
    uint32_t seed,
    uint64_t iterations,
    X set_seed,
    Y draw_u32)
#if defined(__GNUC__)
# if !defined(__clang__)
  __attribute__((optimize("-O0")))
# endif
#endif
{
  // Use a hash so that the compiler doesn't optimize away the for-loop
  uint32_t hash = 0xffffffff;

  set_seed(seed);

  for ( uint64_t n = 0; n < iterations; ++n ) {
    hash ^= draw_u32();
  }

  return hash;
}

struct Benchmark {
  uint32_t hash;
  double best;
  std::vector<double> times;

  Benchmark() : hash(0xffffffff), best(9999999999)
  {
  }
};

template<class SEEDFUNC, class RANDFUNC>
static Benchmark benchmark_hashes(
    SEEDFUNC set_seed,
    RANDFUNC draw_u32,
    const size_t passes = 15,
    const size_t subiterations = 200000000ULL)
{
  Benchmark result;

  for ( size_t pass = 0; pass < passes; ++pass ) {
    Timer timer;
    // use a different seed each time
    result.hash ^= benchmark_hash(pass*19, subiterations, set_seed, draw_u32);
    const double secs = timer.elapsed_secs();
    result.times.push_back(secs);

    if ( secs < result.best ) {
      result.best = secs;
      printf(" %8.6fs ", result.best);
      fflush(stdout);
    } else {
      printf(".");
      fflush(stdout);
    }
  }

  return result;
}

static double mean(const std::vector<double>& v)
{
  double sum = 0;

  for ( size_t n=0; n<v.size(); ++n )
    sum += v[n];

  return sum/v.size();
}

static double min(const std::vector<double>& v)
{
  double out = DBL_MAX;
  for ( size_t n=0; n<v.size(); ++n )
    out = v[n] < out? v[n]: out;
  return out;
}

static double max(const std::vector<double>& v)
{
  double out = 0;
  for ( size_t n=0; n<v.size(); ++n )
    out = v[n] > out? v[n]: out;
  return out;
}

static double stddev(const std::vector<double>& v)
{
  double m = mean(v);
  double sumsq = 0;

  for ( size_t n=0; n<v.size(); ++n )
    sumsq += (v[n] - m) * (v[n] - m);

  return sqrt(sumsq/v.size());
}

static void run_benchmark()
{
  printf("\nTiming reference mt19937ar.c ... ");
  fflush(stdout);

  const auto ref = benchmark_hashes(reference::init_genrand,
      reference::genrand_int32);

  printf("\n");
  printf("  min=%g max=%g mean=%g stddev=%g\n",
      min(ref.times), max(ref.times), mean(ref.times),
      stddev(ref.times));

  printf("\nTiming our implementation ... ");
  fflush(stdout);

  const auto our = benchmark_hashes(mt::seed, mt::rand_u32);

  printf("\n");
  printf("  min=%g max=%g mean=%g stddev=%g\n",
      min(our.times), max(our.times), mean(our.times),
      stddev(our.times));

  if ( our.hash != ref.hash ) {
    printf("Error: Our implementation produces incorrect numbers\n");
  }

  const double ratio = ref.best / our.best;
  printf("\n%g times %s than the reference (ratio of best runs)\n", ratio,
      ratio > 1 ? "faster" : "slower");
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

  run_benchmark();
  return 0;
}
