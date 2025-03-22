#include "VideoPlayer.h"

extern "C"
{
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
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

// 线程退出标志
int thread_exit = 0;
// 暂停标志
bool pause = false;

int refresh_video(void *opaque)
{
    thread_exit = 0;
    pause = false;
    while (!thread_exit)
    {
        if (!pause)
        {
            SDL_Event event;
            event.type = REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(40);
    }
    thread_exit = 0;
    pause = false;
    // 推送一个退出主线程的事件
    SDL_Event event;
    event.type = BREAK_EVENT;
    SDL_PushEvent(&event);
    SDL_Delay(40);
    return 0;
}

int ffmpegplayer()
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

    char filepath[] = "../video/Titanic.ts";

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
    SDL_Window *screen;
    // SDL 2.0 Support for multiple windows
    screen_w = pCodecCtx->width;
    screen_h = pCodecCtx->height;
    screen =
        SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!screen)
    {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }
    // 创建一个渲染器，将窗口与渲染器关联
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    Uint32 pixformat = 0;

    // IYUV: Y + U + V  (3 planes)
    // YV12: Y + V + U  (3 planes)
    // 设置像素格式
    // SDL_PIXELFORMAT_IYUV 表示使用 YUV420
    pixformat = SDL_PIXELFORMAT_IYUV;

    // 创建纹理，用于存储视频数据
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING,
                                                pCodecCtx->width, pCodecCtx->height);

    SDL_Rect sdlRect;

    // 创建一个子线程，用于定时触发视频刷新事件
    // 第一个参数是函数指针
    SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video, NULL, NULL);
    SDL_Event event;

    while (1)
    {
        SDL_WaitEvent(&event);
        if (event.type == REFRESH_EVENT)
        {
            while (1)
            {
                if ((av_read_frame(pFormatCtx, packet) < 0))
                    thread_exit = 1;
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
                sdlRect.w = screen_w;
                sdlRect.h = screen_h;

                SDL_RenderClear(sdlRenderer);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
                SDL_RenderPresent(sdlRenderer);
                printf("Succeed to decode 1 frame!\n");
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
            thread_exit = 1;
        }
        // 当接收到退出事件时，退出主循环并释放资源
        // 这个退出事件由子线程提供
        else if (event.type == BREAK_EVENT)
        {
            break;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                pause = !pause;
            }
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                thread_exit = 1;
            }
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

VideoPlayer::VideoPlayer(QWidget *parent) : QMainWindow(parent), ui(new Ui_VideoPlayer)
{
    // 关于窗口
    ui->setupUi(this);
    connect(ui->aboutBtn, &QPushButton::clicked, this,
            [=]() { QMessageBox::about(this, "about", "这是一个简单的视频播放器"); });

    // 关闭窗口
    connect(ui->closeBtn, &QPushButton::clicked, this, &VideoPlayer::close);
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::on_playBtn_clicked()
{
    ffmpegplayer();
}

void VideoPlayer::on_fileBtn_clicked()
{
    QString str;
    str.append(avcodec_configuration());
    QMessageBox::critical(this, "test", str);
}