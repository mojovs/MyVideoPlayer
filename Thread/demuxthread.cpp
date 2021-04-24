#include "demuxthread.h"

#include <QTime>

/*---------------------------------构造-----------------------------------*/
DemuxThread::DemuxThread() { isFileOpen = false; }

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
    if (!success) {
        m_mutex.unlock();
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "DEMUX OPEN FAIL";
        return false;
    }
    duration = demux->duration;    //获取视频时长
    isFileOpen = true;             //文件已经打开
    setFileOpenStatus(isFileOpen);

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

/*---------------------------------清理缓存-----------------------------------*/
void DemuxThread::Clean() {
    // m_mutex.lock();
    if (demux)
        demux->Clean();
    if (aThread)
        aThread->Clean();
    if (vThread)
        vThread->Clean();
    // m_mutex.unlock();
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
        if (isFileOpen == false) {    //如果文件没有打开，那么就循环等待文件打开
            m_mutex.unlock();
            msleep(5);
            continue;
        }

        if (isPause) {
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

void DemuxThread::Seek(double pos) {
    Clean();
    setPause(true);    //跳转之前先暂停
    m_mutex.lock();
    bool status = this->isPause;    //当前暂停状态.

    m_mutex.unlock();

    if (demux) {
        demux->Seek(pos);    //跳到关键帧
    }

    // m_mutex.lock();
    // /*--获取当前的pts--*/
    // long long ptsNow = pos * demux->duration;
    // while (!isExit) {
    //     AVPacket* pkt = demux->ReadVideoPacket();    //找到一帧视频帧
    //     qDebug() << "  set Pause time delay is " << timer.elapsed();
    //     if (!pkt)
    //         break;
    //     if (vThread->RepaintByPts(pkt, ptsNow)) {    //把当前的帧给重新绘制出来
    //         this->pts = pts;
    //         break;
    //     }
    // }
    // m_mutex.unlock();
    //如果暂停了，设置播放
    if (!status)    //如果当前状态为播放，那么就继续播放，如果不是，那么就继续暂停
        setPause(false);
}

void DemuxThread::setFileOpenStatus(bool status) {
    std::mutex dmux;
    dmux.lock();
    isFileOpen = status;
    if (vThread)
        vThread->isFileOpen = status;
    if (aThread)
        aThread->isFileOpen = status;
    dmux.unlock();
}

void DemuxThread::setPause(bool isPause) {
    this->isPause = isPause;    // isPause必须放在锁外，否则，还需要等待别处锁解开
    m_mutex.lock();
    if (vThread) {
        vThread->setPause(isPause);
    }
    if (aThread) {
        aThread->setPause(isPause);
    }
    m_mutex.unlock();    //解锁
}

void DemuxThread::setVol(int value) {}
