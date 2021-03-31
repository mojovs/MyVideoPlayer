#include "videothread.h"

#include <QTime>
#pragma execution_character_set("utf-8")

VideoThread::VideoThread() {}

VideoThread::~VideoThread() {}
/*--打开视频解码器，初始化播放窗口--*/
bool VideoThread::Open(AVCodecParameters* para, VideoBase* video) {
    if (!para)
        return false;
    m_mux.lock();    //上锁

    /*--视频初始化界面--*/
    this->video = video;    //给VideoBase类赋值
    if (video) {
        video->Init(para->width, para->height);    //初始化视频界面
    } else {
        qDebug() << "VideoBase is NULL";
    }

    /*-打开解码器--*/
    int success = decode->Open(para);    //这里可能会对param进行释放
    if (!success) {
        m_mux.unlock();
        qDebug() << "decode open failed";
        return false;
    }
    if (success)
        qDebug() << "Video open success";
    m_mux.unlock();    //解锁
    return success;
}

void VideoThread::run() {
    while (!isExit) {
        m_mux.lock();
        /*--播放pts大于同步pts-*/
        if (syncPts > 0 && syncPts < decode->pts) {    //目前刚解码出来的视频帧的pts 比同步pts(当前播放的音频帧)快,等待
            m_mux.unlock();
            msleep(1);
            continue;
        }
        AVPacket* pkt = this->Pop();     //从队列里获取数据
        bool ret = decode->Send(pkt);    //发送包
        if (!ret) {
            m_mux.unlock();
            qDebug() << __FILE__ << __LINE__ << "decode send failed:" << ret;
            msleep(1);
            continue;
        }
        /*--一包内容可能含有多个帧--*/
        while (!isExit) {
            AVFrame* frame = decode->Receive();    //接受解码后的数据
            if (!frame)
                break;
            //数据写入界面
            if (video) {
                video->Repaint(frame);    // frame内存在此被释放
                                          // qDebug() << "绘图用时" << time.elapsed();
            }
        }
        m_mux.unlock();
    }
}
