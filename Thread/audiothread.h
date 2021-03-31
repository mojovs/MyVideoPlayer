#ifndef QAUDIOTHREAD_H
#define QAUDIOTHREAD_H
/*	该类创建了一个线程,该线程打开音频处理方面的接口。并且创建了队列，把解码数据给推送到队列里面
 * 播放的时候取出
 *
 */
#include <QThread>
#include <mutex>

#include "decodethread.h"
#include "myaudioplay.h"
#include "mydecode.h"
#include "myresample.h"

class AudioThread : public DecodeThread {
  public:
    long long pts = 0;

    AudioThread();
    virtual ~AudioThread();

    virtual bool Open(AVCodecParameters* para);
    void run();
    virtual void Close();    //停止线程时

  protected:
    std::mutex m_AudioMutex;    //锁

    MyAudioPlay* audioPlayer = NULL;
    MyResample* resample = NULL;
};

#endif    // QAUDIOTHREAD_H
