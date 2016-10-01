cmake_minimum_required (VERSION 2.6)
PROJECT(liveimage)

###
### sudo apt-get install libpng-dev
### sudo apt-get install libv4l-dev
### sudo apt-get install libjpeg-dev
###

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexceptions -Wall -fpermissive")
add_executable (liveimage  main.cpp  jpeger.cpp outfilefmt.cpp  pnger.cpp  sock.cpp  sockserver.cpp  v4ldevice.cpp)
TARGET_LINK_LIBRARIES( liveimage png jpeg v4l2)
