#!/bin/bash

for i in `seq 1 70`
do
    echo ---------------------------------------------
    echo TURN $i
    read
    ./bin.d/bin score ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json $i
    ./bin.d/bin continue ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/jdump.json $i
done
