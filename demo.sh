#!/bin/bash


function ex
{
    echo "file was saved"
}

trap ex SIGUSR1

./liveimage -d /dev/video0 -o filename -s 8080 -s 8080 -m 100 -g $$ &

while [[ 1 ]]; do
    sleep 1
done

kill -9 (pidof liveimage)


