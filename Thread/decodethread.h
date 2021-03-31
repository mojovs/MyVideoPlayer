#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>
#include <mutex>

#include "mydecode.h"

class DecodeThread : public QThread {
  public:
    std::mutex m_mutex;    //线程
    std::list<AVPacket*> pktList;
    virtual void Close();    //清理线程
    virtual void Clean();

    int maxListSize = 100;    // 44100/1024=40多帧，25fps，那么100就是约为2s的缓冲
    bool isExit = false;      //立即退出,防止Push阻塞

    MyDecode* decode = NULL;

    DecodeThread();
    virtual ~DecodeThread();
    virtual void Push(AVPacket* pkt);    //把包推进队列
    virtual AVPacket* Pop();
};

#endif    // DECODETHREAD_H
