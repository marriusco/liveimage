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

    - darklapse=20            Dark average luminance (0 .. 255) when time lapse would stop snapping. 0 to disable
    - darkmotion=10           Dark average luminance when a motion wont be triggered.
    - device=/dev/video0      The device
    - port=9000               TCP Camera server listening port. 0 to disable
    - filename=images/img_%d  Where to save snapsoths. Folder should be precreated. Images are names img_0001 .. img_XXXX, 
                              and would be rolled up to the folder (partition) free space.
    - oneshot=0               When 1 runs, captures and exist
    - quality=80              Quality of the jPG saved and shown in the web
    - motion=20,300           Motion limits (min.max). Max motion is (64*apect-ratio*64), and would never happen. 
                              Motion is triggered when motion pixels are in between min and max values.
                              Test for what are yo ineterested, look at the captures motion data and set the limits acordingly.
                              0,0 to disable motion detection
    - motionnoise=4           Motion noise atenuation (1..16). If camera is noisy increase until under low lighting 
                              wont trigger movement.  Watch /?motion image to tweak.
    - fps=15                  Better be 15
    - motionsnap=200          Interval to capture to sense motion. (50..1000)
    - imagesize=640x480       Camera resolution 
    - timelapse=2000          Iterval for timelapse in ms. 0 to disable  
    - signalin=0              Not used. Send SIGUSR2 to force a capture now.
    - userpid=0               Where to send SIGUSR2 when an image is saved. Last image is allways in /temp/liveimage.jpg
    - httpport=8080           liveimage http port
    - httpip=127.0.0.1        The network IP where liveimage runs. This IP usually.
  
  
### Samples
  
  
#### With Apache/Lighttpd:

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

#### Without Apache:
    - Write in browser address:
        - http://localhost:90000/?html
        
        
        

