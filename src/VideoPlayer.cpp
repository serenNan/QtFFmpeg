#include "VideoPlayer.h"

extern "C"
{
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoPlayer::VideoPlayer(QWidget *parent) : QMainWindow(parent), ui(new Ui_VideoPlayer)
{
    // 关于窗口
    ui->setupUi(this);
    connect(ui->aboutBtn, &QPushButton::clicked, this, [=]() {
        QMessageBox::about(this, "about", "这是一个简单的视频播放器");
    });

    // 关闭窗口
    connect(ui->closeBtn, &QPushButton::clicked, this, &VideoPlayer::close);
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::on_fileBtn_clicked()
{
    QString str;
    str.append(avcodec_configuration());
    QMessageBox::critical(this,"test",str);
}