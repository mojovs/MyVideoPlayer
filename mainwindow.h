#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <mydemux.h>

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
  private slots:
    void on_btnOpenFile_clicked();
    void on_btnOpenUrl_clicked();

  protected:
    void timerEvent(QTimerEvent* event);      //定时器。用来调整滑动条
    void resizeEvent(QResizeEvent* event);    //窗口大小调整事件
};

#endif    // MAINWINDOW_H
