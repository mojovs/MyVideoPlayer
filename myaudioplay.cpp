#include "myaudioplay.h"

#include <QDebug>
#include <QTime>
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
    m_mutex.lock();    //保护io内部的数据
    audioOutput = new QAudioOutput(fmt);
    // audioOutput->setBufferSize(512);    //设置缓冲池大小
    io = audioOutput->start();    //开始播放
    m_mutex.unlock();

    if (io)
        return true;
    return false;
}

void AudioPlayer::Close() {
    m_mutex.lock();
    if (io) {
        io->close();
        io = NULL;    //防止野指针
    }
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = 0;
    }
    m_mutex.unlock();
}

void AudioPlayer::Clean() {
    m_mutex.lock();
    if (io)
        io->reset();
    m_mutex.unlock();
}

bool AudioPlayer::Write(const unsigned char* data, int len) {
    if (!data || len <= 0 || data[0] == '\0')
        return false;
    m_mutex.lock();
    if (!audioOutput || !io) {
        m_mutex.unlock();
        return false;
    }
    /*--数据写入io--*/
    int ret = io->write((const char*)data, len);
    if (ret != len) {
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();
    return true;
}

int AudioPlayer::FreeBufferAvailable() {
    m_mutex.lock();
    if (!audioOutput) {
        m_mutex.unlock();
        return false;
    }
    /*--获取free的缓冲池大小--*/
    int ret = audioOutput->bytesFree();
    m_mutex.unlock();
    return ret;
}

long long AudioPlayer::getPlayDelay() {
    m_mutex.lock();

    double pts = 0;
    /*--计算出qt音频缓冲池里面数据还剩下多少*/
    double len = audioOutput->bufferSize() - audioOutput->bytesFree();

    /*1s 数据大小*/
    double size = sampleRate * sampleSize / 8 * channels;
    if (len == 0) {
        m_mutex.unlock();
        return 0;
    }
    if (size == 0) {
        m_mutex.unlock();
        return 0;
    } else {
        pts = len / size * 1000;    //单位为ms
        m_mutex.unlock();
        return pts;
    }
}

void AudioPlayer::setPause(bool isPause) {
    m_mutex.lock();
    this->isPause = isPause;
    if (!audioOutput) {
        m_mutex.unlock();
        return;
    }
    if (isPause) {
        audioOutput->suspend();    //停止播放
    } else {
        audioOutput->resume();    //恢复播放
    }
    m_mutex.unlock();
}

void AudioPlayer::setVol(qreal value) { audioOutput->setVolume(value); }
