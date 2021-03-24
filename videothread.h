#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

/* 负责视频,解码，显示
 *
 */
#include <QThread>
#include <list>
#include <mutex>

#include "mydecode.h"
#include "videobase.h"

class VideoThread : public QThread {
  public:
    VideoThread();
    ~VideoThread();
    virtual bool Open(AVCodecParameters* para, VideoBase* video);
    virtual void Push(AVPacket* pkt);    //把包推进队列
    void run();

  protected:
    int maxListSize = 100;    // 44100/1024=40多帧，25fps，那么100就是约为2s的缓冲
    bool isExit = false;      //立即退出,防止Push阻塞
    std::list<AVPacket*> pktList;
    std::mutex m_mux;    //锁
    MyDecode* decode = NULL;
    VideoBase* video = NULL;
};

#endif    // VIDEOTHREAD_H
