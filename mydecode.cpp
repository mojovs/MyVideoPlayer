#include "mydecode.h"

#include "common.h"
extern "C" {
#include "libavutil/error.h"
}
MyDecode::MyDecode() {}
MyDecode::~MyDecode() {}
/*---------------------------------- 打开编码器 ----------------------------------*/
bool MyDecode::Open(AVCodecParameters* param) {
    Close();    //先把该解码器关闭
    if (!param) {
        qDebug() << "param  not exist";
        return false;
    }

    mux.lock();    //给上下文加锁
    /*--根据解码器参数id--*/
    AVCodec* m_codec = avcodec_find_decoder(param->codec_id);
    if (!m_codec) {
        /*--释放内存--*/
        mux.unlock();
        avcodec_parameters_free(&param);
        qDebug() << "find decoder fail";
        return false;
    }
    /*--分配解码器内存--*/
    codecCtx = avcodec_alloc_context3(m_codec);
    avcodec_parameters_to_context(codecCtx, param);    //编码器上下文获取
    qDebug() << codecCtx->sample_fmt;
    qDebug() << codecCtx->sample_rate;
    avcodec_parameters_free(&param);
    /*--设置解码线程为8--*/
    codecCtx->thread_count = 8;

    /*--打开解码器--*/
    int ret = avcodec_open2(codecCtx, 0, NULL);
    if (ret < 0) {
        /*--释放内存--*/
        avcodec_free_context(&codecCtx);

        mux.unlock();    //解锁
        qDebug() << "codec open erro ";

        return false;
    }

    /*--释放内存--*/
    mux.unlock();    //解锁

    return true;
}

/*---------------------------------- 关闭编码器----------------------------------*/
bool MyDecode::Close() {
    mux.lock();
    if (codecCtx) {
        /*--清内存并且释放--*/
        avcodec_close(codecCtx);
        avcodec_free_context(&codecCtx);
    }
    mux.unlock();

    return true;
}

/*---------------------------------- 发一包----------------------------------*/
bool MyDecode::Send(AVPacket* pkt) {
    /*--判断pkt是否为无效包(NULL,空包，没数据)--*/
    if (!pkt || pkt->size <= 0 || NULL == pkt->data) {
        qDebug() << "Packet empty or invalid";
        return false;
    }
    mux.lock();
    if (!codecCtx) {
        mux.unlock();
        qDebug() << "There is no codecCtx";
        return false;
    }
    /*--发送包到缓冲池--*/
    int ret = avcodec_send_packet(codecCtx, pkt);
    if (ret < 0) {
        mux.unlock();    //解锁
        qDebug() << __FILE__ << __LINE__ << "send packet error:"
                 << "codec type is :" << codecCtx->codec_type << av_err2str(ret);
        return false;
    }
    /*--释放掉包--*/
    mux.unlock();
    av_packet_free(&pkt);
    return true;
}

/*---------------------------------- 接受一帧----------------------------------*/
AVFrame* MyDecode::Receive() {
    mux.lock();
    if (!codecCtx) {
        mux.unlock();
        qDebug() << "There is no codecCtx";
        return false;
    }

    AVFrame* frame = av_frame_alloc();
    int ret = avcodec_receive_frame(codecCtx, frame);    //接受一帧内容
    if (ret < 0) {
        qDebug() << "avcodec receive error " << av_err2str(ret);
        qDebug() << "codec_type is " << codecCtx->codec_type;
        mux.unlock();
        av_frame_free(&frame);
        return false;
    }
    mux.unlock();
    return frame;
}

/*---------------------------------- 清理解码器看\----------------------------------*/
bool MyDecode::Clean() {
    mux.lock();
    if (codecCtx) {
        avcodec_flush_buffers(codecCtx);    //刷新缓存
    }
    mux.unlock();

    return true;
}
