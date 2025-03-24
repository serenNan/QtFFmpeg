#include <QObject>
#include <QDebug>
extern "C"
{
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
class VideoPlayThread : public QObject
{
    Q_OBJECT
  public:
    VideoPlayThread(QObject *parent = nullptr);

    // 工作函数
    // 播放视频
    void play(QString filePath,QWidget *videoWidget);

  private:
    QWidget *videoWidget = nullptr;

};