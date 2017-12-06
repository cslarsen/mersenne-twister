A fast Mersenne Twister in C++
==============================

This is an implementation of the fast pseudo-random number generator (PRNG)
**MT19937**, colloquially called the _Mersenne Twister_.  It was given this
name because it has a period of 2^19937 - 1, which is a Mersenne prime.

The Mersenne Twister is highly regarded for its performance and high quality
pseudo-random numbers.  In spite of this, it is *not* suited for cryptographic
code, because one only needs to observe 624 iterates to predict all future
ones.  It was designed with statistical simulations in mind, and should
therefore be quite good for Monte Carlo simulations, probabilistic algorithms
and so on.

You can read more about the [Mersenne Twister on
Wikipedia](https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister).

**UPDATE**

I removed the `srand` and `rand` C standard library drop-in functions, because
I believe they contained errors.

**UPDATE**

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
non-SIMD, implementation in the original paper on the computers I've checked
with (all Intel CPUs of different generations). To be sure, please run the test
program by typing `make check`.

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

Portability
-----------

The code should be portable, although I have only tried on Intel CPUs. I don't
know if the speed holds up on other CPUs (and I doubt it). If you're not on
UNIX, you can pretty easily port the code.

The MT19937 algorithm is inherently 32-bit, so you only get 32-bit values.

Compilation and usage
---------------------

To build the example, just type

    $ make clean check

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
