#ifndef MYAUDIOPLAY_H
#define MYAUDIOPLAY_H
#include <QAudioOutput>
#include <mutex>

/*  把接受到的音频数据给播放出来
 *
 */

/*--基类--*/
class MyAudioPlay {
  public:
    int sampleRate = 44100;
    int sampleSize = 16;    //单位为位
    int channels = 2;
    MyAudioPlay();
    virtual ~MyAudioPlay();    //保证子类的父类空间被析构到
    static MyAudioPlay* GetAudioPlay();
    virtual bool Open() = 0;                                       //纯虚:打开播放
    virtual void Close() = 0;                                      //纯虚:关闭播放
    virtual bool Write(const unsigned char* data, int len) = 0;    //纯虚：向音频区里写入数据
    virtual int FreeBufferAvailable() = 0;                         //纯虚：显示缓冲池可用空间
};

class AudioPlayer : public MyAudioPlay {
  public:
    QAudioOutput* audioOutput = NULL;
    QIODevice* io = NULL;
    std::mutex m_mux;

    virtual bool Open();                                       //打开音频设备
    virtual void Close();                                      //关闭音频设备
    virtual bool Write(const unsigned char* data, int len);    //向音频区里写入数据
    virtual int FreeBufferAvailable();                         //显示缓冲池可用空间
};

#endif    // MYAUDIOPLAY_H
