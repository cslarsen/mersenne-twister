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

#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <sys/resource.h>
#include <math.h>
#include <vector>

namespace mt {
  // In case we've got stdint's RAND_MAX, unset it
  #undef RAND_MAX
  #include "mersenne-twister.h"
};

static double mark;

double rusage_self()
{
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
}

void mark_time()
{
  mark = rusage_self();
}

double elapsed_secs()
{
  return rusage_self() - mark;
}

/*
 * Number of digits in number.
 */
int digits(double n)
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
const char* sscale(double n, int decimals = 1)
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

double estimate_calls_per_second(double run_secs = 1.0)
{
  uint64_t count = 0;
  mark_time();

  while ( count < 10000000 ) {
    mt::rand_u32();

    if ( (++count % 10000) == 0 ) {
      if ( elapsed_secs() >= run_secs )
        break;
    }
  }

  return count / elapsed_secs();
}

double numbers_per_second(const uint64_t count)
{
  printf("Generating %s numbers... ", sscale(count));
  fflush(stdout);

  mark_time();

  for ( uint64_t n = 0; n < count; ++n )
    mt::rand_u32();

  double secs = elapsed_secs();
  printf("%f seconds\n", secs);

  return count/secs;
}

double mean(const std::vector<double>& v)
{
  double sum = 0;

  for ( size_t n=0; n<v.size(); ++n )
    sum += v[n];

  return sum/v.size();
}

double stddev(const std::vector<double>& v)
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

  printf("Will generate %llu batches of numbers\n", part);
  printf("Using getrusage(), i.e., not wall-clock time\n");
  printf("\n");

  std::vector<double> persec;

  // smaller batches
  for ( uint64_t n=0; n<part-30; ++n )
    persec.push_back(numbers_per_second(count/(2*part)));

  // normal batches
  for ( uint64_t n=0; n<part-30; ++n )
    persec.push_back(numbers_per_second(count/part));

  // bigger batches
  for ( uint64_t n=0; n<10; ++n )
    persec.push_back(numbers_per_second(2*count/part));

  printf("\n");
  printf("RESULTS\n");
  printf("\n");
  printf("  Mean performance: %s numbers/second\n", sscale(mean(persec), 4));
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

  return 0;
}
