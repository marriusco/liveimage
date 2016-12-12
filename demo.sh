#!/bin/bash

[[ ! -d "/data/snaps/" ]] && mkdir -p /data/snaps
chown cocam:cocam /data/snaps
chmod 775 /data/snaps


./liveimage -d /dev/video0 -o /data/snaps/img-%05d -s 8080 -m 20,300 -T 500  



