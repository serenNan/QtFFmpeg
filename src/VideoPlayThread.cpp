#include "VideoPlayThread.h"
#include <MainWindow.h>
#include <QThread>
#include <atomic>

VideoPlayThread::VideoPlayThread(QObject *parent) : QObject(parent), exit_flag(0)
{
}

const int bpp = 12;

int screen_w = 500, screen_h = 500;
const int pixel_w = 320, pixel_h = 180;

unsigned char buffer[pixel_w * pixel_h * bpp / 8];

// SDL_USEREVENT是SDL库中预定义的一个用户事件起始值
// 刷新事件
#define REFRESH_EVENT (SDL_USEREVENT + 1)
// 中断事件
#define BREAK_EVENT (SDL_USEREVENT + 2)
// 暂停事件
#define PAUSE_EVENT (SDL_USEREVENT + 3)

/**
 * @brief 刷新视频
 *
 * @param opaque
 * @return int
 */
int VideoPlayThread ::refresh_video(void *opaque)
{
    VideoPlayThread *instance = static_cast<VideoPlayThread *>(opaque);
    qDebug() << "暂停标志：" << instance->pause_flag.load();
    while (!instance->exit_flag.load(std::memory_order_relaxed))
    {
        if (!instance->pause_flag.load())
        {
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(40);
    }

    return 0;
}

/**
 * @brief 处理播放视频逻辑的函数
 *
 * @param file 播放视频文件
 * @param videoWidget 播放窗口
 * @return int
 */
int VideoPlayThread ::ffmpegplayer(char file[], QWidget *videoWidget)
{
    AVFormatContext *pFormatCtx = NULL;
    int videoindex = -1;
    AVCodecContext *pCodecCtx = NULL;
    const AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL, *pFrameYUV = NULL;
    unsigned char *out_buffer = NULL;
    AVPacket *packet = NULL;
    int ret = 0;
    struct SwsContext *img_convert_ctx = NULL;

    char filepath[1024];
    strcpy(filepath, file);

    FILE *fp_yuv = fopen("output.yuv", "wb+");

    // 初始化FFmpeg库
    avformat_network_init();

    // 打开输入文件
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    // 获取流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }

    // 查找视频流
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }

    if (videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    // 获取解码器
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);
    if (pCodec == NULL)
    {
        printf("Codec not found.\n");
        return -1;
    }

    // 创建解码器上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx)
    {
        printf("Could not allocate video codec context\n");
        return -1;
    }

    // 复制流参数到解码器上下文
    if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar) < 0)
    {
        printf("Could not copy codec parameters to context\n");
        return -1;
    }

    // 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Could not open codec.\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (unsigned char *)av_malloc(
        av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P,
                         pCodecCtx->width, pCodecCtx->height, 1);

    packet = av_packet_alloc();

    // 输出文件信息
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    printf("-------------------------------------------------\n");

    img_convert_ctx =
        sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width,
                       pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    //==================SDL==================
    // 初始化 SDL 库
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    // 创建一个窗口

    // SDL 2.0 Support for multiple windows
    screen_w = pCodecCtx->width;
    screen_h = pCodecCtx->height;
    // 显示在弹出窗口
    // screen =
    //     SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_CENTERED,
    //     SDL_WINDOWPOS_CENTERED,
    //                      screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    // 显示在 Qt 控件上
    if (!videoWidget)
    {
        qDebug() << "视频窗口为空";
        return -1;
    }
    WId winId = videoWidget->winId();
    screen = SDL_CreateWindowFrom((void *)winId);
    if (!screen)
    {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    // 创建一个渲染器，将窗口与渲染器关联
    sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    Uint32 pixformat = 0;

    // IYUV: Y + U + V  (3 planes)
    // YV12: Y + V + U  (3 planes)
    // 设置像素格式
    // SDL_PIXELFORMAT_IYUV 表示使用 YUV420
    pixformat = SDL_PIXELFORMAT_IYUV;

    // 创建纹理，用于存储视频数据
    sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING,
                                   pCodecCtx->width, pCodecCtx->height);

    SDL_Rect sdlRect;

    // 创建一个子线程，用于定时触发视频刷新事件
    // 第一个参数是函数指针
    refresh_thread = SDL_CreateThread(refresh_video, "refresh_video", this);
    SDL_Event event;

    while (!exit_flag)
    {
        if (SDL_PollEvent(&event))
        {
            if (event.type == REFRESH_EVENT)
            {
                if (pause_flag.load())
                    continue;
                while (1)
                {
                    if ((av_read_frame(pFormatCtx, packet) < 0))
                        exit_flag = 1;
                    if (packet->stream_index == videoindex)
                        break;
                }

                ret = avcodec_send_packet(pCodecCtx, packet);
                if (ret < 0)
                {
                    printf("Error sending a packet for decoding\n");
                    return -1;
                }

                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(pCodecCtx, pFrame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0)
                    {
                        printf("Error during decoding\n");
                        return -1;
                    }

                    sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data,
                              pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
                              pFrameYUV->linesize);

                    int y_size = pCodecCtx->width * pCodecCtx->height;
                    // U V 是分量，宽高各压缩一半，所以大小是 Y 的 1/4

                    SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);

                    // FIX: If window is resize
                    sdlRect.x = 0;
                    sdlRect.y = 0;
                    // 动态获取窗口的宽度和高度
                    SDL_GetWindowSize(screen, &sdlRect.w, &sdlRect.h);
                    SDL_RenderClear(sdlRenderer);
                    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
                    SDL_RenderPresent(sdlRenderer);
                    // printf("Succeed to decode 1 frame!\n");
                }

                av_packet_unref(packet);
            }
            // SDL_WINDOWEVENT 当窗口大小改变时，更新屏幕宽度和高度
            else if (event.type == SDL_WINDOWEVENT)
            {
                // window
                SDL_GetWindowSize(screen, &screen_w, &screen_h);
            }
            // 当用户关闭窗口时，设置退出标志使子线程退出
            else if (event.type == SDL_QUIT)
            {
                exit_flag = 1;
            }
            // 当接收到退出事件时，退出主循环并释放资源
            // 这个退出事件由子线程提供
            else if (event.type == BREAK_EVENT)
            {
                break;
            }
        }
        else
        {
            QCoreApplication::processEvents();
            // 短暂休眠以降低CPU占用
            SDL_Delay(1);
        }
    }

    // 刷新解码器
    avcodec_send_packet(pCodecCtx, NULL);
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(pCodecCtx, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
        {
            printf("Error during decoding\n");
            return -1;
        }

        sws_scale(img_convert_ctx, (const unsigned char *const *)pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

        int y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);     // Y
        fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv); // U
        fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv); // V

        printf("Flush Decoder: Succeed to decode 1 frame!\n");
    }

    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    av_packet_free(&packet);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

/**
 * @brief 用于启动播放视频逻辑函数ffmpegplayer
 *
 * @param filePath 播放视频文件的路径
 * @param videoWidget 播放窗口
 */
void VideoPlayThread::play(QString filePath, QWidget *videoWidget)
{
    // 先判断释放有线程正在播放
    if (isPlaying())
    {
        return;
    }
    isPlaying_flag = true;
    qDebug() << "播放线程对象地址：" << QThread::currentThread();
    exit_flag.store(0, std::memory_order_release); // 重置线程退出标志

    // 将视频文件路径转换成可播放的文件格式
    // 将 QString 转换为 std::string
    std::string stdFilePath = filePath.toStdString();
    // 将 std::string 转换为 char*
    const char *cFilePath = stdFilePath.c_str();

    ffmpegplayer(const_cast<char *>(cFilePath), videoWidget);
    qDebug() << "线程执行完毕";
    isPlaying_flag = false;
}

/**
 * @brief 暂停视频
 *
 */
void VideoPlayThread::pauseVideo()
{
    pause_flag = !pause_flag.load();

    qDebug() << "Pause state toggled to:" << pause_flag.load();
}

/**
 * @brief 停止视频
 *
 */
void VideoPlayThread::stopVideo()
{
    exit_flag.store(1, std::memory_order_release); // 通知线程退出
    // 释放SDL资源
    if (sdlTexture)
    {
        SDL_DestroyTexture(sdlTexture);
        sdlTexture = nullptr;
    }
    if (sdlRenderer)
    {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }
    if (screen)
    {
        SDL_DestroyWindow(screen);
        screen = nullptr;
    }
}

/**
 * @brief 判断是否有线程正在播放
 *
 */
bool VideoPlayThread::isPlaying()
{
    qDebug() << "isplaying_flag:" << isPlaying_flag;
    if (isPlaying_flag)
    {
        qDebug() << "有线程正在播放视频";
        return true;
    }
    return false;
}

VideoPlayThread::~VideoPlayThread()
{
    stopVideo();
    // 等待刷新线程结束
    if (refresh_thread)
    {
        SDL_WaitThread(refresh_thread, nullptr);
        refresh_thread = nullptr;
    }
}