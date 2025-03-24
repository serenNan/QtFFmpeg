#pragma once
#include "ui_VideoPlayer.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <About.h>
#include <QThread>
#include <QFileDialog>


class VideoPlayer : public QMainWindow
{
    Q_OBJECT

  public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

  public slots:
    void on_playBtn_clicked();
  signals:
    void playSignal(const QString &fileName);

  private:
    Ui_VideoPlayer *ui;
};