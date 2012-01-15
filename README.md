A fast Mersenne Twister in C++
==============================

This is an implementation of the fast pseudo-random number generator (PRNG)
**MT19937**, colloquially called the _Mersenne Twister_.  It was given this
name because it has a period of 2^19937 - 1, which is a Mersenne prime.

The Mersenne Twister is highly regarded for its performance and high quality pseudo-random numbers.
In spite of this, it is *not* suited for cryptographic code, because one
only needs to observe 624 iterates to predict all future ones.  It was
designed with statistical simulations in mind, and should therefore be quite
good for Monte Carlo simulations, probabilistic algorithms and so on.

You can read more about the [Mersenne Twister on Wikipedia](https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister).

Drop-in replacement for libc's srand() and rand()
-------------------------------------------------

This implementation was specifically designed to be a drop-in replacement
for libc's rand() and srand().  The function signatures are identical with
libc's and use C-style name mangling for binary compatibility.

You can even mix the two PRNGs by wrapping either of them in a namespaces.

Performance
-----------

I originally wanted to create a simple implementation of MT19937 with clean
and readable source code.  But I couldn't resist doing some optimizations,
so unfortunately the code is a bit muddled now.

This implementation is _very fast_.  It runs faster than the reference
implementation in the original paper, at least on _my_ computer.  Most other
_non-SIMD_ implementations are based on the reference code, so I consider this to be
one of the faster ones.  However, performance measurement is difficult, and
especially across various CPU architectures and systems, so you should
really test it out and decide for yourself.

The original optimization trick I did was to unroll the loop in
`generate_number()` three times to avoid the relatively expensive modulus
operations.  The mod instructions were used to have the array index wrap
around, but was alleviated with the loops and some simple arithmetic.
This is a well known trick.

However, I tried unrolling each loop even more, since the loop counters can
be factorized.  The idea was to fill the CPU's [instruction pipeline](http://en.wikipedia.org/wiki/Instruction_pipeline) 
and avoid flushing it.  It worked fine on my Intel Core i7 computer, and
increased performance from around 186M numbers/sec to 204M numbers/sec. It
may not work as well on other architectures, though.

I added code for doing simple benchmarking, and on my computer, the Mersenne
Twister implementation generates numbers at a rate steadily over 200 million
pseudo-random numbers per second.  The other implementations I tested
performed around 180M numbers/second, which is not bad either.

The test code reports mean and standard deviation for your system.  Here is
an example run from mine:

![MT19937 performance probability distribution](https://github.com/cslarsen/mersenne-twister/raw/master/html/plot-numpersec-small.png)

You can also view the [MT19937 performance on Wolfram|Alpha](http://www.wolframalpha.com/input/?i=normal+distribution+mean+207283382.666440+standard+deviation+1671408.324083)

Finally, note that people have done SIMD and CUDA implementations.  If
you are looking for even more speed, I suggest you check them out.

Portability
-----------

The code should be portable.

The MT19937 algorithm is inherently 32-bit, but works nicely on 64-bit
systems.  

While it uses the all 32 bits to express pseudo-random numbers, `rand()` does
not.  By design it will only return numbers in the range `0 ... INT32_MAX`,
effectively using only 31 bits of randomness.  This is a well known point of
criticism for rand().

To implement `rand()` with `rand_u32()`, I just chop off the MSB.  This assumes
that the compiler is using two's complement for encoding negative numbers.

Compilation and usage
---------------------

To build the example, just type

    $ make clean check

which should produce the following output:

    Mersenne Twister -- printing the first 200 numbers seed 1
    
    1791095845  4282876139  3093770124  4005303368      491263 
     550290313  1298508491  4290846341   630311759  1013994432 
     396591248  1703301249   799981516  1666063943  1484172013 
    2876537340  1704103302  4018109721  2314200242  3634877716 
    1800426750  1345499493  2942995346  2252917204   878115723 
    1904615676  3771485674   986026652   117628829  2295290254 
    2879636018  3925436996  1792310487  1963679703  2399554537 
    1849836273   602957303  4033523166   850839392  3343156310 
    3439171725  3075069929  4158651785  3447817223  1346146623 
     398576445  2973502998  2225448249  3764062721  3715233664 
    3842306364  3561158865   365262088  3563119320   167739021 
    1172740723   729416111   254447594  3771593337  2879896008 
     422396446  2547196999  1808643459  2884732358  4114104213 
    1768615473  2289927481   848474627  2971589572  1243949848 
    1355129329   610401323  2948499020  3364310042  3584689972 
    1771840848    78547565   146764659  3221845289  2680188370 
    4247126031  2837408832  3213347012  1282027545  1204497775 
    1916133090  3389928919   954017671   443352346   315096729 
    1923688040  2015364118  3902387977   413056707  1261063143 
    3879945342  1235985687   513207677   558468452  2253996187 
      83180453   359158073  2915576403  3937889446   908935816 
    3910346016  1140514210  1283895050  2111290647  2509932175 
     229190383  2430573655  2465816345  2636844999   630194419 
    4108289372  2531048010  1120896190  3005439278   992203680 
     439523032  2291143831  1778356919  4079953217  2982425969 
    2117674829  1778886403  2321861504   214548472  3287733501 
    2301657549   194758406  2850976308   601149909  2211431878 
    3403347458  4057003596   127995867  2519234709  3792995019 
    3880081671  2322667597   590449352  1924060235   598187340 
    3831694379  3467719188  1621712414  1708008996  2312516455 
     710190855  2801602349  3983619012  1551604281  1493642992 
    2452463100  3224713426  2739486816  3118137613   542518282 
    3793770775  2964406140  2678651729  2782062471  3225273209 
    1520156824  1498506954  3278061020  1159331476  1531292064 
    3847801996  3233201345  1838637662  3785334332  4143956457 
      50118808  2849459538  2139362163  2670162785   316934274 
     492830188  3379930844  4078025319   275167074  1932357898 
    1526046390  2484164448  4045158889  1752934226  1631242710 
    1018023110  3276716738  3879985479  3313975271  2463934640 
    1294333494    12327951  3318889349  2650617233   656828586 
    
    Generating 64-bit pseudo-random numbers
    
     6025534914829572726  9722505991841274134 16342747097180917645
     6590463838393199218 16759515313480685237 11498964545438157834
      291850381902561463 17145090797266914872 12744798652718096789
    18397359401874807904  3179121295446253523  2529708085239731697
    17203349865520957581 12854026314996063860  1217488311597731462
    13935833576665547448 13906561200002743183 17026797477495256059
    13125315094098747842  2292394638421159733   366723709801908644
      483507374032440056   522162562461265192  4541792594424072411
    15864715500164049265  9939678776442258893 10197765464658613710
    
    Float values in range [0..1]
    
    0.842031 0.278319 0.124173 0.233623 0.279184
    0.091556 0.585759 0.570067 0.969596 0.417927
    0.561030 0.367843 0.018647 0.812995 0.800633
    0.289760 0.232974 0.717392 0.807105 0.612948
    0.387861 0.426910 0.863542 0.751873 0.747122
    0.427813 0.556240 0.428382 0.136455 0.361904
    0.059918 0.153338 0.121343 0.937190 0.044552
    0.916410 0.107494 0.781958 0.225709 0.616329
    
    Found 0 incorrect numbers
    
    ./test-bench
    Mersenne Twister MT19937 non-rigorous benchmarking
    
    Priming system performance... ca. 157.39 million / second
    
    Will generate 40 batches of numbers
    Using getrusage(), i.e., not wall-clock time
    
    Generating 78.7 million numbers... 0.378168 seconds
    Generating 78.7 million numbers... 0.379396 seconds
    Generating 78.7 million numbers... 0.378976 seconds
    Generating 78.7 million numbers... 0.379467 seconds
    Generating 78.7 million numbers... 0.379825 seconds
    Generating 78.7 million numbers... 0.380530 seconds
    Generating 78.7 million numbers... 0.378477 seconds
    Generating 78.7 million numbers... 0.379181 seconds
    Generating 78.7 million numbers... 0.382254 seconds
    Generating 78.7 million numbers... 0.394970 seconds
    Generating 157.4 million numbers... 0.762486 seconds
    Generating 157.4 million numbers... 0.755937 seconds
    Generating 157.4 million numbers... 0.755708 seconds
    Generating 157.4 million numbers... 0.755189 seconds
    Generating 157.4 million numbers... 0.756070 seconds
    Generating 157.4 million numbers... 0.755928 seconds
    Generating 157.4 million numbers... 0.755958 seconds
    Generating 157.4 million numbers... 0.755801 seconds
    Generating 157.4 million numbers... 0.759795 seconds
    Generating 157.4 million numbers... 0.760088 seconds
    Generating 314.8 million numbers... 1.524510 seconds
    Generating 314.8 million numbers... 1.523943 seconds
    Generating 314.8 million numbers... 1.521945 seconds
    Generating 314.8 million numbers... 1.525202 seconds
    Generating 314.8 million numbers... 1.510472 seconds
    Generating 314.8 million numbers... 1.509650 seconds
    Generating 314.8 million numbers... 1.511206 seconds
    Generating 314.8 million numbers... 1.512368 seconds
    Generating 314.8 million numbers... 1.509925 seconds
    Generating 314.8 million numbers... 1.521888 seconds
    
    RESULTS
    
      Mean performance: 207.2834 million numbers/second
      Standard deviation: 1.6714 million
    
    If we assume a normal distribution, you can plot the above with R:
    
      mean=207283382.666440;
      sd=1671408.324083;
      x=seq(mean-4*sd, mean+4*sd, length=200);
      y=dnorm(x, mean=mean, sd=sd);
      plot(x, y, type="l", xlab="Numbers / second", ylab="");
      title("Mersenne Twister performance");
    
    Note that while the mean is quite consistent between runs, standard
    deviation may not.  Be sure to compile at maximum optimization levels,
    using your native instruction set.

It makes a quick check that the first 200 numbers with seed = 1 are
correct, then produces some 64-bit and float values.

The last bit performs a simple benchmarking and lets you plot the
performance with R.  For the run above on:

![Mersenne Twister performance graph](https://github.com/cslarsen/mersenne-twister/raw/master/html/plot-numpersec.png)

Bugs
----

Please report any bugs to the author.

Author and license
------------------

Written by [Christian Stigen Larsen](http://csl.sublevel3.org)

Distributed under the modified BSD license.

2012-01-11

References
----------

* This code was originally a translation of the [MT19937 pseudo-code on
Wikipedia](https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister)
* The [original Mersenne Twister paper](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf)
