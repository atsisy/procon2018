#!/bin/bash

function wait_user(){
    echo 'your turn'
    read
    ./bin.d/bin score ./jdump.json $turn
    read
}

echo 0 0 > score.dat

rm db.bin cdb.bin 

./bin.d/bin db ./sample_qrformat.dat 50 17000

mv db.bin cdb.bin

./bin.d/bin init ./sample_qrformat.dat 0 ./cdb.bin

wait_user

turn=0
for i in `seq 1 $1`
do

    if test $((i % 5)) -eq 0 ; then
        nice -n 13 ./bin.d/bin db ./cdump.json 50 30000 > /dev/null &
    fi
    turn=$(($turn + 1))
    echo ---------------------------------------------
    echo turn $turn
    ./bin.d/bin continue ./jdump.json $turn ./cdb.bin 
        
    wait_user
        
    ./bin.d/bin gnuscore ./jdump.json $turn  >> score.dat

    mv db.bin cdb.bin
    
done

./bin.d/bin score ./cdump.json 0 ./learning.dat 

#/usr/java/default/bin/java -jar ~/IdeaProjects/simple_viewer/out/artifacts/simple_viewer_jar/simple_viewer.jar ./cdump.json
