#include "decodethread.h"

/*------------------------------清理-------------------------*/
void DecodeThread::Close() {
    Clean();
    isExit = true;    //这个需不要被锁值得商榷
    wait();

    m_mutex.lock();
    decode->Close();    //先关闭接口再删除对象
    delete decode;
    decode = NULL;
    m_mutex.unlock();
}

void DecodeThread::Clean() {
    m_mutex.lock();
    decode->Clean();
    while (!pktList.empty())    //清空队列
    {
        AVPacket* pkt = pktList.front();
        if (pkt)    //
            av_packet_free(&pkt);

        pktList.pop_front();
    }
    m_mutex.unlock();
}

DecodeThread::DecodeThread() {
    if (!decode)
        decode = new MyDecode();
}

DecodeThread::~DecodeThread() {
    isExit = true;    //退出标志位为真
}

void DecodeThread::Push(AVPacket* pkt) {
    if (!pkt)
        return;
    /*-- 由于解码的数据比读取的快,如果不阻塞，就容易爆缓冲池--*/
    while (!isExit) {
        m_mutex.lock();
        if (pktList.size() < maxListSize) {
            pktList.push_back(pkt);    //把包扔进队列里
            m_mutex.unlock();
            break;
        }
        m_mutex.unlock();
        msleep(1);    //如果队列大等于100，那么就等1ms.让队列里的包被取走
    }
}

AVPacket* DecodeThread::Pop() {
    m_mutex.lock();
    if (pktList.empty() || !decode) {    //队列为空,就下个循环
        m_mutex.unlock();
        msleep(1);
        return false;
    }
    /*--从队列里面抽出头--*/
    AVPacket* pkt = pktList.front();
    pktList.pop_front();    //从队列里面断开连接
    m_mutex.unlock();
    return pkt;
}
