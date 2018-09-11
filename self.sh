#!/bin/bash

for i in `seq 1 70`
do
    echo ---------------------------------------------
    ./bin continue ./cdump.json $i
    ./bin.d/bin continue ./cdump.json $i
    echo turn $i
done

./bin.d/bin score ./cdump.json

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
