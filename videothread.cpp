#include "videothread.h"

VideoThread::VideoThread() {}

VideoThread::~VideoThread() {}

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

    /*-- decode为空则实例化--*/
    if (NULL == decode)
        decode = new MyDecode();    //创建decode实例
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

void VideoThread::Push(AVPacket* pkt) {
    if (!pkt)
        return;
    /*-- 由于解码的数据比读取的快,如果不阻塞，就容易爆缓冲池--*/
    while (!isExit) {
        m_mux.lock();
        if (pktList.size() < maxListSize) {
            pktList.push_back(pkt);    //把包扔进队列里
            m_mux.unlock();
            break;
        }
        m_mux.unlock();
        msleep(1);    //如果队列大等于100，那么就等1ms.让队列里的包被取走
    }
}

void VideoThread::run() {
    while (!isExit) {
        m_mux.lock();
        if (pktList.empty() || !decode) {    //队列为空,就下个循环
            m_mux.unlock();
            msleep(1);
            continue;
        }
        qDebug() << __FILE__ << __LINE__ << "Video pkt listsize" << pktList.size();
        AVPacket* pkt = pktList.front();    //队列头提出
        pktList.pop_front();                //队列头出栈

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
            }
        }
        m_mux.unlock();
    }
}
