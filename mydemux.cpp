#include "mydemux.h"

#include <QDebug>
#pragma execution_character_set("utf-8")

char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
#define av_err2str(errnum) av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, errnum)

#define int2str(X) (#X)    //将数字直转成字符串
/*-------------分母为0的值，整个值为0*-----------------*/
static double r2d(AVRational r) { return r.den == 0 ? 0 : (double)r.num / (double)r.den; }

MyDemux::MyDemux() {
    static bool is_av_init_first_time = true;    //用来保证库只被初始化一次
    /*--上锁，防止初始化ffmpeg库被打断--*/
    static std::mutex dmux;
    dmux.lock();
    //初始化库
    if (is_av_init_first_time) {
        av_register_all();
        avformat_network_init();

        is_av_init_first_time = false;
    }
    dmux.unlock();
}

MyDemux::~MyDemux() {}
/*--------------------------打开流媒体----------------------------*/
bool MyDemux::Open(const char* url) {
    Close();    //关闭已经存在的fmtCtx，防止冲突
    /*--设置format参数--*/
    AVDictionary* opts = NULL;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);    // rtsp传输方式
    av_dict_set(&opts, "max_delay", "500", 0);         //网络最大延迟

    /*--mux上下文--*/
    mux.lock();    //上锁
    int ret = avformat_open_input(&fmtCtx, url, NULL, &opts);
    if (ret < 0) {
        mux.unlock();
        qDebug() << "Format open input erro: " << av_err2str(ret);
        return false;
    }
    /*--查找流并打印信息--*/
    ret = avformat_find_stream_info(fmtCtx, 0);
    if (ret < 0) {
        mux.unlock();
        qDebug() << "find stream info error: " << av_err2str(ret);
        return false;
    }
    av_dump_format(fmtCtx, 0, url, 0);
    duration = fmtCtx->duration / (AV_TIME_BASE / 1000);    //毫秒

    qDebug() << "duration" << duration / 3600 << ":" << duration % 3600 / 60 << ":" << duration % 3600 % 60;
    ;

    /*--获取视频流号-*/
    vStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream* inputStream = fmtCtx->streams[vStreamIndex];
    width = inputStream->codecpar->width;
    height = inputStream->codecpar->height;
    //打印该流的信息
    qDebug() << "codec_id = " << inputStream->codecpar->codec_id;
    qDebug() << "format = " << inputStream->codecpar->format;

    qDebug() << "<===================================================>";
    qDebug() << vStreamIndex << "视频频信息";
    qDebug() << vStreamIndex << "宽" << inputStream->codecpar->width;
    qDebug() << vStreamIndex << "高" << inputStream->codecpar->height;

    qDebug() << vStreamIndex << "fps " << r2d(inputStream->avg_frame_rate);

    /*--获取频音流号和相关信息--*/
    aStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    inputStream = fmtCtx->streams[aStreamIndex];
    /*--获取音频参数--*/
    sampleRate = inputStream->codecpar->sample_rate;
    channels = inputStream->codecpar->channels;
    qDebug() << "<===================================================>";
    qDebug() << aStreamIndex << "音频信息";
    qDebug() << aStreamIndex << "采样率=" << inputStream->codecpar->sample_rate;
    qDebug() << aStreamIndex << "通道数=" << inputStream->codecpar->channels;
    qDebug() << aStreamIndex << "输出类型=" << inputStream->codecpar->channel_layout;

    mux.unlock();    //整个读取文件流程完毕，解锁

    return true;
}

/*--------------------------读取出包----------------------------*/
AVPacket* MyDemux::Read() {
    mux.lock();    //防止CtxFmt被临时关掉，导致读该模块出问题

    if (!fmtCtx) {
        mux.unlock();
        qDebug() << "AVFORMATContext not exist";
        return NULL;
    }
    AVPacket* pkt = av_packet_alloc();       //分配pkt内存
    int ret = av_read_frame(fmtCtx, pkt);    //读取一包
    if (ret < 0) {
        mux.unlock();
        qDebug() << "Read packet error" << av_err2str(ret);
        //释放对象内存
        av_packet_free(&pkt);
        return NULL;
    }
    /*--pkt的pts dts单位 定为毫秒--*/
    pkt->pts = pkt->pts * r2d(fmtCtx->streams[pkt->stream_index]->time_base) * 1000;
    // qDebug() << "pkt->pts:" << pkt->pts;
    pkt->dts = pkt->dts * r2d(fmtCtx->streams[pkt->stream_index]->time_base) * 1000;
    // qDebug() << "pkt->dts:" << pkt->dts;

    mux.unlock();    //解锁
    return pkt;
}
/*--------------------------拷贝视频参数出来,需要手动free掉----------------------------*/
AVCodecParameters* MyDemux::CopyVideoParam() {
    mux.lock();    //给fmtCtx上锁
    if (!fmtCtx) {
        mux.unlock();
        qDebug() << "cant get AVFormatContex";
        return false;
    }

    AVCodecParameters* param = avcodec_parameters_alloc();    //分配内存
    if (!fmtCtx->streams[vStreamIndex]->codecpar) {
        mux.unlock();
        qDebug() << "cant get codecpar from fmtCtx";
        return false;
    }

    /*--参数拷贝--*/
    int ret = avcodec_parameters_copy(param, fmtCtx->streams[vStreamIndex]->codecpar);
    if (ret < 0) {
        mux.unlock();
        qDebug() << "cant get copy video param ,erro is " << av_err2str(ret);
        return false;
    }
    mux.unlock();    //解锁
    return param;
}

/*--------------------------拷贝音频参数出去,需要手动free掉----------------------------*/
AVCodecParameters* MyDemux::CopyAudioParam() {
    mux.lock();    //给fmtCtx上锁
    if (!fmtCtx) {
        mux.unlock();
        qDebug() << "cant get AVFormatContex";
        return false;
    }

    AVCodecParameters* param = avcodec_parameters_alloc();    //分配内存
    if (!fmtCtx->streams[aStreamIndex]->codecpar) {
        mux.unlock();
        qDebug() << "cant get codecpar from fmtCtx";
        return false;
    }

    /*--参数拷贝--*/
    int ret = avcodec_parameters_copy(param, fmtCtx->streams[aStreamIndex]->codecpar);
    if (ret < 0) {
        mux.unlock();
        qDebug() << "cant get copy audio param ,erro is " << av_err2str(ret);
        return false;
    }
    mux.unlock();    //解锁
    return param;
}

bool MyDemux::Seek(double pos) {
    mux.lock();    //上锁保护fmtCtx
    if (!fmtCtx) {
        mux.unlock();
        qDebug() << "cant get AVFormatContex";
        return false;
    }
    /*--问题解决：出现查找的只是缓冲中的fmtxContex--*/
    avformat_flush(fmtCtx);

    long long real_pos = 0;
    real_pos = fmtCtx->streams[vStreamIndex]->duration * pos;    //利用百分比计算出时间戳
    /*--跳到关键帧I帧--*/
    int ret = av_seek_frame(fmtCtx, vStreamIndex, real_pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    if (ret < 0) {
        mux.unlock();
        qDebug() << "cant seek frame,erro is " << av_err2str(ret);
        return false;
    }
    mux.unlock();
    return true;
}
/*--------------------------清空缓存----------------------------*/
bool MyDemux::Clean() {
    mux.lock();    //上锁保护fmtCtx
    if (!fmtCtx) {
        mux.unlock();
        qDebug() << "cant get AVFormatContex";
        return false;
    }
    avformat_flush(fmtCtx);    //清理
    mux.unlock();
    return true;
}

/*--------------------------清空缓存----------------------------*/
bool MyDemux::Close() {
    mux.lock();    //上锁保护fmtCtx
    if (!fmtCtx) {
        mux.unlock();
        qDebug() << "cant get AVFormatContex";
        return false;
    }
    avformat_close_input(&fmtCtx);    //关闭输入,缓存在这里就不需要清空：w了

    mux.unlock();
    return true;
}

/*--------------------------判断包的类型是否为视频----------------------------*/
bool MyDemux::isVideo(AVPacket* pkt) {
    if (!pkt)
        return false;
    if (pkt->stream_index == vStreamIndex) {
        return true;
    } else
        return false;
}
/*--------------------------判断包的类型是否为音视频----------------------------*/
bool MyDemux::isAudio(AVPacket* pkt) {
    if (!pkt)
        return false;
    if (pkt->stream_index == aStreamIndex) {
        return true;
    } else
        return false;
}
