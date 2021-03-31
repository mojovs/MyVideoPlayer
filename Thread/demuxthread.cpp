#include "demuxthread.h"

/*---------------------------------构造-----------------------------------*/
DemuxThread::DemuxThread() {}

/*---------------------------------析构-----------------------------------*/
DemuxThread::~DemuxThread() {
    isExit = true;
    wait();
}

/*---------------------------------打开解复用，音视频播放线程-----------------------------------*/
bool DemuxThread::Open(const char* url, VideoBase* videobase) {
    if (!url || url[0] == '\0' /*判断下字符串有效不*/
        || !videobase) {
        std::cout << "url not valid or video base NULL" << std::endl;
        return false;
    }

    m_mutex.lock();
    /*--创建变量--*/
    if (!demux)
        demux = new MyDemux();    //实例解复用类
    if (!vThread)
        vThread = new VideoThread();    //实例视频播放线程
    if (!aThread)
        aThread = new AudioThread();    //实例音频播放线程

    /*--打开复用--*/
    bool success = demux->Open(url);
    duration = demux->duration;
    if (!success) {
        m_mutex.unlock();
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "DEMUX OPEN FAIL";
        return false;
    }

    success = vThread->Open(demux->CopyVideoParam(), videobase);
    if (!success) {
        m_mutex.unlock();
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "VTHREAD OPEN FAIL";
        return false;
    }

    success = aThread->Open(demux->CopyAudioParam());
    if (!success) {
        m_mutex.unlock();
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "ATHREAD OPEN FAIL";
        return false;
    }

    m_mutex.unlock();
    return true;
}

void DemuxThread::Close() {
    isExit = true;
    wait();
    if (vThread)
        vThread->Close();
    if (aThread)
        aThread->Close();
    m_mutex.lock();

    delete vThread;
    delete aThread;
    aThread = NULL;
    vThread = NULL;
    m_mutex.unlock();
}

/*---------------------------------启动音视频播放线程-----------------------------------*/
void DemuxThread::start() {
    m_mutex.lock();
    /*--创建变量--*/
    if (!demux)
        demux = new MyDemux();    //实例解复用类
    if (!vThread)
        vThread = new VideoThread();    //实例视频播放线程
    if (!aThread)
        aThread = new AudioThread();    //实例音频播放线程
    /*启动线程*/
    QThread::start();    //启动父类线程
    if (vThread)
        vThread->start();
    if (aThread)
        aThread->start();

    m_mutex.unlock();
}

/*---------------------------------线程运行时-----------------------------------*/
void DemuxThread::run() {
    while (!isExit) {
        m_mutex.lock();
        if (!demux) {    //等待打开解复用
            m_mutex.unlock();
            msleep(5);
            continue;
        }

        /*--pts同步--*/
        if (aThread && vThread) {
            vThread->syncPts = aThread->pts;    //视频线程的同步pts  等于当前播放的音频的pts
            pts = aThread->pts;                 //当前音频pts作为基准pts传出
        }

        /*-读取出包-*/
        AVPacket* pkt = demux->Read();
        if (!pkt) {    //没读取出包就再读
            m_mutex.unlock();
            msleep(5);
            continue;
        }
        if (demux->isVideo(pkt)) {    //视频包
            if (vThread)
                vThread->Push(pkt);    //推送包至队列

        } else if (demux->isAudio(pkt)) {    //音频包

            if (aThread)
                aThread->Push(pkt);    //推送包至队列

        } else {    //如果是字幕或者其他包
        }

        m_mutex.unlock();
    }
}
