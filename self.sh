#!/bin/bash

./bin iddinit ./sample_qrformat.dat 0 ./good_learning.dat
./bin.d/bin experiment ./cdump.json 0 ./good_learning.dat

echo 0 0 > score.dat

for i in `seq 1 70`
do
    #read
    echo ---------------------------------------------
    ./bin idd ./cdump.json $i ./good_learning.dat
    ./bin.d/bin experiment ./cdump.json $i ./good_learning.dat 
    ./bin.d/bin gnuscore ./cdump.json $i ./good_learning.dat  >> score.dat
    echo turn $i
done

./bin.d/bin score ./cdump.json 0 ./good_learning.dat 

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
