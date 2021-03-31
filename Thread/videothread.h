#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

/* 负责视频,解码，显示
 *
 */
#include <QThread>
#include <list>
#include <mutex>

#include "decodethread.h"
#include "mydecode.h"
#include "videobase.h"

class VideoThread : public DecodeThread {
  public:
    long long syncPts = 0;    //同步pts
    long long pts = 0;        //当前播放的pts

    VideoThread();
    ~VideoThread();
    virtual bool Open(AVCodecParameters* para, VideoBase* video);

    void run();

  protected:
    std::mutex m_mux;    //锁
    VideoBase* video = NULL;
};

#endif    // VIDEOTHREAD_H
