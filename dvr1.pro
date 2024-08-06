QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiooutput.cpp \
    avframequeue.cpp \
    avpacketqueue.cpp \
    decodethread.cpp \
    demuxthread.cpp \
    glyuvwidget.cpp \
    log.cpp \
    main.cpp \
    stream.cpp \
    videooutput.cpp \
    videosurface.cpp \
    widget.cpp

HEADERS += \
    audiooutput.h \
    avframequeue.h \
    avpacketqueue.h \
    avsync.h \
    decodethread.h \
    demuxthread.h \
    glyuvwidget.h \
    log.h \
    queue.h \
    stream.h \
    thread.h \
    videooutput.h \
    videosurface.h \
    widget.h


LIBS += /usr/lib/x86_64-linux-gnu/libavutil.so \
        /usr/lib/x86_64-linux-gnu/libavformat.so \
        /usr/lib/x86_64-linux-gnu/libavdevice.so.58 \
        /usr/lib/x86_64-linux-gnu/libavutil.so \
        /usr/lib/x86_64-linux-gnu/libpostproc.so \
        /usr/lib/x86_64-linux-gnu/libswresample.so \
        /usr/lib/x86_64-linux-gnu/libswscale.so\
        /usr/lib/x86_64-linux-gnu/libavcodec.so \
        /usr/lib/x86_64-linux-gnu/libpthread.so \
        /usr/local/lib/libSDL2.so


FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
