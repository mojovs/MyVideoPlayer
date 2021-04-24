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
    bool isFileOpen = false;    //文件打开状态

    /*--函数--*/
    AudioThread();
    virtual ~AudioThread();

    virtual bool Open(AVCodecParameters* para);
    void run();
    virtual void Close();    //停止线程时
    virtual void Clean();    //清理缓存

    void setPause(bool isPause);
    virtual void setVol(qreal value);

  protected:
    bool isPause = false;
    std::mutex m_AudioMutex;    //锁

    /*--函数--*/
    MyAudioPlay* audioPlayer = NULL;
    MyResample* resample = NULL;
};

#endif    // QAUDIOTHREAD_H
