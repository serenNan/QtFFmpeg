#include "VideoPlayer.h"
#include "VideoPlayThread.h"

VideoPlayer::VideoPlayer(QWidget *parent) : QMainWindow(parent), ui(new Ui_VideoPlayer)
{
    // 关于窗口
    ui->setupUi(this);
    connect(ui->aboutBtn, &QPushButton::clicked, this,
            [=]() { QMessageBox::about(this, "about", "这是一个简单的视频播放器"); });

    qDebug() << "主线程播放地址:" << QThread::currentThread();

    // 获取视频文件
    connect(ui->fileBtn, &QPushButton::clicked, this, [=]() {
        QString arg("Text files (*.txt)");
        QString fileName = QFileDialog::getOpenFileName(
            this, "choose video", "/home/serenNan/Projects/QtFFmpeg/video",
            "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv *.ts)", &arg);
        if (!fileName.isEmpty())
        {
            ui->filePath->setText(fileName);
        }
        else
        {
            // 用户取消了文件选择对话框
            QMessageBox::warning(this, "警告", "你没有选择文件");
        }
    });

    // 播放视频的线程
    QThread *playThread = new QThread(this);
    // 工作类对象
    VideoPlayThread *playVideo = new VideoPlayThread;
    playVideo->moveToThread(playThread);

    // 连接 playSignal 信号到 VideoPlayThread 的 play 方法
    connect(this, &VideoPlayer::playSignal, playVideo, &VideoPlayThread::play);

    // 暂停
    connect(ui->pauseBtn, &QPushButton::clicked, playVideo, &VideoPlayThread::pauseVideo);

    // 停止
    connect(ui->stopBtn, &QPushButton::clicked, playVideo, &VideoPlayThread::stopVideo);

    playThread->start();
    // 释放线程
    connect(this, &VideoPlayer::destroyed, this, [=] {
        playVideo->stopVideo();
        playThread->quit();
        playThread->wait();
        playThread->deleteLater();
    });

    // 关闭窗口
    connect(ui->closeBtn, &QPushButton::clicked, this, &VideoPlayer::close);
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::on_playBtn_clicked()
{
    if (playVideo->isPlaying())
    {
        QMessageBox::warning(this, "警告", "视频已经在播放中");
        return;
    }
    // 发送 playSignal 信号
    QString fileName = ui->filePath->text();
    if (fileName.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请先选择文件");
        return;
    }

    if (!ui->videoWidget)
    {
        QMessageBox::warning(this, "警告", "视频窗口为空");
        return;
    }

    emit playSignal(fileName, ui->videoWidget);
}

void VideoPlayer::closeEvent(QCloseEvent *event)
{
    // 停止播放
    playVideo->stopVideo();

    // 等待线程停止
    playThread->quit();
    playThread->wait();

    // 调用基类的 closeEvent
    QMainWindow::closeEvent(event);
}