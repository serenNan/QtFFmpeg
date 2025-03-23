#include "VideoPlayer.h"
#include "VideoPlayThread.h"

VideoPlayer::VideoPlayer(QWidget *parent) : QMainWindow(parent), ui(new Ui_VideoPlayer)
{
    // 关于窗口
    ui->setupUi(this);
    connect(ui->aboutBtn, &QPushButton::clicked, this,
            [=]() { QMessageBox::about(this, "about", "这是一个简单的视频播放器"); });

    // 关闭窗口
    connect(ui->closeBtn, &QPushButton::clicked, this, &VideoPlayer::close);

    qDebug() << "主线程播放地址:" << QThread::currentThread();

    // 播放视频的线程
    QThread *playThread = new QThread(this);
    // 工作类对象
    VideoPlayThread *play = new VideoPlayThread;
    play->moveToThread(playThread);
    playThread->start();
    connect(ui->playBtn, &QPushButton::clicked, play, &VideoPlayThread::play);

    // 释放线程
    connect(this, &VideoPlayer::destroyed, this, [=] {
        playThread->quit();
        playThread->wait();
        playThread->deleteLater();
    });
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::on_playBtn_clicked()
{
    // ffmpegplayer();
}

void VideoPlayer::on_fileBtn_clicked()
{
    QString str;
    // str.append(avcodec_configuration());
    QMessageBox::critical(this, "test", str);
}