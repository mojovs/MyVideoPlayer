#include "mainwindow.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "Thread/demuxthread.h"
#include "common.h"
#include "ui_mainwindow.h"
#include "volSliderWidget.h"
static DemuxThread dThread;    //解复用

/*---------------------------------------------构造---------------------------------------------------------*/
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    dThread.start();
    isPause = false;
    startTimer(40);

    /*--设置默认url--*/
    ui->lineEdit->setText("rtmp://58.200.131.2:1935/livetv/cctv2");
    /*--音量调节界面--*/
    volWidget = new volSliderWidget(this);
    connect(volWidget, SIGNAL(setVol(int)), this, SLOT(on_setVol(int)));    //绑定函数
    volWidget->hide();
}

/*---------------------------------------------析构---------------------------------------------------------*/
MainWindow::~MainWindow() {
    delete ui;
    dThread.Close();
}

/*-------------------------------------------打开文件-------------------------------------*/
void MainWindow::on_btnOpenFile_clicked() {
    // QString path = QDir::currentPath();    //获取当前路径
    QString fileName("F:/电视剧电影漫画音乐/爱情公寓5/05.mp4");
    // QString filter("视频文件(*.mp4);;所有文件(*.*)");
    // QString fileName = QFileDialog::getOpenFileName(this, "打开一个视频文件", path, filter);
    if (fileName.isNull()) {
        return;
    }

    if (!dThread.Open(fileName.toLocal8Bit(), ui->openGLWidget)) {
        QMessageBox::information(this, "警告", "打开解复用线程失败");
        return;
    }
    this->isFileOpen = true;    //已经打开了一个视频文件
    dThread.isFileOpen = true;
    /*--设置播放状态--*/
    this->setPause(false);
}

void MainWindow::on_btnOpenUrl_clicked() {}

/*-------------------------------------------持续获取当前的播放时间-------------------------------------*/
void MainWindow::timerEvent(QTimerEvent* event) {
    if (isSliderPress)    //拖动时候停止刷新进度条
        return;
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
/*-------------------------------------------窗口缩放事件-------------------------------------*/
void MainWindow::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event)
    ui->openGLWidget->resize(this->size());    //把视频窗口界面给设置成当前窗口大小
}

/*-------------------------------------------双击全屏-------------------------------------*/
void MainWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    Q_UNUSED(event)
    if (isFullScreen()) {
        this->showNormal();
        ui->timeSlider->show();
    } else {
        this->showFullScreen();
        ui->timeSlider->hide();
    }
}
/*---------------------------------------------设置界面按钮状态---------------------------------------------------------*/
void MainWindow::setPause(bool isPause) {
    if (isPause == true) {
        ui->btnPause->setText("播放");
    } else {
        ui->btnPause->setText("暂停");
    }
}
void MainWindow::on_btnPause_clicked() {
    isPause = !isPause;
    setPause(this->isPause);    //设置界面的暂停按钮显示
    dThread.setPause(isPause);
}

/*-------------------------------------------滑动条按下---------------------------------------------------------*/
void MainWindow::on_timeSlider_sliderPressed() { isSliderPress = true; }

void MainWindow::on_timeSlider_sliderReleased() {
    isSliderPress = false;
    double pos = ui->timeSlider->value() / (double)ui->timeSlider->maximum();
    dThread.Seek(pos);
}

/*-------------------------------------------按钮 音量调节---------------------------------------------------------*/
void MainWindow::on_btnSetVol_clicked(bool checked) {
    volWidget->move(ui->btnSetVol->x() + ui->btnSetVol->width() / 2, ui->btnSetVol->y() - volWidget->height());
    if (checked)    //如果按下，就打开音量滑动条
    {
        volWidget->show();
    } else {
        volWidget->setVisible(checked);
    }
}

void MainWindow::on_setVol(int value) { QMessageBox::information(this, "提示", QString::asprintf("值是:%d", value)); }
