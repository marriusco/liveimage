# liveimage  (Animated JPG, Animated PNG)

## Camera streamingive to animated JPG or PNG . 
### Does not require javascript, 
### Does not require video tags and or any other playback controls. 
### Streams live right into the  IMG HTML element  &lt; img src='http://liveimage_ip:port' / &gt;.

```diff
- <!-- create an image tag. That's it-->
- <img src='IP_OF_liveimage:PORT'>


```javascript
clone the repo
install prerequisites
sudo apt-get install libpng-dev
sudo apt-get install libv4l-dev
sudo apt-get install libjpeg-dev
cd liveimage
cmake .
make
sudo adduser $USER video
#run
./liveimage
-d Video device '/dev/video#'. Default  /dev/video0. Add user to video group!!!
-s Http server port.
-o Output filename, no extension (extension added by format [-f]). 
-i jpg|png Image format. Default jpg
-q NN JPEG quality (0-100). Default 90%
-z WxH Image width and height. Could be adjusted. Default 640x480
-f FFF frames per second. For 0 saves one snapshot then exits. Values 1-100 
-n NNN At how many frames [-f] to save a snapshot
```

### Preview

  - To preview a video as animated JPEG in a web page at localhost at address 8080 run as:
    - ./liveimage -d /dev/video0 -s 8080 -i jpg
  - Then open the browser and open the image.html  file
  
  - To preview a video as animated PNG in a web page at localhost at address 8080 run as:
    - ./liveimage -d /dev/video0 -s 8080 -i png
  - Then open the browser and open the image.html  file
  
  - For timelapse  snapshots, and have the live previev at the same time use -f and -n flags
  - Next would preview at 30 fps and would save an snapsot in filename.jpg each 60 frames (2 seconds)
    - ./liveimage -d /dev/video0 -s 8080 -i jpg -o filename -f 30 -n 60
 
### PHP

If you have Apache, Nginx or Lighttpd running, to add a live preview to a php page
use following approach:
   - have liveimage running, install it in sysV as a service.
    
```javascript

<?php
     echo "<img src='http://{$_SERVER['SERVER_ADDR']}:8080' />";
?>

```

[![Demo Video](https://github.com/comarius/liveimage/blob/master/v4l2n.png?raw=true)(https://github.com/comarius/liveimage/blob/master/Docs/liveimage.mov?raw=true)



### Automating

'liveimage -d /dev/video0 -s 8080 -i jpg -o filename -f 30 -n 60' would save every 2 seconds a snapsot called filename.jpg. If the filename requires post-processing the '-g' flag is the saviour. Passing a process id via -g, liveimage would signal that process by SIGUSR1. Following bash shows the concept.

```javascript
#!/bin/bash
function ex
{
    echo "file was saved"
    # move, rename file or post porcess the file
}

trap ex SIGUSR1

# run in bg so it can fire signals here
./liveimage -d /dev/video1 -o filename -s 8080 -g $$ &

# stay here to trap the process
while [[ 1 ]]; do
    sleep 1
done
kill -9 (pidof liveimage)

```





Marius C. Developed on the Last week of September, 2016
Credits: jpegstreamer, motion, imagemagic, v4l2grab 
