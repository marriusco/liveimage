# LIGHTWEIGHT CAMERA MOTION DETECTION, 
# TIME LAPSE and WEB CAM 
# NOT FOR COMMERCIAL PRODUCTS AND COMPANIES. ONLY FOR HOMEUSERS!!! 

#### Streams live right into the  IMG HTML element  &lt; img src='http://liveimage_ip:port/?live' / &gt;.
#### Acceessible direct from browser without additional web server. http://IP:PORT/?html
### Time lapse snapshots 
### Motion Detection
### Life Stream


Light weight camera designed to run on small ARM Linuxes. Plays animated jpg in the browser,
Captures images when senses motion, or at certain interval based on settings. Scripts to 
configure a Linux ARM board equiped with a wifi into an camera wifi access point. 
Check each script before running in. 
Questions, just ask. 



[![Demo Video](https://github.com/comarius/liveimage/blob/master/v4l2n.png?raw=true)](https://youtu.be/gebbErEJj1A)

### Build

```javascript
git clone https://github.com/comarius/liveimage
sudo apt-get install libv4l-dev
sudo apt-get install libjpeg-dev
cd liveimage
cmake .
make
sudo adduser $USER video
# create a folder /data/snaps
sudo mkdir /data/snaps
sudo chown $USER:USER /data/snaps
# eventually mount a separate SDD drive to  /data/snaps
# start liveimage 
./liveimage
```

### Tested on

  - HP x86_64 Linux with incorporated webcam
  - R-PI 3 with USB Camera
  - Nano Pi with USB camera and wifi USB dongle supporting AP
  - C.H.I.P with USB camera. The USB cable was changed to allow external power to USB camera due camera higher amperage
  
  
### Configuration liveimage.conf
The configuration file should be placed in the running folder. The file has only one secion. [main]
The settings are: (ask fo rmore details...)
 
  
#### With Lighttpd server:

make a page
```javascript
  
  <html>
  <head>
  </head>
  <body bgcolor="#E6E6FA">
    <center>
<?php
      echo "<img width='320' src='http://{$_SERVER['HTTP_HOST']}:9000/?image' />";
      echo "<img width='320' src='http://{$_SERVER['HTTP_HOST']}:9000/?motion' /><hr />";
?>
    </center>
  </body>
</html>

```

#### No side apache/lighttpd server

```
=======
#### Without Apache:
    - Write in browser address:
        - http://localhost:90000/?html
