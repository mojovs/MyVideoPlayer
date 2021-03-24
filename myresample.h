#ifndef MYRESAMPLE_H
#define MYRESAMPLE_H
#include <mutex>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}

class MyResample {
  public:
    MyResample();
    /*--打开音频--*/
    virtual bool Open(AVCodecParameters* param, bool needCleanParam = false);
    virtual bool Close();
    /*--重采样，并且读取采样后的数据--*/
    virtual int Read(AVFrame* frame, unsigned char* data);

  private:
    std::mutex m_mux;
    SwrContext* swrCtx = NULL;                            //重采样上下文
    AVSampleFormat m_sampleFormat = AV_SAMPLE_FMT_S16;    //采样格式
};

#endif    // MYRESAMPLE_H
