/* 
 * The Mersenne Twister pseudo-random number generator (PRNG)
 *
 * NOTE:  This program only gives you a general idea of PRNG
 *        performance.  It is NOT a scientific benchmark.
 *
 * Written by Christian Stigen Larsen
 * http://csl.sublevel3.org
 *
 * Distributed under the modified BSD license.
 *
 * This small benchmarking program should be rewritten in the same way that
 * Facebook did with Folly benchmarking:
 *
 *   https://github.com/facebook/folly/blob/master/folly/docs/Benchmark.md#a-look-under-the-hood
 *
 * It says that the running time "is not a random variable that fluctuates
 * around an average".  One should simply take the best time in each run.
 *
 */

#include <float.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <math.h>
#include <stdio.h>
#include <sys/resource.h>

#include <vector>

namespace mt {
  // In case we've got stdint's RAND_MAX, unset it
  #undef RAND_MAX
  #include "mersenne-twister.h"
};

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

/*
 * Number of digits in number.
 */
static int digits(double n)
{
  int d = 1;
  n = floor(n);

  while ( (n/=10) >= 1.0 )
    ++d;

  return d;
}

/*
 * Convert number to human readable string, i.e.
 *
 *   - 12345 ==> 12.3 thousand
 *   - 1234567 ==> 1.2 million
 *   - etc.
 *
 * using the SHORT SCALE format (i.e., English
 * variants such as "billion" = 10^9, instead of
 * "milliard".
 */
static const char* sscale(double n, int decimals = 1)
{
  static char s[32];
  static const char* name[] = {
    "",
    "thousand",
    "million",
    "billion",
    "trillion",
    "quadrillion",
    "quintillion",
    "sextillion",
    "septillion"
  };

  int exp = digits(n) <= 4? 0 : 3*((digits(n)-1)/3);
  sprintf(s, "%1.*lf %s", decimals, n/pow(10, exp), name[exp/3]);

  return s;
}

static double estimate_calls_per_second(double run_secs = 1.0)
{
  uint64_t count = 0;
  Timer timer;

  while ( count < 10000000 ) {
    mt::rand_u32();

    if ( (++count % 10000) == 0 ) {
      if ( timer.elapsed_secs() >= run_secs )
        break;
    }
  }

  return count / timer.elapsed_secs();
}

static double numbers_per_second(const uint64_t count)
{
  printf("Generating %s numbers... ", sscale(count));
  fflush(stdout);

  Timer timer;

  for ( uint64_t n = 0; n < count; ++n )
    mt::rand_u32();

  const double secs = timer.elapsed_secs();
  printf("%f seconds\n", secs);

  return count/secs;
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

int main()
{
  printf("Mersenne Twister MT19937 non-rigorous benchmarking\n");
  printf("\n");

  mt::seed(5769);

  // Find out how many numbers we expect to generate per second
  printf("Priming system performance... ");
  fflush(stdout);

  double speed = estimate_calls_per_second();
  printf("ca. %s / second\n\n", sscale(speed, 2));

  // Multiply up an amount and benchmark again in batches
  uint64_t part = 40;
  uint64_t count = part*speed;

  printf("Will generate %" PRIu64 " batches of numbers\n", part);
  printf("Using getrusage(), i.e., not wall-clock time\n");
  printf("\n");

  std::vector<double> persec;
  uint64_t total = 0;
  Timer timer;

  // smaller batches
  for ( uint64_t n=0; n<part-30; ++n ) {
    persec.push_back(numbers_per_second(count/(2*part)));
    total += count/(2*part);
  }

  // normal batches
  for ( uint64_t n=0; n<part-30; ++n ) {
    persec.push_back(numbers_per_second(count/part));
    total += count/part;
  }

  // bigger batches
  for ( uint64_t n=0; n<10; ++n ) {
    persec.push_back(numbers_per_second(2*count/part));
    total += 2*count/part;
  }

  printf("\n");
  printf("RESULTS\n");
  printf("\n");
  printf("  Total numbers generated: %s\n", sscale(total, 2));
  printf("  Total speed: %s numbers/second\n", sscale(total/timer.elapsed_secs(), 4));
  printf("\n");
  printf("  Worst performance: %s numbers/second\n", sscale(min(persec), 4));
  printf("  Best performance:  %s numbers/second\n", sscale(max(persec), 4));
  printf("\n");
  printf("  Mean performance:  %s numbers/second\n", sscale(mean(persec), 4));
  printf("  Standard deviation: %s\n\n", sscale(stddev(persec), 4));

  printf("If we assume a normal distribution, you can plot the above with R:\n"
         "\n"
         "  mean=%f;\n"
         "  sd=%f;\n"
         "  x=seq(mean-4*sd, mean+4*sd, length=200);\n"
         "  y=dnorm(x, mean=mean, sd=sd);\n"
         "  plot(x, y, type=\"l\", xlab=\"Numbers / second\", ylab=\"\");\n"
         "  title(\"Mersenne Twister performance\");\n"
         "\n", mean(persec), stddev(persec));

  printf("Note that while the mean is quite consistent between runs, standard\n"
         "deviation may not.  Be sure to compile at maximum optimization levels,\n"
         "using your native instruction set.\n\n");

  printf("Update: What you really want to look at is the *best* performance.\n"
         "It shows what is possible given the least amount of interruption,\n"
         "and should therefore be closer to the true performance of the code.\n"
         "\n"
         "The standard deviation may tell you how well the code manages to\n"
         "avoid preemption.\n\n");

  return 0;
}
