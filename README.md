# LIGHTWEIGHT CAMERA MOTION DETECTION, TIME LAPSE and WEB 

Light weight camera designed to run on small ARM linuxes. Plays videos in the browser, Captures images when
senses motion, or at certain interval based on settings. There are some scripts (undocumented) which allows to turn
a Linux ARM board equiped with a wifi into an camera wifi access point. Check each acript before running in.
Questions, just ask. Complex Questions, have a paypal account.

[![Demo Video](https://github.com/comarius/liveimage/blob/master/v4l2n.png?raw=true)](https://youtu.be/gebbErEJj1A)

## Camera streaming live to animated JPG 
### Does not require javascript, 
### Streams live right into the  IMG HTML element  &lt; img src='http://liveimage_ip:port/?live' / &gt;.

### Build

```javascript
git clone https://github.com/comarius/liveimage
sudo apt-get install libv4l-dev
sudo apt-get install libjpeg-dev
cd liveimage
cmake .
make
sudo adduser $USER video

# create a folder images under liveimage
# check liveimage.conf
./liveimage
```

### Tested on

  - HP x86_64 Linux with incorporated webcam
  - R-PI 3 with USB Camera
  - Nano Pi with USB camera and wifi USB dongle supporting AP
  - C.H.I.P with USB camera. The USB cable was changed to allow external power to USB camera due C.H.I.P. USB faulty USB port
  
### Configuration liveimage.conf
The configuration file should be placed in the running folder. The file has only one secion. [main]
The settings are:

    - adsf
        -asdfa
    - asd
    





  
  
  
