#!/bin/bash

function wait_user(){
    echo 'your turn'
    read
    ./bin.d/bin score ./jdump.json $turn
    read
}

echo 0 0 > score.dat


./bin.d/bin convert sample_qrformat.dat
time ./bin.d/bin greedy ./cdump.json 0

wait_user

turn=0
for i in `seq 1 8`
do

    for tn in `seq 1 5`
    do
        turn=$(($turn + 1))
        echo ---------------------------------------------
        echo turn $turn
        time ./bin.d/bin greedy ./jdump.json $turn 

        wait_user
        
        ./bin.d/bin gnuscore ./jdump.json $turn  >> score.dat
    done
    
done

./bin.d/bin score ./cdump.json 0 ./learning.dat 

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
