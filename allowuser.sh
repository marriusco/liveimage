#!/bin/bash
sudo usermod -a -G video $USER
sudo chmod 777 /dev/video0
