#ifndef QAUDIOTHREAD_H
#define QAUDIOTHREAD_H
/*	该类创建了一个线程,该线程打开音频处理方面的接口。并且创建了队列，把解码数据给推送到队列里面
 * 播放的时候取出
 *
 */
#include <QThread>
#include <mutex>

#include "myaudioplay.h"
#include "mydecode.h"
#include "myresample.h"

class AudioThread : public QThread {
  public:
    AudioThread();
    virtual ~AudioThread();

    virtual bool Open(AVCodecParameters* para);
    virtual void Push(AVPacket* pkt);    //把包推进队列
    void run();

  protected:
    int maxListSize = 100;    // 44100/1024=40多帧，25fps，那么100就是约为2s的缓冲
    bool isExit = false;      //立即退出,防止Push阻塞
    std::list<AVPacket*> pktList;
    std::mutex m_mux;    //锁
    MyDecode* decode = NULL;
    MyAudioPlay* audioPlayer = NULL;
    MyResample* resample = NULL;
};

#endif    // QAUDIOTHREAD_H
