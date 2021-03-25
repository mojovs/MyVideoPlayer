#-------------------------------------------------
#
# Project created by QtCreator 2021-03-11T13:35:22
#
#-------------------------------------------------

QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyVideoPlayer
TEMPLATE = app
CONFIG += console

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    mydemux.cpp \
    mydecode.cpp \
    myvideowidget.cpp \
    myresample.cpp \
    myaudioplay.cpp \
    Thread/demuxthread.cpp \
    Thread/audiothread.cpp \
    Thread/demuxthread.cpp \
    Thread/videothread.cpp

HEADERS += \
        mainwindow.h \
    mydemux.h \
    mydecode.h \
    myvideowidget.h \
    myresample.h \
    common.h \
    myaudioplay.h \
    Thread/demuxthread.h \
    Thread/audiothread.h \
    Thread/demuxthread.h \
    Thread/videobase.h \
    Thread/videothread.h

FORMS += \
        mainwindow.ui
FORMS += \
        mainwindow.ui


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
#LIBS += $$PWD/../ffmpeg-4.3.2/lib/avcodec.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/avdevice.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/avfilter.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/avformat.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/avutil.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/postproc.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/swresample.lib \
#        $$PWD/../ffmpeg-4.3.2/lib/swscale.lib

