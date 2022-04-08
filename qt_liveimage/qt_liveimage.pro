QT -= gui
QT -= core

CONFIG += c++11 console
CONFIG -= app_bundle
QMAKE_LFLAGS += -no-pie

INCLUDEPATH += ./tp/jpeg-6b

SOURCES += \
    ../config.cpp \
    ../jpeger.cpp \
    ../liconfig.cpp \
    ../mainn.cpp \
    ../motion.cpp \
    ../outfilefmt.cpp \
    ../sock.cpp \
    ../sockserver.cpp \
    ../v4ldevice.cpp \
    ../webcast.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../../../var/www/html/camera.php \
    ../../../../var/www/html/camera_old.php \
    ../../../../var/www/html/server.php \
    ../../../../var/www/html/stream.php \
    ../../../../var/www/html/stream2.php \
    ../../../../var/www/html/upload.php \
    ../liveimage.conf \
    ../liveimage.conf \
    ../liveimage.conf \
    ../liveimage.conf_sample

HEADERS += \
    ../config.h \
    ../jpeger.h \
    ../liconfig.h \
    ../motion.h \
    ../os.h \
    ../outfilefmt.h \
    ../sock.h \
    ../sockserver.h \
    ../strutils.h \
    ../v4ldevice.h \
    ../webcast.h


LIBS +=  -lpthread  -lv4l2 -ljpeg
# -lavdevice - -lavformat -lavcodec -lavutil
LIBS += -L$$usr/lib/x86_64-linux-gnu
