ource /etc/wpa_supplicant/env
#echo $WLAN
ifconfig $WLAN up
sleep 1
systemctl restart wpa_supplicant
sleep 1
dhclient $WLAN
sleep 1
killall liveimage
kill -9 $(pidof liveimage)
cd /home/pi/liveimage/
./liveimage > /tmp/live.log &
sleep 1
oimage="/tmp/liveimage.jpg"
cd /data/snaps
rm /tmp/cronx
TOKEN="---------------------------------"
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
        if [[ -f $image && fsz > 1000 && $oimage != $image ]];then
                sleep .2
                oimage=$image
                echo "saving $oimage"
                curl -F frontcam=@$image -F hdr="Content-Type: image/jpeg" https://meeiot.org/push/$TOKEN
        fi
done
cd /home/pi
reboot

