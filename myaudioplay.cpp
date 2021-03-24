#include "myaudioplay.h"

MyAudioPlay::MyAudioPlay() {}

MyAudioPlay::~MyAudioPlay() {}

/*--获取变量--*/
MyAudioPlay* MyAudioPlay::GetAudioPlay() {
    static AudioPlayer play;    //实例化子类
    return &play;
}
/*----------------------子类--------------------------*/

/*--打开音频设备--*/
bool AudioPlayer::Open() {
    Close();
    /* 设置音频格式 */
    QAudioFormat fmt;
    fmt.setSampleRate(sampleRate);
    fmt.setSampleSize(sampleSize);
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setChannelCount(channels);
    fmt.setCodec("audio/pcm");
    fmt.setSampleType(QAudioFormat::UnSignedInt);

    /* 打开设备 */
    m_mux.lock();    //保护io内部的数据
    audioOutput = new QAudioOutput(fmt);
    io = audioOutput->start();    //开始播放
    m_mux.unlock();

    if (io)
        return true;
    return false;
}

void AudioPlayer::Close() {
    m_mux.lock();
    if (io) {
        io->close();
        io = NULL;    //防止野指针
    }
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = 0;
    }
    m_mux.unlock();
}

bool AudioPlayer::Write(const unsigned char* data, int len) {
    if (!data || len <= 0)
        return false;
    m_mux.lock();
    if (!audioOutput || !io) {
        m_mux.unlock();
        return false;
    }
    /*--数据写入io--*/
    int ret = io->write((const char*)data, len);
    if (ret != len) {
        m_mux.unlock();
        return false;
    }
    m_mux.unlock();
    return true;
}

int AudioPlayer::FreeBufferAvailable() {
    m_mux.lock();
    if (!audioOutput) {
        m_mux.unlock();
        return false;
    }
    /*--获取free的缓冲池大小--*/
    int ret = audioOutput->bytesFree();
    m_mux.unlock();
    return ret;
}
