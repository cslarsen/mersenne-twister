A Mersenne Twister in C++
=========================

This is an implementation of the fast pseudo-random number generator (PRNG)
**MT19937**, colloquially called the _Mersenne Twister_.  It was given this
name because it has a period of 2^19937 - 1, which is a Mersenne prime.

The Mersenne Twister is highly regarded for its performance and high quality pseudo-random numbers.
In spite of this, it is *not* suited for cryptographic code, because one
only needs to observe 624 iterates to predict all future ones.  It was
designed with statistical simulations in mind, and should therefore be quite
good for Monte Carlo simulations, probabilistic algorithms and so on.

You can read more about it on the Wikipedia page at
https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister

Drop-in replacement for libc's srand() and rand()
-------------------------------------------------

This implementation was specifically designed to be a drop-in replacement
for libc's rand() and srand().  The function signatures are identical with
libc's and use C-style name mangling for binary compatibility.

You can even mix the two PRNGs by wrapping either of them in a namespaces.

Performance
-----------

Regarding speed; I haven't optimized the code in any way.  My aim was to
write a *clean* and *readable* implementation of the Mersenne Twister.
Luckily, the resulting code is *fast enough*.  Indeed, in a simple benchmark
I did on my system, the **unoptimized** code ran just as fast as the
**optimized** version of rand() that came with the OS.

If you want to optimize the code, you can start by unrolling the loop in
generate_numbers() to avoid all the modulus operations.  This is trivial,
but I'm not sure if it will affect performance.

**EDIT** I've now unrolled the generate_numbers() loop to avoid modulus
operations and used better data types for indices.  This is still a very
simple optimization trick, but the code now runs twice as fast.

**EDIT 2** Ok, so I couldn't resist and have done another optimization
trick.  On my Intel i7 laptop it generates about 203 million numbers per
second.  On _my_ computer, this is the speed measurement:

http://www.wolframalpha.com/input/?i=normal+distribution+mean+203700000+standard+deviation+3226951.557

On _my_ computer it runs faster than the reference implementation and all
other non-SIMD Mersenne Twisters I've tested.  But that's on _my_ system.
Your mileage may wary. :)

Portability
-----------

The MT19937 algorithm is inherently 32-bit, but works nicely on 64-bit
systems.  

While it uses the all 32 bits to express pseudo-random numbers, rand() does
not --- by design it will only return numbers in the range 0 ... INT32_MAX,
effectively using only 31 bits of randomness.  This is a well known point of
criticism for rand().

To implement rand() with rand_u32(), I just chop off the MSB.  This assumes
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
    
     3012767458657393270  4861252995939981590  8171373547965632397
     3295231920207293042  8379757656234809013  5749482273722797066
      145925191528956087  8572545398272020024  6372399325395526037
     9198679701087902816  1589560648737759187  1264854042067457521
     8601674933172701325  6427013158495133300   608744156293566086
     6967916787276743352  6953280600449268623  8513398738853508091
     6562657546016137666  1146197319580544821   183361855752030628
      241753687246785272   261081280177958184  2270896297431369947
     7932357750702041457  4969839388718826957  5098882734407357902
    
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
    
    Priming system performance... ca. 0.1 billion / second
    Generating 1.4 billion numbers... 7.99 seconds
    
    This equals 0.18 billion pseudo-random numbers / second

It makes a quick check that the first 200 numbers with seed = 1 are
correct.

Bugs
----

Please report any bugs to the author.

Author and license
------------------
Written by Christian Stigen Larsen, http://csl.sublevel3.org

Distributed under the modified BSD license.

2012-01-11

References
----------
The code is a translation of the MT19937 pseuco-code at 
https://secure.wikimedia.org/wikipedia/en/wiki/Mersenne_twister

