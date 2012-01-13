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
#include <sys/time.h>
#include "mersenne-twister.h"

static struct timeval mark;

void start_timer()
{
  gettimeofday(&mark, NULL);
}

double elapsed_secs()
{
  struct timeval stop;
  gettimeofday(&stop, NULL);

  return (stop.tv_sec - mark.tv_sec)
      + (stop.tv_usec - mark.tv_usec)/1000000.0;
}

double calls_per_second(double run_secs = 1.0)
{
  uint64_t count = 0;
  start_timer();

  while ( count < 10000000 ) {
    rand_u32();

    if ( (++count % 10000) == 0 ) {
      if ( elapsed_secs() >= run_secs )
        break;
    }
  }

  return count / elapsed_secs();
}

double benchmark(const uint64_t count)
{
  start_timer();

  for ( uint64_t n = 0; n < count; ++n )
    rand_u32();

  return elapsed_secs();
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

int main()
{
  printf("Mersenne Twister MT19937 non-rigorous benchmarking\n");
  printf("\n");

  seed(5769);

  // Find out how many numbers we expect to generate per second
  printf("Priming system performance... ");
  fflush(stdout);

  double speed = calls_per_second();
  uint64_t dim = dimension(speed);
  printf("ca. %.1lf %s / second\n", speed/pow10(dim), unit(dim));

  // Multiply up an amount and benchmark again
  uint64_t count = 10.0*speed;
  dim = dimension(count);

  printf("Generating %.1lf %s numbers... ",
    (double)count/pow10(dim-1), unit(dim-1));
  fflush(stdout);

  double secs = benchmark(count);
  printf("%.2lf seconds\n\n", secs);

  dim = dimension(count);
  printf("This equals %.3lf %s pseudo-random numbers / second\n\n",
    (count/secs)/pow10(dim), unit(dim));

  return 0;
}
