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

int main(int argc, char* argv[])
{
  if ( argc < 2 ) {
    printf("Usage: %s [ number of iterations ]\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);

  mt::seed(5769);

  mark_time();

  for ( int i=0; i<n; ++i )
    mt::rand_u32();

  double speed = elapsed_secs();

  printf("%d %lf\n", n, speed);
  return 0;
}
