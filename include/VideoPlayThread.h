#include <QDebug>
#include <QObject>
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

    void play(QString filePath, QWidget *videoWidget); // 播放视频
    void pauseVideo();                                 // 暂停视频
    void stopVideo();                                  // 停止视频
    bool isPlaying();                                  // 判断视频是否在播放

  private:
    SDL_Thread *refresh_thread = nullptr;
    std::atomic<bool> pause_flag{false}; // 停止标志
    std::atomic<int> exit_flag{0};       // 退出标志

    int ffmpegplayer(char file[], QWidget *videoWidget); // 播放视频的处理函数
    static int refresh_video(void *opaque);              // 刷新函数

    SDL_Window *screen = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;
    SDL_Texture *sdlTexture = nullptr;
    bool isPlaying_flag{false};
};