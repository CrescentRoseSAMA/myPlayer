#include "videoPlayer.hpp"
using namespace std;
int mouseTime = 0;
string path = "../mad.mp4";
videoPlayer::videoPlayer(QWidget *parent) : QWidget(parent), minSize(450, 450), maxSize(1870, 1030), showAera(0, 0, 500, 500)
{
    setMaximumSize(maxSize);
    setMinimumSize(minSize);
    // this->setStyleSheet("background-color:rgba(125, 125, 125, 100);");
    playButton = new QPushButton();
    playButton->setFixedSize(80, 80);
    playButton->setObjectName("play");
    pauseButton = new QPushButton();
    pauseButton->setFixedSize(80, 80);
    pauseButton->setObjectName("pause");
    stopButton = new QPushButton();
    stopButton->setObjectName("stop");
    stopButton->setFixedSize(80, 80);
    selectButton = new QPushButton();
    selectButton->setFixedSize(80, 80);
    selectButton->setObjectName("select");

    QHBoxLayout *layout = new QHBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout();
    buttonBar = new QFrame();
    buttonBar->setObjectName("bottomBar");
    buttonBar->setFixedHeight(80);
    buttonBar->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(playButton, 0, Qt::AlignCenter);
    layout->addWidget(pauseButton, 0, Qt::AlignCenter);
    layout->addWidget(selectButton, 0, Qt::AlignCenter);
    vlayout->addWidget(buttonBar, 0, Qt::AlignBottom);
    this->setLayout(vlayout);
    menuBar = new QMenuBar(this);
    menuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto menu = menuBar->addMenu("speed");
    auto action0 = menu->addAction("0.5x");
    action0->setObjectName("s05");
    auto action1 = menu->addAction("1x");
    action1->setObjectName("s1");
    auto action2 = menu->addAction("2x");
    action2->setObjectName("s2");
    timer.setObjectName("timer");
    timer.start(100);
    connect(&cap, &myCapture::sigFrame, this, &videoPlayer::mshow);
    connect(pauseButton, &QPushButton::clicked, &cap, &myCapture::pauseVideo);
    connect(playButton, &QPushButton::clicked, &cap, &myCapture::resumeVideo);
    connect(&timer, &QTimer::timeout, this, &videoPlayer::on_timer_timeout);

    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setWindowFlags(Qt::FramelessWindowHint);
    QMetaObject::connectSlotsByName(this);
}

videoPlayer::~videoPlayer()
{
}

void videoPlayer::setVideo(string path)
{

    if (cap.open(path))
    {
        cap.start(); // 添加这行，确保视频开始播放
    }
    else
    {
        QMessageBox::warning(this, "错误", "无法打开视频文件");
    }
}

void videoPlayer::on_select_clicked()
{
    string path = QFileDialog::getOpenFileName(this, "Open Video", "../", "Video Files (*.mp4 *.avi *.flv *.mkv *.wmv)").toStdString();
    setVideo(path);
}
void videoPlayer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!frame.isNull())
    {
        QImage scaledFrame = frame.scaled(showAera.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int x = showAera.x() + (showAera.width() - scaledFrame.width()) / 2;
        int y = showAera.y() + (showAera.height() - scaledFrame.height()) / 2;

        painter.drawImage(x, y, scaledFrame);
    }
}
void videoPlayer::resizeEvent(QResizeEvent *event)
{
    showAera = QRect(0, 0, event->size().width(), event->size().height());
    update();
    QWidget::resizeEvent(event);
}
void videoPlayer::mshow(QImage img)
{
    if (showAera.size().isValid() && !img.isNull())
    {
        this->frame = img.scaled(showAera.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        this->frame = img;
    }
    update();
}

void videoPlayer::on_s05_triggered()
{
    cap.setPlaySpeed(0.5);
}

void videoPlayer::on_s1_triggered()
{
    cap.setPlaySpeed(1);
}

void videoPlayer::on_s2_triggered()
{
    cap.setPlaySpeed(2);
}

void videoPlayer::on_timer_timeout()
{
    mouseTime += 100;
    if (mouseTime > 3000 && buttonBarVisible)
    {
        this->setCursor(Qt::BlankCursor);
        this->buttonBar->hide();
        this->menuBar->hide();
        buttonBarVisible = false;
    }
}

void videoPlayer::mouseMoveEvent(QMouseEvent *event)
{
    mouseTime = 0;
    if (!buttonBarVisible)
    {
        this->setCursor(Qt::ArrowCursor);
        this->buttonBar->show();
        this->menuBar->show();
        buttonBarVisible = true;
    }
    QWidget::mouseMoveEvent(event);
}