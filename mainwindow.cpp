#include "mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "Thread/demuxthread.h"
#include "common.h"
#include "ui_mainwindow.h"
static DemuxThread dThread;    //解复用

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    dThread.start();
    startTimer(40);

    /*--设置默认url--*/
    ui->lineEdit->setText("rtmp://58.200.131.2:1935/livetv/cctv2");
}

MainWindow::~MainWindow() {
    delete ui;
    dThread.Close();
}

/*-------------------------------------------打开文件-------------------------------------*/
void MainWindow::on_btnOpenFile_clicked() {
    QString path = QDir::currentPath();    //获取当前路径
    QString filter("视频文件(*.mp4);;所有文件(*.*)");
    QString fileName = QFileDialog::getOpenFileName(this, "打开一个视频文件", path, filter);
    if (fileName.isNull()) {
        return;
    }

    if (!dThread.Open(fileName.toLocal8Bit(), ui->openGLWidget)) {
        QMessageBox::information(this, "警告", "打开解复用线程失败");
        return;
    }
}

void MainWindow::on_btnOpenUrl_clicked() {}

/*-------------------------------------------持续获取当前的播放时间-------------------------------------*/
void MainWindow::timerEvent(QTimerEvent* event) {
    long long pts = dThread.pts;
    long long duration = dThread.duration;
    int sliderPos = 0;
    if (0 == duration)    //防止分母无穷大
    {
        sliderPos = 0;
    } else {
        sliderPos = pts / (double)duration * ui->timeSlider->maximum();    //获取当前比例
    }
    ui->timeSlider->setValue(sliderPos);
}
