#pragma once
#include "ui_VideoPlayer.h"
#include <About.h>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

  public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

  signals:
    void playSignal(const QString &fileName,QWidget *videoWidget);

  public slots:
    void on_playBtn_clicked();

  private:
    Ui_VideoPlayer *ui;
};