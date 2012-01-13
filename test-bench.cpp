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
 */

#include <stdio.h>
#include <stdint.h>
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

uint32_t dimension(uint64_t n)
{
  uint32_t d = 0;

  while ( n )
    n /= 10, ++d;

  return d;
}

uint64_t pow10(uint32_t n)
{
  n = 3*(n/3); // nearest clear cut unit
  uint64_t r = 1;

  while ( n-- )
    r *= 10;

  return r;
}

const char* unit(uint32_t dimension)
{
  static const char* units[] = {
    "",
    "thousand",
    "million",
    "billion",
    "trillion",
    "quadrillion",
    "quintillion"
  };

  dimension /= 3;

  if ( dimension > sizeof(units)/sizeof(char*) )
    return "kabillion";

  return units[dimension];
}

double numbers_per_second(const uint64_t count)
{
  printf("Generating %.1lf %s numbers... ",
    (double)count/pow10(dimension(count)-1),
    unit(dimension(count)-1));
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
  uint64_t dim = dimension(speed);
  printf("ca. %.1lf %s / second\n\n", speed/pow10(dim), unit(dim));

  // Multiply up an amount and benchmark again in batches
  uint64_t part = 40;
  uint64_t count = part*speed;

  printf("Will generate %Lu batches of numbers\n", part);
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

  dim = dimension(mean(persec));
  dim -= 1;

  double spd = mean(persec)/pow10(dim);
  double dev = stddev(persec);

  printf("\nRESULTS\n");
  printf("Mean performance: %.1lf %s pseudo-random numbers / second\n",
    spd, unit(dim));

  printf("Standard deviation: %.3lf\n", dev);
  printf("\n");

  return 0;
}
