#include <iostream>
#include "tqueue.hpp"
#include <QFile>
#include <QApplication>
#include <QProxyStyle>
#include <QStyle>
#include <QTextStream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QProgressBar>
#include <thread>
#include <opencv2/opencv.hpp>
#include "openVideo.hpp"
#include "videoPlayer.hpp"
using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QFile file(":/ui/style/style.qss");
    if (file.open(QFile::ReadOnly))
    {
        QString style = QTextStream(&file).readAll();
        app.setStyleSheet(style);
    }
    // videoPlayer player;
    // player.show();
    mainWindow win;
    win.show();
    return app.exec();
}
