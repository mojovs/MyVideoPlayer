#ifndef MYDEMUX_H
#define MYDEMUX_H

#include <iostream>
#include <mutex>
#define __STDC_CONSTANT_MACROS
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
class MyDemux {
  public:
    int64_t duration = 0;    //流的时长,单位ms
    int width = 0;
    int height = 0;
    int channels = 2;
    int sampleRate = 44100;

    MyDemux();
    virtual ~MyDemux();
    /*--打开 流媒体--*/
    virtual bool Open(const char* url);
    /*--读取出包，注意：内存泄漏问题--*/
    AVPacket* Read();
    /*--只读取视频流的包--*/
    AVPacket* ReadVideoPacket();
    /*--拷贝参数--*/
    AVCodecParameters* CopyVideoParam();    //拷贝视频参数
    AVCodecParameters* CopyAudioParam();    //拷贝音频参数
    /*--位置查找，百分比查找,0-1,跳到的是关键帧--*/
    virtual bool Seek(double pos);
    /*--清理对象--*/
    virtual bool Clean();
    virtual bool Close();

    /*--是否为视频流--*/
    virtual bool isVideo(AVPacket* pkt);
    virtual bool isAudio(AVPacket* pkt);

  protected:
    std::mutex mux;    //为了对fmtCtx进行保护
    std::mutex dmux;
    AVFormatContext* fmtCtx = NULL;
    int vStreamIndex = 0;    //音视频流索引
    int aStreamIndex = 0;
};

#endif    // MYDEMUX_H
