#!/bin/bash

./bin.d/bin init ./sample_qrformat.dat 0 ./learning.dat
./bin greedy ./cdump.json 0 ./learning2.dat

echo 0 0 > score.dat

for i in `seq 1 69`
do
    echo ---------------------------------------------
    ./bin.d/bin continue ./cdump.json $i ./learning.dat
    ./bin greedy ./cdump.json $i ./learning2.dat
    ./bin.d/bin gnuscore ./cdump.json $i ./learning.dat  >> score.dat
    echo turn $i
done

./bin.d/bin score ./cdump.json 0 ./learning.dat 

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
