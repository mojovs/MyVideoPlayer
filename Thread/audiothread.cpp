#include "audiothread.h"

#include "common.h"

AudioThread::AudioThread() {
    isExit = false;
    /*--实例化MyResample --*/
    if (NULL == resample)
        resample = new MyResample();
    /*--音频播放器实例化--*/
    if (NULL == audioPlayer)
        audioPlayer = MyAudioPlay::GetAudioPlay();
}

/*--------------------------------- 打开音频设备的解码、重采样、播放接口 ---------------------------------------*/
AudioThread::~AudioThread() {
    isExit = true;
    wait();    //阻塞等待线程关闭
}

bool AudioThread::Open(AVCodecParameters* para) {
    if (!para)
        return false;
    m_AudioMutex.lock();    //上锁
    pts = 0;

    /*--打开接口--*/
    bool success = resample->Open(para);
    if (!success) {
        m_AudioMutex.unlock();
        qDebug() << "resample open failed";
        return false;
    } /*--打开音频--*/
    success = audioPlayer->Open();
    if (!success) {
        m_AudioMutex.unlock();
        qDebug() << "audio open failed";
        return false;
    }
    /*-打开编码器--*/
    success = decode->Open(para);    //这里可能会对param进行释放
    if (!success) {
        m_AudioMutex.unlock();
        qDebug() << "decode open failed";
        return false;
    }
    if (success)
        qDebug() << "AudioThread open success";
    m_AudioMutex.unlock();    //解锁
    return success;
}

/*--------------------------------- 线程启动---------------------------------------*/
void AudioThread::run() {
    unsigned char* pcmData = new unsigned char[1024 * 1024 * 2];    // 2M
    while (!isExit) {
        AVPacket* pkt = this->Pop();
        m_AudioMutex.lock();
        bool ret = decode->Send(pkt);    //发送包
        if (!ret) {
            m_AudioMutex.unlock();
            qDebug() << __FILE__ << __LINE__ << "decode send failed:" << ret;
            msleep(1);
            continue;
        }

        /*--一包内容可能含有多个帧--*/
        while (!isExit) {
            AVFrame* frame = decode->Receive();    //接受解码后的数据
            if (!frame)
                break;
            /*--同步相关--*/
            int dpts = decode->pts;
            /*--当前解码的pts 减去 因音缓冲导致的延迟时间, 就是正在播放的那一帧的pts--*/
            pts = decode->pts - audioPlayer->getPlayDelay();

            int ret = resample->Read(frame, pcmData);    //读出数据
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

        m_AudioMutex.unlock();
    }
    delete[] pcmData;    //删除数据
}

void AudioThread::Close() {
    DecodeThread::Close();    //父类继承
    if (resample) {
        m_AudioMutex.lock();
        resample->Close();
        delete resample;
        resample = NULL;
        m_AudioMutex.unlock();
    }
    if (audioPlayer) {
        m_AudioMutex.lock();
        audioPlayer->Close();
        audioPlayer = NULL;
        m_AudioMutex.unlock();
    }
}
