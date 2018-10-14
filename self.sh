#!/bin/bash

rm db.bin cdb.bin 

./bin.d/bin db ./sample_qrformat.dat 50 16000

mv db.bin cdb.bin

./bin.d/bin init ./sample_qrformat.dat 0 ./cdb.bin
./bin greedy ./cdump.json 0 ./learning2.dat

echo 0 0 > score.dat

turn=0
for i in `seq 1 14`
do
    ./bin.d/bin db ./cdump.json 50 25000 > /dev/null &

    for tn in `seq 1 5`
    do
        turn=$(($turn + 1))
        echo ---------------------------------------------
        echo turn $turn
        ./bin.d/bin continue ./cdump.json $turn ./cdb.bin 
        ./bin greedy ./cdump.json $turn ./learning2.dat
        ./bin.d/bin gnuscore ./cdump.json $turn  >> score.dat
    done

    mv db.bin cdb.bin
    
done

./bin.d/bin score ./cdump.json 0 ./learning.dat 

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
