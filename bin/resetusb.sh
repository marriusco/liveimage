#!/bin/bash

set -euo pipefail
IFS=$'\n\t'
#1908:2311
VENDOR="1908"
PRODUCT="2311"

service udev stop
for DIR in $(find /sys/bus/usb/devices/ -maxdepth 1 -type l); do
  if [[ -f $DIR/idVendor && -f $DIR/idProduct &&
        $(cat $DIR/idVendor) == $VENDOR && $(cat $DIR/idProduct) == $PRODUCT ]]; then
    echo 0 > $DIR/authorized
    sleep 1
    echo 1 > $DIR/authorized
  fi
done
sleep 1
service udev start


