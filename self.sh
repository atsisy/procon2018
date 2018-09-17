#!/bin/bash

./bin.d/bin init ./sample_qrformat.dat 0
./bin continue ./cdump.json 0

echo 0 0 > score.dat

for i in `seq 1 70`
do
    echo ---------------------------------------------
    ./bin.d/bin continue ./cdump.json $i
    ./bin continue ./cdump.json $i
    ./bin.d/bin gnuscore ./cdump.json $i >> score.dat
    echo turn $i
done

./bin.d/bin score ./cdump.json

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
