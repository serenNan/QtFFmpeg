#pragma once
#include "ui_VideoPlayer.h"
#include <QMainWindow>
#include <QMessageBox>

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

  public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

  public slots:
    void on_playBtn_clicked();

  private:
    Ui_VideoPlayer *ui;
};