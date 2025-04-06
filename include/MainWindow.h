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

  signals:
    void playSignal(const QString &fileName, QWidget *videoWidget);

  public slots:
    void on_playBtn_clicked();
    void closeEvent(QCloseEvent *event); // 声明 closeEvent 方法
    void cleanup();

  private:
    Ui_MainWindow *ui;
    QThread *playThread;
    VideoPlayThread *playVideo; // 声明 playVideo 指针
};