#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <QImage>
#include <QThread>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "Format_Print.hpp"
#include "tqueue.hpp"
#include <thread>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

struct img_info
{
    QImage img;
    uint64_t pts;
};
using ImgQueue = myThreadQueue<std::optional<img_info>>;
class myCapture : public QObject
{
    Q_OBJECT
private:
    bool inited = false;
    bool opened = false;
    std::string videoPath;
    int videoStreamIdx = -1;
    uint8_t *buffer = nullptr;
    AVFormatContext *pFormatCtx = nullptr;
    AVCodec *pCodec = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVPacket *pPacket = nullptr;
    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;
    SwsContext *pSwsCtx = nullptr;

    PacketQueue packetQueue;
    ImgQueue imgQueue;

    uint64_t stratTime = 0;
    uint64_t clock = 0;

    bool isPause = false;
    QMutex pause_mutex;
    QWaitCondition pause_cond;
    bool isStop = false;
    QMutex stop_mutex;
    QWaitCondition stop_cond;

    double playSpeed = 1;

    void clearQueues();
    void waitThreadsExit();

public:
    myCapture();
    myCapture(std::string path);
    ~myCapture();
    void release();
    bool open(std::string path);
    bool init();
    void read(cv::Mat &img);
    void read(QImage &img);
    void operator>>(cv::Mat &img) { read(img); };
    void operator>>(QImage &img) { read(img); };
    bool isOpening();
    double getFrameRate();
    double synchronizePts(AVFrame *frame, double pts);
    void theardPacket();
    void theardDecode();
    void theardDisplay();

public Q_SLOTS:
    void pauseVideo();
    void resumeVideo();
    void start();
    void stop();
    void setPlaySpeed(double speed);

Q_SIGNALS:
    void sigFrame(QImage img);
};