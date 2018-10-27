#!/bin/bash

function wait_user(){
    echo 'your turn'
    read
    ./bin.d/bin score ./jdump.json $turn
    read
}

echo 0 0 > score.dat

rm db.bin cdb.bin 

./bin.d/bin db ./sample_qrformat.dat 25 15000

mv db.bin cdb.bin


./bin.d/bin convert sample_qrformat.dat
time ./bin.d/bin greedy ./cdump.json 0 ./cdb.bin

wait_user

turn=0
for i in `seq 1 4`
do
    ./bin.d/bin db ./jdump.json 25 27000 > /dev/null &

    for tn in `seq 1 5`
    do
        turn=$(($turn + 1))
        echo ---------------------------------------------
        echo turn $turn
        time ./bin.d/bin mt ./jdump.json $turn ./cdb.bin 

        wait_user
        
        ./bin.d/bin gnuscore ./jdump.json $turn  >> score.dat
    done

    mv db.bin cdb.bin
    
done

./bin.d/bin score ./cdump.json 0 ./learning.dat 

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
