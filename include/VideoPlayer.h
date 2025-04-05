#pragma once
#include "ui_VideoPlayer.h"
#include <About.h>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>
#include <QMutex>

class VideoPlayThread;

class VideoPlayer : public QMainWindow
{
    Q_OBJECT

  public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

  signals:
    void playSignal(const QString &fileName, QWidget *videoWidget);

  public slots:
    void on_playBtn_clicked();
    void closeEvent(QCloseEvent *event); // 声明 closeEvent 方法

  private:
    Ui_VideoPlayer *ui;
    QThread *playThread;
    VideoPlayThread *playVideo; // 声明 playVideo 指针
};