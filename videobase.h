#ifndef VIDEOBASE_H
#define VIDEOBASE_H
/*作为视频显示的基类，如果换播放方式，可以继承这个类，这样子类的Init和Repaint接口就被隔离出来了
 *
 *
 */
extern "C" {
#include "libavcodec/codec.h"
}

class VideoBase {
  public:
    virtual void Init(int width, int height) = 0;
    virtual void Repaint(AVFrame* frame) = 0;    //利用传进来的帧进行绘图
};

#endif    // VIDEOBASE_H
