#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(QWidget *parent) : QMainWindow(parent), ui(new Ui_VideoPlayer)
{
    ui->setupUi(this);
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::on_playBtn_clicked()
{
    QMessageBox::about(this, "about", "这是一个简单的消息提示框!!!");
}