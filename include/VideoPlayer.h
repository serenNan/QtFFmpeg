#pragma once
#include "ui_VideoPlayer.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <About.h>

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

  public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

  public slots:
    void on_fileBtn_clicked();

  private:
    Ui_VideoPlayer *ui;
};