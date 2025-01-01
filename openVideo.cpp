#include "openVideo.hpp"
using namespace std;
using namespace cv;

#define capcityOfQueue 20
#define Assert(expr)                                       \
    do                                                     \
    {                                                      \
        if ((expr))                                        \
        {                                                  \
            printf(__CLEAR__                               \
                       __HIGHLIGHT__ __FRED__ #expr "\n"); \
            exit(-1);                                      \
        }                                                  \
    } while (0)
int k = 0;
myCapture::myCapture() : packetQueue(capcityOfQueue), imgQueue(capcityOfQueue)
{
}

myCapture::myCapture(std::string path) : packetQueue(capcityOfQueue), imgQueue(capcityOfQueue)
{
    this->videoPath = path;
    init();
}

myCapture::~myCapture()
{
    cout << "析构" << endl;
    release();
}

void myCapture::release()
{
    std::cout << "release " << std::endl;
    if (this->buffer)
    {
        av_free(this->buffer);
        buffer = nullptr;
    }

    if (this->pPacket)
    {
        av_packet_free(&this->pPacket);
        pPacket = nullptr;
    }

    if (this->pFrame)
    {
        av_frame_free(&this->pFrame);
        pFrame = nullptr;
    }

    if (this->pFrameRGB)
    {
        av_frame_free(&this->pFrameRGB);
        pFrameRGB = nullptr;
    }

    if (this->pCodecCtx)
    {
        avcodec_free_context(&this->pCodecCtx);
        pCodecCtx = nullptr;
    }

    if (this->pFormatCtx)
    {
        avformat_close_input(&this->pFormatCtx);
        pFormatCtx = nullptr;
    }

    if (this->pSwsCtx)
    {
        sws_freeContext(this->pSwsCtx);
        pSwsCtx = nullptr;
    }

    videoStreamIdx = -1;
    inited = false;
    opened = false;
}

bool myCapture::init()
{
    k += 1;
    /*打开文件获取视频流*/
    // 打开视频文件,文件信息会存储在pFormatCtx中
    Assert(avformat_open_input(&pFormatCtx, videoPath.c_str(), nullptr, nullptr) != 0);
    // 获取流信息，包括视频流、音频流等
    Assert(avformat_find_stream_info(pFormatCtx, nullptr) < 0);
    // 遍历所有流，找到视频流
    videoStreamIdx = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIdx = i;
            break;
        }
    }
    cout << "videoStreamIdx: " << videoStreamIdx << endl;
    cout << "视频流数量: " << av_q2d(pFormatCtx->streams[videoStreamIdx]->avg_frame_rate) << endl;
    Assert(videoStreamIdx == -1);
    /*解码文件*/
    // 获取视频流的解码器
    AVCodecParameters *pCodecParam = pFormatCtx->streams[videoStreamIdx]->codecpar;
    AVCodec *pCodec = NULL;
    Assert(!(pCodec = avcodec_find_decoder(pCodecParam->codec_id)));
    // 创建解码器上下文
    Assert(!(pCodecCtx = avcodec_alloc_context3(pCodec)));
    // 复制解码器参数到上下文中
    Assert(avcodec_parameters_to_context(pCodecCtx, pCodecParam) < 0);
    // 初始化解码器
    Assert(avcodec_open2(pCodecCtx, pCodec, nullptr));

    /*初始化转换上下文，将yuv转化到rgb*/
    pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                             pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                             AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    /*分配容器内存*/
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    pPacket = av_packet_alloc();
    //  获取一帧图像的大小
    int bufferSz = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(bufferSz); // 存储一帧图像的内存
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    inited = true;
    opened = true;
    cout << "初始化成功" << endl;
    return true;
}
bool myCapture::open(string path)
{
    // 1. 先停止当前播放
    stop();

    // 2. 等待线程完全退出
    waitThreadsExit();

    // 3. 清理队列
    clearQueues();

    // 4. 释放旧的资源
    if (pFormatCtx)
        release();

    // 5. 设置新路径并初始化
    videoPath = path;
    if (!init())
        return false;

    // 不在这里调用 start()，而是让外部显式调用
    return true;
}

void myCapture::read(Mat &img)
{
    Assert(inited == false);
    if (av_read_frame(pFormatCtx, pPacket) >= 0)
    {
        if (pPacket->stream_index == videoStreamIdx)
        {
            if (avcodec_send_packet(pCodecCtx, pPacket) == 0)
            {
                if (avcodec_receive_frame(pCodecCtx, pFrame) == 0)
                {
                    sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0,
                              pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                    img = Mat(pCodecCtx->height, pCodecCtx->width, CV_8UC3, pFrameRGB->data[0]).clone();
                }
            }
        }
        av_packet_unref(pPacket);
    }
    else
    {
        opened = false;
    }
}

void myCapture::read(QImage &img)
{
    Assert(inited == false);
    if (av_read_frame(pFormatCtx, pPacket) >= 0)
    {
        if (pPacket->stream_index == videoStreamIdx)
        {
            if (avcodec_send_packet(pCodecCtx, pPacket) == 0)
            {
                if (avcodec_receive_frame(pCodecCtx, pFrame) == 0)
                {
                    sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0,
                              pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                    img = QImage((uchar *)pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888).copy();
                }
            }
        }
        av_packet_unref(pPacket);
    }
    else
    {
        opened = false;
    }
}

bool myCapture::isOpening()
{
    return opened;
}

double myCapture::getFrameRate()
{
    Assert(inited == false);
    return av_q2d(pFormatCtx->streams[videoStreamIdx]->time_base);
}

void myCapture::theardPacket()
{
    cout << "数据包线程开始" << endl;
    Assert(inited == false);
    while (av_read_frame(pFormatCtx, pPacket) >= 0)
    {
        {
            QMutexLocker locker(&stop_mutex);
            if (isStop)
            {
                cout << "停止" << endl;
                break;
            }
        }
        if (pPacket->stream_index == videoStreamIdx)
        {
            packetQueue.push(pPacket);
            av_packet_unref(pPacket);
        }
    }
    packetQueue.push(std::nullopt);
    cout << "数据包线程结束" << endl;
}

void myCapture::theardDecode()
{
    cout << "解码线程开始" << endl;
    if (!inited)
        return;
    while (true)
    {
        {
            QMutexLocker locker(&stop_mutex);
            if (isStop)
            {
                cout << "停止" << endl;
                break;
            }
        }
        auto pack = packetQueue.pop();
        if (!pack)
        {
            break;
        }
        if (avcodec_send_packet(pCodecCtx, pack.value()) == 0)
        {
            if (avcodec_receive_frame(pCodecCtx, pFrame) == 0)
            {
                double pts;
                if ((pts = pFrame->best_effort_timestamp) == AV_NOPTS_VALUE)
                    pts = 0;
                pts *= 1e6 * av_q2d(pFormatCtx->streams[videoStreamIdx]->time_base);
                pts = this->synchronizePts(pFrame, pts);
                // pFrame->opaque = &pts;
                // frameQueue.push(pFrame);
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0,
                          pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                QImage img = QImage((uchar *)pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB888).copy();
                img_info info{.img = img, .pts = static_cast<uint64_t>(pts)};
                imgQueue.push(info);
            }
        }
    }
    imgQueue.push(std::nullopt);
    cout << "解码线程结束" << endl;
}

void myCapture::theardDisplay()
{
    cout << "显示线程开始" << endl;
    if (!inited)
        return;
    bool firstFrame = true;
    stratTime = 0;
    uint64_t totalPlayTime = 0;  // 记录总的播放时间
    uint64_t pauseStartTime = 0; // 记录暂停开始的时间
    uint64_t totalPauseTime = 0; // 记录总的暂停时间
    uint64_t lastFramePts = 0;   // 记录上一帧pts
    uint64_t delay = 0;          // 记录播放延迟
    while (true)
    {
        {
            QMutexLocker locker(&stop_mutex);
            if (isStop)
            {
                cout << "停止" << endl;
                break;
            }
        }
        {
            QMutexLocker locker(&pause_mutex);
            if (isPause)
            {
                if (pauseStartTime == 0) // 刚开始暂停
                {
                    pause_cond.wait(&pause_mutex); // 等待恢复
                    pauseStartTime = 0;
                }
            }
        }
        auto frame = imgQueue.pop();
        if (!frame)
            break;
        delay = (frame.value().pts - lastFramePts - totalPauseTime) / playSpeed; // 和上一帧的时间差

        if (delay > 0)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(delay)));
        }
        lastFramePts = frame.value().pts;
        frame.value().pts /= 1e6; // 转化为秒
        emit sigFrame(frame.value());
    }

    cout << "显示线程结束" << endl;
}
double myCapture::start()
{
    // 在创建线程前应该确保所有资源都已经正确初始化
    if (!inited || !opened || !pFormatCtx || !pCodecCtx)
    {
        qDebug() << "视频未正确初始化，无法开始播放";
        return 0;
    }
    double duration = static_cast<double>(pFormatCtx->streams[videoStreamIdx]->duration * av_q2d(pFormatCtx->streams[videoStreamIdx]->time_base));
    // 重置所有状态
    isStop = false;
    isPause = false;
    clearQueues();

    std::thread decode([this]()
                       { this->theardDecode(); });
    std::thread packet([this]()
                       { this->theardPacket(); });
    std::thread display([this]()
                        { this->theardDisplay(); });

    display.detach();
    decode.detach();
    packet.detach();
    return duration;
}

double myCapture::synchronizePts(AVFrame *frame, double pts)
{
    double frameDelay = 0;
    if (pts)
        clock = pts;
    else
        pts = clock;
    frameDelay = av_q2d(pFormatCtx->streams[videoStreamIdx]->avg_frame_rate);
    frameDelay += frame->repeat_pict * (frameDelay * 0.5);
    clock += frameDelay;
    return pts;
}

void myCapture::pauseVideo()
{
    QMutexLocker locker(&pause_mutex);
    isPause = true;
}

void myCapture::resumeVideo()
{
    QMutexLocker locker(&pause_mutex);
    isPause = false;
    pause_cond.wakeAll();
}

void myCapture::stop()
{
    {
        QMutexLocker locker(&stop_mutex);
        if (isStop) // 如果已经停止，直接返回
            return;
        isStop = true;
    }

    // 确保暂停的视频也能正确停止
    {
        QMutexLocker locker(&pause_mutex);
        isPause = false;
        pause_cond.wakeAll();
    }

    // 清理队列，确保线程能够退出
    clearQueues();
}

void myCapture::setPlaySpeed(double speed)
{
    playSpeed = speed;
}

void myCapture::clearQueues()
{
    packetQueue.clear();
    imgQueue.clear();
}

void myCapture::waitThreadsExit()
{
    // 等待一小段时间确保线程完全退出
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}