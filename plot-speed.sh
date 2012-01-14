#!/bin/bash

RUN=./test-run
OUT=`mktemp /tmp/mersenne-twister.XXXXXX`
PNG=plot.png

run()
{
  ${RUN} $1 | tee -a ${OUT}
}

run 1
run 10
run 100
run 1000
run 10000
run 100000
run 1000000
run 10000000
run 100000000
run 200000000
run 300000000
run 400000000
run 500000000
run 600000000
run 700000000
run 800000000
run 900000000
run 1000000000

gnuplot - <<EOF
set term png color;
set output '${PNG}';
set title 'Mersenne Twister PRNG speed';
set xlabel "Numbers Generated";
set ylabel "Seconds";
plot '${OUT}' with linespoints title 'Speed';
EOF

rm ${OUT}

echo Plot saved to ${PNG}
