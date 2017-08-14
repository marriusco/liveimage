#!/bin/bash

[[ ! -d "/data/snaps/" ]] && mkdir -p /data/snaps
chown cocam:cocam /data/snaps
chmod 775 /data/snaps

echo running this bash pid:$$
function exfoo
{
}


trap exfoo SIGUSR2

./liveimage

while [[ 1 ]]; do
    sleep 1
done

kill -9 (pidof liveimage)


