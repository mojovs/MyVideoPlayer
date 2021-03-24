#ifndef MYVIDEOWIDGET_H
#define MYVIDEOWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <mutex>
#include <qglshaderprogram>

#include "videobase.h"
extern "C" {
#include "libavutil/frame.h"
}
class MyVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public VideoBase {
    Q_OBJECT
  public:
    MyVideoWidget(QWidget* parent);
    ~MyVideoWidget();
    /*--处处化材质内存空间--*/
    void Init(int width, int height);
    virtual void Repaint(AVFrame* frame);    //利用传进来的帧进行绘图
  protected:
    void paintGL();                 //刷新显示
    void initializeGL();            //初始化gl
    void resizeGL(int w, int h);    //窗口尺寸大小
  private:
    std::mutex mux;
    QGLShaderProgram program;
    // shader中yuv变量地址
    GLuint unis[3] = {0};
    GLuint texs[3] = {0};
    unsigned char* data[3] = {0};    //内存空间
    int width = 1024;
    int height = 544;
};
#endif    // MYVIDEOWIDGET_H
