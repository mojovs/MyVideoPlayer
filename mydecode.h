#ifndef MYDECODE_H
#define MYDECODE_H

#include <QDebug>
#include <iostream>
#include <mutex>

extern "C" {
#include "libavcodec/avcodec.h"
}

/*--音频和视频是通用的--*/
class MyDecode {
  public:
    long long pts = 0;    //解码出的帧的pts
    MyDecode();
    virtual ~MyDecode();

    bool is_video = true;                           //默认是视频解码器
    virtual bool Open(AVCodecParameters* param);    //打开流
                                                    //  virtual bool Read();		//读取
    virtual bool Clean();                           //清除缓存
    virtual bool Close();                           //关闭解码器上下文

    /*--发包和收包--*/
    //把一包内容发送到线程缓冲中, pkt的数据内存空间需要清理,不然泄漏了
    virtual bool Send(AVPacket* pkt);
    //从线程缓冲里面收到一个包,注意要释frame，因为返回的帧， 是从内存空间里面复制出来的,因此实际有两份frame空间
    virtual AVFrame* Receive();

  protected:
    AVCodecContext* codecCtx = NULL;
    std::mutex mux;    //锁,保护codec和codecCtx变量
};

#endif    // MYDECODE_H
