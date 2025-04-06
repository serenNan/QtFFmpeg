#include "MainWindow.h"
#include "VideoPlayThread.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui_MainWindow)
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

    // 创建播放视频的线程
    playThread = new QThread(this);
    // 创建工作类对象
    playVideo = new VideoPlayThread;
    playVideo->moveToThread(playThread);

    // 连接 playSignal 信号到 VideoPlayThread 的 play 方法
    connect(this, &MainWindow::playSignal, playVideo, &VideoPlayThread::play);

    // 暂停
    connect(ui->pauseBtn, &QPushButton::clicked, playVideo, &VideoPlayThread::pauseVideo);

    // 停止
    connect(ui->stopBtn, &QPushButton::clicked, playVideo, &VideoPlayThread::stopVideo);

    playThread->start();

    // 关闭窗口
    connect(ui->closeBtn, &QPushButton::clicked, this, &MainWindow::close);

    // 释放线程
    connect(this, &MainWindow::destroyed, this, &MainWindow::cleanup);
}

void MainWindow::cleanup()
{
    // 停止视频
    if (playVideo)
    {
        playVideo->stopVideo();
        playVideo->deleteLater();
        playVideo = nullptr;
    }

    // 退出线程
    if (playThread)
    {
        playThread->quit();
        playThread->wait();
        playThread->deleteLater();
        playThread = nullptr;
    }
}

MainWindow::~MainWindow()
{
    cleanup();
    delete ui;
}

void MainWindow::on_playBtn_clicked()
{
    // 防止重复创建线程
    if (playVideo->isPlaying())
    {
        QMessageBox::warning(this, "警告", "视频已经在播放中");
        return;
    }

    // 防止播放文件为空或播放窗口为空
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

    // 发送播放信号，传递播放文件和播放窗口
    emit playSignal(fileName, ui->videoWidget);
}