
TARGET = qtliveimage
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11
TEMPLATE = app
DEFINES += cimg_display=0

SOURCES += \
    ../v4ldevice.cpp \
    ../sockserver.cpp \
    ../sock.cpp \
    ../pnger.cpp \
    ../outfilefmt.cpp \
    ../mainn.cpp \
    ../jpeger.cpp \
    ../liconfig.cpp \
    ../config.cpp

DISTFILES += \
    ../makemov.sh \
    ../install-rereq.sh \
    ../demo1.sh \
    ../demo0.sh \
    ../demo.sh \
    ../allowuser.sh \
    ../video.php \
    ../motion.php \
    ../index.php \
    ../image.php \
    ../CMakeLists.txt \
    ../liveimage.conf \
    ../../../../../var/www/html/motion.php

HEADERS += \
    ../v4ldevice.h \
    ../sockserver.h \
    ../sock.h \
    ../pnger.h \
    ../outfilefmt.h \
    ../os.h \
    ../jpeger.h \
    ../liconfig.h \
    ../config.h \
    ../strutils.h

LIBS += -lpthread -lpng -ljpeg -lv4l2
