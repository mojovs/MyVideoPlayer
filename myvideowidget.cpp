#include "myvideowidget.h"

#include <stdio.h>
#include <stdlib.h>

#include <QDebug>
#include <QTime>
#include <QTimer>
#define GET_STR(X) #X
#define A_VER (3)
#define T_VER (4)
extern "C" {
#include "libavformat/avformat.h"
}
/*--定点着色器*/
const char* vStr = GET_STR(attribute vec4 vertexIn; attribute vec2 textureIn; varying vec2 textureOut; void main(void) {
    gl_Position = vertexIn;
    textureOut = textureIn;
});

/*--片段着色器-*/
const char* tStr = GET_STR(varying vec2 textureOut; uniform sampler2D tex_y; uniform sampler2D tex_u;
                           uniform sampler2D tex_v; void main(void) {
                               vec3 yuv;
                               vec3 rgb;
                               yuv.x = texture2D(tex_y, textureOut).r;
                               yuv.y = texture2D(tex_u, textureOut).r - 0.5;
                               yuv.z = texture2D(tex_v, textureOut).r - 0.5;

                               rgb = mat3(1.0, 1.0, 1.0, 0.0, -0.39465, 2.03211, 1.13983, -0.58060, 0.0) * yuv;
                               gl_FragColor = vec4(rgb, 1.0);
                           });
MyVideoWidget::MyVideoWidget(QWidget* parent) : QOpenGLWidget(parent) {}

MyVideoWidget::~MyVideoWidget() {}
void MyVideoWidget::Repaint(AVFrame* frame) {
    mux.lock();
    if (frame == NULL || frame->data[0] == NULL)    //帧为空，或者帧里面，没有数据
    {
        mux.unlock();
        return;
    }
    if (width * height == 0 || frame->width != this->width || frame->height != this->height) {
        av_frame_free(&frame);
        mux.unlock();
        return;
    }
    /*--帧数据拷贝进data--*/
    //    unsigned char *p0 = NULL, *p1 = NULL, *p2 = NULL;
    //    p0 = frame->data[0];
    //    p1 = frame->data[1];
    //    p2 = frame->data[2];
    //    int wrap = frame->linesize[0];
    /*--qtbug:memcpy无法拷贝数据,逐字节拷贝整个过程开销2-3ms--*/
    /*--Y数据逐字节拷贝进材质data里面--*/
    // for (int i = 0; i < frame->height; i++) {
    //    for (int j = 0; j < frame->width; j++) {
    //        data[0][j + i * width] = *(p0 + i * wrap + j);
    //    }
    //}
    ///*--UV数据逐字节拷贝进材质data里面--*/
    // for (int i = 0; i < frame->height / 2; i++) {
    //    for (int j = 0; j < frame->width / 2; j++) {
    //        data[1][j + i * width / 2] = *(p1 + i * wrap / 2 + j);
    //        data[2][j + i * width / 2] = *(p2 + i * wrap / 2 + j);
    //    }
    //}
    if (width == frame->linesize[0])    //如果没有进行数据对齐，紧密型数据
    {
        memcpy(data[0], frame->data[0], width * height);
        memcpy(data[1], frame->data[1], width * height / 4);
        memcpy(data[2], frame->data[2], width * height / 4);
    } else    //
    {
        for (int i = 0; i < height; i++)    // Y
            memcpy(data[0] + width * i, frame->data[0] + frame->linesize[0] * i, width);
        for (int i = 0; i < height / 2; i++)    // U
            memcpy(data[1] + width / 2 * i, frame->data[1] + frame->linesize[1] * i / 2, width / 2);
        for (int i = 0; i < height / 2; i++)    // V
            memcpy(data[2] + width / 2 * i, frame->data[2] + frame->linesize[2] * i / 2, width / 2);
    }

    mux.unlock();
    av_frame_free(&frame);    //释放frame内存
    update();
}

/*--创建图像数据内存空间，创建纹理以及设置纹理相关参数*/
void MyVideoWidget::Init(int width, int height) {
    mux.lock();    //上锁保护texs空间
    this->width = width;
    this->height = height;
    /*--创建内存空间--*/
    /*--清除data里面残留内存--*/
    delete data[0];
    delete data[1];
    delete data[2];
    data[0] = new unsigned char[width * height];
    data[1] = new unsigned char[width * height / 4];
    data[2] = new unsigned char[width * height / 4];
    if (texs[0]) {
        glDeleteTextures(3, texs);
    }

    /*--创建纹理--*/
    glGenTextures(3, texs);
    // Y
    glBindTexture(GL_TEXTURE_2D, texs[0]);
    /*--放大过滤，线性插值--*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*--创建材质显卡的空间--*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    // U
    glBindTexture(GL_TEXTURE_2D, texs[1]);
    /*--放大过滤，线性插值--*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*--创建材质显卡的空间--*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // V
    glBindTexture(GL_TEXTURE_2D, texs[2]);
    /*--放大过滤，线性插值--*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*--创建材质显卡的空间--*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    mux.unlock();
}

/*--------------------------函数重载:初始化-------------------------------*/
void MyVideoWidget::initializeGL() {
    mux.lock();
    qDebug() << "initial";
    /*--初始化openGL函数--*/
    initializeOpenGLFunctions();

    /*-- program加载shader脚本--*/
    /*--片元shader--*/
    qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment, tStr);
    /*--顶点shader--*/
    qDebug() << program.addShaderFromSourceCode(QGLShader::Vertex, vStr);

    /*--设置顶点坐标的变量--*/
    program.bindAttributeLocation("vertexIn", A_VER);
    /*--设置材质坐标的变量--*/
    program.bindAttributeLocation("textureIn", T_VER);
    /*--编译并绑定shader--*/
    qDebug() << program.link();
    qDebug() << program.bind();
    /*--传递顶点和材质的坐标--*/
    /*--顶点坐标--*/
    static const GLfloat ver[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    /*--材质--*/
    static const GLfloat tex[] = {0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    /*--顶点--*/
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
    glEnableVertexAttribArray(A_VER);
    /*--材质--*/
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
    glEnableVertexAttribArray(T_VER);

    /*--从shader程序里面获取材质--*/
    unis[0] = program.uniformLocation("tex_y");
    unis[1] = program.uniformLocation("tex_u");
    unis[2] = program.uniformLocation("tex_v");
    mux.unlock();
}
void MyVideoWidget::paintGL() {
    mux.lock();

    glActiveTexture(GL_TEXTURE0);             //激活零层材质
    glBindTexture(GL_TEXTURE_2D, texs[0]);    //绑定到Y材质
    /*--修改材质的内容--*/
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, data[0]);
    /*--与shader变量关联起来--*/
    glUniform1i(unis[0], 0);    // 0层

    glActiveTexture(GL_TEXTURE0 + 1);         //激活1层材质
    glBindTexture(GL_TEXTURE_2D, texs[1]);    //绑定到U材质
    /*--修改材质的内容--*/
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, data[1]);
    /*--与shader变量关联起来--*/
    glUniform1i(unis[1], 1);    // 1层

    glActiveTexture(GL_TEXTURE0 + 2);         //激活零层材质
    glBindTexture(GL_TEXTURE_2D, texs[2]);    //绑定到V材质
    /*--修改材质的内容--*/
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, data[2]);
    /*--与shader变量关联起来--*/
    glUniform1i(unis[2], 2);    // 0层

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    mux.unlock();
}
void MyVideoWidget::resizeGL(int w, int h) {
    mux.lock();
    qDebug() << "resize";
    mux.unlock();
}
