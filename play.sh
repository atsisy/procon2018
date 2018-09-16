#!/bin/bash

echo 0 0 > score.dat

echo Type enter key to start...
read
./bin.d/bin init ./sample_qrformat.dat 0
echo your turn
read

./bin.d/bin gnuscore ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json 1 >> score.dat

for i in `seq 2 70`
do
    echo ---------------------------------------------
    echo TURN $i
    ./bin.d/bin score ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json $i
    echo Type enter key
    read
    ./bin.d/bin continue ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json $i
    echo your turn
    read
    ./bin.d/bin gnuscore ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json $i >> score.dat
    
done
