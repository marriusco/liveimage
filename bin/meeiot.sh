#!/bin/bash

whereis=/home/<user>/liveimage

killall liveimage
kill -9 $(pidof liveimage)
cd /home$whereis/
./liveimage > /dev/null &
sleep 1
oimage="/tmp/liveimage.jpg"
cd /data/snaps
rm /tmp/cronx
TOKEN="get a token form https://meeiot.org/?p=start.php"
error=0
while [[ 1 ]];do
        sleep 1
        [[ -f /tmp/cronx ]] && exit

        p=`ping -c 1 192.168.1.1 2>&1 | grep "1 received"`

        if [[ -z ${p} ]];then
                error=$((error+1))
                if (( a > 8 )); then # 8 seconds check
                        break;
                fi
        fi
        error=0
        image=$(ls -rt | tail --lines=1)
        [[ -z $image ]] && continue
        fsz=$(stat -c%s $image)
	echo "chec king $image"
        if [[ -f $image && fsz > 1000 && $oimage != $image ]];then
                sleep .2
                oimage=$image
                echo "saving $oimage"
                curl -F nanocam1=@$image -F hdr="Content-Type: image/jpeg" https://meeiot.org/push/$TOKEN
        fi

	li=$(ps ax | grep liveimage | grep -v meeiot | grep -v grep) 
	if [[ -z $li ]];then
		/home$whereis/cron/resetusb.sh
		break;
	fi

done
cd /home/marius
echo "rebooting"
reboot


