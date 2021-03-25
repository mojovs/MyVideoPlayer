#include "audiothread.h"

#include "common.h"

AudioThread::AudioThread() { isExit = false; }

/*--------------------------------- 打开音频设备的解码、重采样、播放接口 ---------------------------------------*/
AudioThread::~AudioThread() {
    isExit = true;
    wait();    //阻塞等待线程关闭
}

bool AudioThread::Open(AVCodecParameters* para) {
    if (!para)
        return false;
    m_mux.lock();    //上锁
    /*-- decode为空则实例化--*/
    if (NULL == decode)
        decode = new MyDecode();    //创建decode实例
    /*--实例化MyResample --*/
    if (NULL == resample)
        resample = new MyResample();
    /*--音频播放器实例化--*/
    if (NULL == audioPlayer)
        audioPlayer = MyAudioPlay::GetAudioPlay();
    /*--打开接口--*/
    bool success = resample->Open(para);
    if (!success) {
        m_mux.unlock();
        qDebug() << "resample open failed";
        return false;
    } /*--打开音频--*/
    success = audioPlayer->Open();
    if (!success) {
        m_mux.unlock();
        qDebug() << "audio open failed";
        return false;
    }
    /*-打开编码器--*/
    success = decode->Open(para);    //这里可能会对param进行释放
    if (!success) {
        m_mux.unlock();
        qDebug() << "decode open failed";
        return false;
    }
    if (success)
        qDebug() << "AudioThread open success";
    m_mux.unlock();    //解锁
    return success;
}

/*---------------------------------把包推入队列 ---------------------------------------*/
void AudioThread::Push(AVPacket* pkt) {
    if (!pkt)
        return;
    /*-- 由于解码的数据比读取的快,如果不阻塞，就容易爆缓冲池--*/
    while (!isExit) {
        m_mux.lock();
        if (pktList.size() < maxListSize) {
            pktList.push_back(pkt);    //把包扔进队列里
            m_mux.unlock();
            break;
        }
        m_mux.unlock();
        //  msleep(1);    //如果队列大等于100，那么就等1ms.让队列里的包被取走
    }
}

/*--------------------------------- 线程启动---------------------------------------*/
void AudioThread::run() {
    unsigned char* pcmData = new unsigned char[1024 * 1024 * 10];
    while (!isExit) {
        m_mux.lock();
        if (pktList.empty() || !decode || !resample || !audioPlayer) {    //队列为空,就下个循环
            m_mux.unlock();
            msleep(1);
            continue;
        }
        qDebug() << __FILE__ << __LINE__ << "Audio pkt listsize" << pktList.size();
        AVPacket* pkt = pktList.front();    //队列头提出
        pktList.pop_front();                //队列头出栈

        bool ret = decode->Send(pkt);    //发送包
        if (!ret) {
            m_mux.unlock();
            qDebug() << __FILE__ << __LINE__ << "decode send failed:" << ret;
            msleep(1);
            continue;
        }
        /*--一包内容可能含有多个帧--*/
        while (!isExit) {
            AVFrame* frame = decode->Receive();    //接受解码后的数据
            if (!frame)
                break;
            int ret = resample->Read(frame, pcmData);    //读出数据
            qDebug() << "resample read  frame ,size is" << ret;
            //数据写入播放音频
            while (!isExit) {
                if (ret <= 0)
                    break;                                         //如果读取数据小，直接返回
                if (audioPlayer->FreeBufferAvailable() < ret) {    //缓冲够
                    msleep(1);                                     //缓冲不够就等待
                    continue;
                }
                audioPlayer->Write(pcmData, ret);
                break;
            }
        }

        m_mux.unlock();
    }
    delete pcmData;    //删除数据
}
