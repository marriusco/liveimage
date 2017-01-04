#!/bin/bash

[[ ! -d "/data/snaps/" ]] && mkdir -p /data/snaps
chown cocam:cocam /data/snaps
chmod 775 /data/snaps

./liveimage -d /dev/video0 -o /data/snaps/img-%06d -s 8080 -m 35,600 -t 4000 -n 20 -k 100
