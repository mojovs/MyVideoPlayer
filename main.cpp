#include <QApplication>
#include <QLabel>
#include <iostream>

#include "common.h"
#include "mainwindow.h"
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    /*--播放部分--*/
    // const char* url = "";
    //  const char* url = "rtmp://58.200.131.2:1935/livetv/cctv2";		//cctv
    // const char* url = "rtmp://58.200.131.2:1935/livetv/ahtv";    //安徽卫视

    return a.exec();
}
