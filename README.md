A fast Mersenne Twister in C++
==============================

This is an implementation of the fast pseudo-random number generator (PRNG)
**MT19937**, colloquially called the _Mersenne Twister_.  It was given this
name because it has a period of 2^19937 - 1, which is a Mersenne prime.

The code here is actually ~1.6 times *faster* (on Intel CPUs) than the
reference implementation mt19937ar.c (see below).

The Mersenne Twister is highly regarded for its performance and high quality
pseudo-random numbers. In spite of this, it is *not* suited for cryptographic
code, because one only needs to observe 624 iterates to predict all future
ones.  It was designed with statistical simulations in mind, and should
therefore be quite good for Monte Carlo simulations, probabilistic algorithms
and so on.

You can read more about the [Mersenne Twister on
Wikipedia](https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister).

**UPDATE**

I removed the `srand` and `rand` C standard library drop-in functions, because
I believe they contained errors.

**OLDER UPDATE**

All prior versions with loop unrolling had a bug that caused numbers to differ
significantly from the reference implementation. This has now been fixed, and
the tests have been expanded to test 2000 consecutive numbers and numbers at
doubling index positions up to over four billion.  Thanks to Mikael Leetmaa for
letting me know about this!

I've also fixed an out-of-bounds read in the MT array. Thanks to Nyall Dawson
for finding this bug!

Performance
-----------

This implementation is _very fast_.  It runs faster than the reference,
non-SIMD, implementation in the original paper (*and* the more recent code in
mt19937ar.c) on the computers I've checked with (all Intel CPUs of different
generations). To be sure, please run the test program by typing `make check`.
See below for numbers.

The original optimization trick I did was to unroll the loop in
`generate_number()` three times to avoid the relatively expensive modulus
operations.  The mod instructions were used to have the array index wrap
around, but was alleviated with the loops and some simple arithmetic.  This is
a well known trick.

However, I tried unrolling each loop even more, since the loop counters can be
factorized.  The idea was to fill the CPU's [instruction
pipeline](http://en.wikipedia.org/wiki/Instruction_pipeline) and avoid flushing
it.  It worked fine on my Intel Core i7 computer, and increased performance
from around 186M numbers/sec to 204M numbers/sec. It may not work as well on
other architectures, though.

I was tipped by Michel Valin of another neat trick to choose which value in the
matrix to use. The previous code checked if `y` was odd with a bitwise AND, and
used that to index into an array to choose between zero and a magic value.
However, there's a really cool trick that can be used instead: Just put the `y`
value in a _signed_ variable, shift left 31 then shift right 31 again. What
happens then is that, because the variable is signed, it will use the `SARL`
x86 instruction so that the LSB is effectively copied to all other bit
positions. The final step is then to bitwise AND the result with the magic
value. In other words, previous code:

    static int MATRIX[] = {0, 0x12345678};
    // ...
    uint32_t foo = MATRIX[y & 1] // 0 if y is even, 0x12345678 if it's odd

changes to

    uint32_t foo = ((int32_t(y) << 31) >> 31) & 0x12345678

With `-ftree-vectorize`, it seems that this trick does wonders and speeds up
the code even more.

Finally, note that people have done SIMD and CUDA implementations.  If
you are looking for even more speed, I suggest you check them out.

Compilation and usage
---------------------

To build the example, just type

    $ make clean check

You'll see if this implementation runs faster than the reference non-SIMD
Mersenne Twister.

On an older Intel Core i7 (my machine), using clang 4 (I think) the output
looks like this:

    $ ./test-mt 20
    Testing Mersenne Twister with reference implementation
      * Pass 1/2  OK
      * Pass 2/2  OK

    Timing our implementation (best times over 20 passes) ... 
      1.0321990s 
      0.9729490s ..................
      min=0.972949s max=1.0322s mean=1.00114s stddev=0.0143048s
      193.8 million — 205.6 million numbers/second

    Timing reference mt19937ar.c (best times over 20 passes) ... 
      1.1132160s .
      1.1116460s ..
      1.0994660s .
      1.0944240s ............
      min=1.09442s max=1.14412s mean=1.12278s stddev=0.0126344s
      174.8 million — 182.7 million numbers/second

    1.12485 times faster than the reference (ratio of best runs)

On an Intel Xeon with gcc 6.3:

    Testing Mersenne Twister with reference implementation
      * Pass 1/2  OK
      * Pass 2/2  OK

    Timing our implementation (best times over 20 passes) ...
      0.5661380s
      0.5654360s
      0.5652670s .
      0.5649580s ...
      0.5641450s ...........
      min=0.564145s max=0.569173s mean=0.565719s stddev=0.00129429s
      351.4 million — 354.5 million numbers/second

    Timing reference mt19937ar.c (best times over 20 passes) ...
      0.9026930s
      0.9001340s
      0.8963510s .
      0.8963100s .
      0.8959060s ..
      0.8956320s .
      0.8955830s ......
      0.8953400s .
      min=0.89534s max=0.902693s mean=0.896933s stddev=0.00172019s
      221.6 million — 223.4 million numbers/second

    1.58707 times faster than the reference (ratio of best runs)

You can pass the number of iterations to perform on the command line, e.g.

    $ ./test-mt 100

This is quite important to let the CPU throttle up to get the best numbers. For
each iteration, the time is printed if it's better than seen before. If it
isn't better, a dot is printed.

To actually use the code, include the header and cpp file into your project.
Then

    namespace mt {
      #include "mersenne-twister.h"
    }

    // ...

    mt::seed(1234);
    printf("a pseudo-random number: %d\n", mt::rand_u32());

Also look at the `Makefile` here as well, it contains a few optimization flags
that you may want to use.

Portability
-----------

The code should be portable, although I have only tried on Intel CPUs. I don't
know if the speed holds up on other CPUs (and I doubt it). If you're not on
UNIX, you can pretty easily port the code.

The MT19937 algorithm is inherently 32-bit, so you only get 32-bit values.


Bugs
----

Please report any bugs to the author.

Author and license
------------------

Written by [Christian Stigen Larsen](https://csl.name)

Distributed under the modified BSD license.

2015-02-17, 2017-12-06

References
----------

* This code was originally a translation of the [MT19937 pseudo-code on
  Wikipedia](https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister)
* The [original Mersenne Twister paper](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf)
