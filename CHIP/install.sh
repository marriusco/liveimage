#!/bin/bash

echo "Don't run this on another machien than C H I P ARM Board"
echo "Will install lighttpd, dhcp, hostapd. Make sure you've dist upgrade your C.H.I.P."
echo "Will chnage lighttpd to run under $USER credentials"
echo "WIll enable elan1 as HOSTAPD with address 10.5.5.1, and AP name chip"
echo "Please look at this file before running it and comment what you don't like. Continue ? (y/n)"
read yn
[[ $yn == 'n' ]] && exit

sudo apt-get update
sudo apt-get -y install lighttpd php5-cgi
sudo apt-get -y install --reinstall hostapd
sudo apt-get install dnsmasq
sudo usermod -a -G www-data chip
sudo lighty-enable-mod fastcgi-php
sudo service lighttpd stop
sudo update-rc.d dnsmasq defaults
sudo update-rc.d hostapd disable
sudo update-rc.d lighttpdd defaults


[[ $USER == "root" ]] && echo "run as regular user and pass in password when asked" && exit

for f in $(ls -a *@*);do
	dest=$(echo @"$f" | tr '@' '/')
	if [[ "$f" =~ "etc" ]];then
		echo "installing as root root: $f -> $dest"
		[[ ! -f "$dest".old ]] &&  sudo cp "$dest" "$dest"-old
		sudo cp $f $dest
	else
		echo "installing as $USER: $f -> $dest"
		[[ ! -f "$dest".old ]] && cp "$dest" "$dest"-old
		cp $f $dest
	fi
done

sudo chmod +x /etc/init.d/autostart
sudo update-rc.d autostart remove
sudo update-rc.d autostart defaults
[[ -f /etc/lighttpd/lighttpd.conf ]] && sudo update-rc.d lighttpd defaults
chmod +x $HOME/autostart
chmod +x $HOME/autostart-root


