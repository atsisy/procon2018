#!/bin/bash

f=$1
get_th=2

if [ ! -e $f ]; then
    touch $f
fi

for i in `seq 1 50`
do
    echo "in progress ==> " $i / 50
    ./self.sh
    score=`./bin.d/bin final-score ./cdump.json $i`
    if test $score -gt $get_th ; then
        cat learning.dat >> $f
    fi
    rm learning.dat
    touch learning.dat
done
