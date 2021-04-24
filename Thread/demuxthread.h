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
    long long duration = 0;     //视频总时长
    long long pts = 0;          //当前的pts
    bool isFileOpen = false;    //文件打开状态

    DemuxThread();
    ~DemuxThread();

    virtual bool Open(const char* url, VideoBase* videobase);    //打开音视频播放线程接口
    virtual void Close();                                        //关闭线程
    virtual void Clean();                                        //清理缓存

    virtual void start();                           //视频音频播放线程，解复用线程给启动起来
    virtual void run() Q_DECL_OVERRIDE;             //
    virtual void Seek(double pos);                  //寻找到进度条所在的帧
    virtual void setFileOpenStatus(bool status);    //设置文件打开状态
    void setPause(bool isPause);
    virtual void setVol(int value);    //设置音频音量大小
  protected:
    bool isExit = false;    //判断是线程要中断退出
    std::mutex m_mutex;
    MyDemux* demux = 0;
    VideoThread* vThread = 0;
    AudioThread* aThread = 0;
    bool isPause = false;
};

#endif    // DEMUXTHREAD_H
