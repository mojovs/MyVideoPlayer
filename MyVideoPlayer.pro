QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyVideoPlayer
TEMPLATE = app
CONFIG += console # dbg consle


DEFINES += QT_DEPRECATED_WARNINGS \
        #vld	\
        #QT_NO_WARNING_OUTPUT	\
        #QT_NO_DEBUG_OUTPUT

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH +=

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    mydemux.cpp \
    mydecode.cpp \
    myslider.cpp \
    myvideowidget.cpp \
    myresample.cpp \
    myaudioplay.cpp \
    Thread/demuxthread.cpp \
    Thread/audiothread.cpp \
    Thread/demuxthread.cpp \
    Thread/videothread.cpp \
    Thread/decodethread.cpp \
    volSliderWidget.cpp

HEADERS += \
        mainwindow.h \
    mydemux.h \
    mydecode.h \
    myslider.h \
    myvideowidget.h \
    myresample.h \
    common.h \
    myaudioplay.h \
    Thread/demuxthread.h \
    Thread/audiothread.h \
    Thread/demuxthread.h \
    Thread/videobase.h \
    Thread/videothread.h \
    Thread/decodethread.h \
    volSliderWidget.h

FORMS += \
        mainwindow.ui \
        volSliderWidget.ui
FORMS += \
        mainwindow.ui


# ffmpeg
INCLUDEPATH += $$PWD/../ffmpeg-4.3.2/include
DEPENDPATH += $$PWD/../ffmpeg-4.3.2/include
LIBS += -L$$PWD/../ffmpeg-4.3.2/lib -lavformat \
                -lavcodec \
                -lavfilter \
                -lavdevice \
                -lavutil \
                -lpostproc \
                -lswresample \
                -lswscale

# vld detect memory leak
if(contains(DEFINES,vld))
{
INCLUDEPATH += $$PWD/../vld/include
DEPENDPATH += $$PWD/../vld/include
LIBS += -L$$PWD/../vld/lib/Win64 -lvld
}

