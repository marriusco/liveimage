#!/bin/bash

[[ -z $1 ]] && echo "pass in the wlan" && exit
WLAN=$1
./resetip.sh $WLAN
sudo iptables -i $WLAN -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j A$
sudo iptables -i $WLAN -A INPUT -p tcp --dport 80 -j ACCEPT
sudo iptables -i $WLAN -A INPUT -p tcp --dport 8080 -j ACCEPT
sudo iptables -i $WLAN -A INPUT -p udp --dport 53 -j ACCEPT
sudo iptables -i $WLAN -A INPUT -p udp --dport 67:68 -j ACCEPT
sudo iptables -i $WLAN -A INPUT -j DROP
sudo  iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination $
sudo  iptables -t nat -A PREROUTING -p tcp --dport 8080 -j DNAT --to-destinatio$
sudo  iptables -t nat -A PREROUTING -p tcp --dport 443 -j DNAT --to-destination$
sudo sh -c "iptables-save > /etc/iptables.rules"
cat /etc/iptables.rules


