#include "demuxthread.h"

demuxThread::demuxThread() {}

bool demuxThread::Open(const char* url, VideoBase* videobase) {
    if (!url || url[0] = '\0' /*判断下字符串有效不*/
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
        std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "DEMUX OPEN FAIL" << std::endl;
        m_mutex.unlock();
        return false;
    }

    success = vThread->Open(demux->CopyVideoParam(), videobase);
    if (!success) {
        std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "VTHREAD OPEN FAIL" << std::endl;
        m_mutex.unlock();
        return false;
    }

    success = aThread->Open(demux->CopyAudioParam());
    if (!success) {
        std::cout << __FILE__ << __FUNCTION__ << __LINE__ << "ATHREAD OPEN FAIL" << std::endl;
        m_mutex.unlock();
        return false;
    }

    m_mutex.unlock();
    return true;
}
