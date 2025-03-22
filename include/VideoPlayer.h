#pragma once
#include "ui_VideoPlayer.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <About.h>
#include <QThread>

extern "C"
{
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

  public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

  public slots:
    void on_fileBtn_clicked();
    void on_playBtn_clicked();

  private:
    Ui_VideoPlayer *ui;
};