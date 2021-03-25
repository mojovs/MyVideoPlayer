#include "myresample.h"

#include <QDebug>

#include "common.h"

MyResample::MyResample() {}

/*-------------------------------打开音频------------------------------------*/
bool MyResample::Open(AVCodecParameters* param, bool needCleanParam) {
    if (!param)
        return false;
    m_mux.lock();    //上锁
    channels = param->channels;
    qDebug() << "RESAMPLE Param->sample_rate" << param->sample_rate;
    swrCtx = swr_alloc_set_opts(swrCtx, av_get_default_channel_layout(2), m_sampleFormat, param->sample_rate,
                                av_get_default_channel_layout(channels), (AVSampleFormat)param->format,
                                param->sample_rate, 0, 0);
    if (needCleanParam)
        avcodec_parameters_free(&param);
    int ret = swr_init(swrCtx);    ///初始化重采样
    if (ret < 0) {
        m_mux.unlock();
        qDebug() << "sws init error:" << av_err2str(ret);
        return false;
    }

    m_mux.unlock();
    return true;
}

bool MyResample::Close() {
    m_mux.lock();
    if (!swrCtx) {
        m_mux.unlock();
        return false;
    }
    swr_free(&swrCtx);
    swr_close(swrCtx);
    m_mux.unlock();
}

int MyResample::Read(AVFrame* frame, unsigned char* outdata) {
    if (!frame)
        return -1;
    m_mux.lock();
    if (!outdata) {
        m_mux.unlock();
        av_frame_free(&frame);
        return 0;
    }
    unsigned char* data[2] = {0};    //由于是双通道
    data[0] = outdata;
    /*--重采样之后，数据 输出--*/
    qDebug() << "nb-samples" << frame->nb_samples;
    /*--数据转换 ,此处aac格式ret为1024byte--*/
    int ret = swr_convert(swrCtx, data, frame->nb_samples, (const unsigned char**)frame->data, frame->nb_samples);
    if (ret <= 0) {
        m_mux.unlock();
        qDebug() << "swr convert failed: %s" << av_err2str(ret);
        return -1;
    }
    int size = ret * 2 * av_get_bytes_per_sample(m_sampleFormat);    //返回重采样过后的数据大小
    m_mux.unlock();

    return size;
}
