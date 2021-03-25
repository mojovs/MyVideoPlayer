#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H

#include <QThread>
#include <mutex>

#include "Thread/audiothread.h"
#include "Thread/videobase.h"
#include "Thread/videothread.h"
#include "mydemux.h"
class demuxThread : QThread {
  public:
    demuxThread();
    ~demuxThread();

    virtual bool Open(const char* url, VideoBase* videobase);    //

  protected:
    std::mutex m_mutex;
    MyDemux* demux = 0;
    VideoThread* vThread = 0;
    AudioThread* aThread = 0;
};

#endif    // DEMUXTHREAD_H
