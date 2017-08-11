#!/bin/sh

###  run this script to remove X11 and DESKTOPS
apt-get -y autoremove xfce4-*
apt-get -y autoremove xcommon-*
apt-get -y autoremove xorg
apt-get -y autoremove xorg-*
apt-get -y autoremove xserver-*
apt-get -y autoremove x11-*
apt-get -y autoremove x11-common
apt-get -y autoremove xauth
apt-get remove --auto-remove --purge libx11-.*
apt-get remove --auto-remove --purge task-desktop
rm -rf /usr/share/icons/*
rm -rf /usr/share/sounds/
rm -rf /usr/share/squeak/
rm -rf /usr/share/wallpapers
rm -rf /usr/share/themes
rm -rf /usr/share/images/*
apt-get -y autoremove
apt-get -y clean

