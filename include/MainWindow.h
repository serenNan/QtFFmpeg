#pragma once
#include "ui_MainWindow.h"
#include <About.h>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QMutex>
#include <QPushButton>
#include <QThread>

class VideoPlayThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 信号
  signals:
    void playSignal(const QString &fileName,
                    QWidget *videoWidget); // 播放信号，传递一个播放文件和一个播放窗口

    // 槽函数
  public slots:
    void on_playBtn_clicked(); // 用于发送 playSignal 信号
    void cleanup();            // 清理线程的函数

  private:
    Ui_MainWindow *ui;
    QThread *playThread;        // 播放线程
    VideoPlayThread *playVideo; // 播放工作
};