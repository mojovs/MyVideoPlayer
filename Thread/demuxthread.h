#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include <QThread>
#include <mutex>

#include "Thread/audiothread.h"
#include "Thread/videobase.h"
#include "Thread/videothread.h"
#include "mydemux.h"
class DemuxThread : QThread {
  public:
    long long duration = 0;    //视频总时长
    long long pts = 0;         //当前的pts

    DemuxThread();
    ~DemuxThread();

    virtual bool Open(const char* url, VideoBase* videobase);    //打开音视频播放线程接口
    virtual void Close();                                        //关闭线程

    virtual void start();                  //视频音频播放线程，解复用线程给启动起来
    virtual void run() Q_DECL_OVERRIDE;    //

  protected:
    bool isExit = false;    //判断是线程要中断退出
    std::mutex m_mutex;
    MyDemux* demux = 0;
    VideoThread* vThread = 0;
    AudioThread* aThread = 0;
};

#endif    // DEMUXTHREAD_H
