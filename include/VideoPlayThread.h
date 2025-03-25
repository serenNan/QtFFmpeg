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
    ~VideoPlayThread();

    // 工作函数
    // 播放视频
    void play(QString filePath, QWidget *videoWidget);

    void pauseVideo();

  private:
    std::atomic<bool> pause_flag{false};
    std::atomic<int> thread_exit{0};

    int ffmpegplayer(char file[], QWidget *videoWidget);
    static int refresh_video(void *opaque);
};