#!/bin/bash

echo 0 0 > score.dat

echo Type enter key to start...
read
./bin.d/bin iddinit ./sample_qrformat.dat 0 ./good_learning.dat
echo your turn
read

./bin.d/bin gnuscore ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json 1 >> score.dat

for i in `seq 2 70`
do
    echo ---------------------------------------------
    echo TURN $i
    ./bin.d/bin score ./jdump.json $i ./good_learning.dat
    echo Type enter key
    read
    ./bin.d/bin idd ./jdump.json $i ./good_learning.dat
    echo your turn
    read
    ./bin.d/bin gnuscore ./jdump.json $i ./good_learning.dat >> score.dat
    
done
