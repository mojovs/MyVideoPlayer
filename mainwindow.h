#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <mydemux.h>
#include <volSliderWidget.h>

#include <QMainWindow>
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

  private:
    Ui::MainWindow* ui;
    volSliderWidget* volWidget = NULL;

    bool isPause = true;        //默认为暂停
    bool isFileOpen = false;    //文件打开了没
    bool isSliderPress = false;
    void setPause(bool isPause);    //设置视频暂停
  private slots:
    void on_btnOpenFile_clicked();
    void on_btnOpenUrl_clicked();

    void on_btnPause_clicked();

    void on_timeSlider_sliderPressed();

    void on_timeSlider_sliderReleased();

    void on_btnSetVol_clicked(bool checked);

    void on_setVol(int value);

  protected:
    void timerEvent(QTimerEvent* event);               //定时器。用来调整滑动条
    void resizeEvent(QResizeEvent* event);             //窗口大小调整事件
    void mouseDoubleClickEvent(QMouseEvent* event);    //窗口双击事件
};

#endif    // MAINWINDOW_H
