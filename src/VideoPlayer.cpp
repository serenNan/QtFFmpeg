#include "VideoPlayer.h"

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
