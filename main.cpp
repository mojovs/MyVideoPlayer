﻿#include <QApplication>
#include <QDebug>
#include <QThread>
#include <iostream>

#include "audiothread.h"
#include "mainwindow.h"
#include "myaudioplay.h"
#include "mydecode.h"
#include "mydemux.h"
#include "myresample.h"
#include "myvideowidget.h"
#include "ui_mainwindow.h"
#include "videothread.h"
class MyThread : public QThread {
  public:
    MyDemux demux;
    MyVideoWidget* video = NULL;
    AudioThread aThread;
    VideoThread vThread;
    void Init() {
        const char* url = "E:/ffmpeg/ffmpeg-4.3.1/bin/good.mp4";
        /*--解复用--*/
        qDebug() << "demux.Open = " << demux.Open(url);

        /*--视频部分--*/
        qDebug() << "CopyVPara = " << demux.CopyVideoParam() << endl;
        qDebug() << "CopyVPara = " << demux.CopyAudioParam() << endl;
        vThread.Open(demux.CopyVideoParam(), video);
        vThread.start();
        /*--音频部分--*/
        qDebug() << "aThread open:" << aThread.Open(demux.CopyAudioParam());    //打开音频处理线程
        aThread.start();
    }

    void run() {
        for (;;) {
            AVPacket* pkt = demux.Read();
            if (demux.isVideo(pkt))    //视频帧处理
            {                          // cout << "Video:" << frame << endl;
                vThread.Push(pkt);
            } else if (demux.isAudio(pkt))    //音频处理
            {
                aThread.Push(pkt);
            }
            if (!pkt)
                break;
        }
    }
};

int main(int argc, char* argv[]) {
    MyThread mthread;

    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    mthread.video = w.ui->openGLWidget;
    mthread.Init();
    mthread.start();

    return a.exec();
}