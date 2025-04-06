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
    void playSignal(const QString &fileName, QWidget *videoWidget);

    // 槽函数
  public slots:
    void on_playBtn_clicked();

    void cleanup();

  private:
    Ui_MainWindow *ui;
    QThread *playThread;
    VideoPlayThread *playVideo; // 声明 playVideo 指针
};