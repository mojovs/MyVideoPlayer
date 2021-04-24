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
    long long syncPts = 0;      //同步pts
    long long pts = 0;          //当前播放的pts
    bool isFileOpen = false;    //当前文件打开不

    VideoThread();
    ~VideoThread();
    virtual bool Open(AVCodecParameters* para, VideoBase* video);
    virtual void run();
    virtual void setPause(bool isPuase);
    virtual bool RepaintByPts(AVPacket* pkt, long long ptsNow);

  protected:
    std::mutex m_mutex;    //锁
    VideoBase* video = NULL;
    bool isPause = false;
};

#endif    // VIDEOTHREAD_H
