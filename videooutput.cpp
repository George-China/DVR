
#include "videooutput.h"
#include "log.h"
#include <thread>
#include<iostream>
#include<string>
VideoOutput::VideoOutput(AVSync *avsync, AVRational time_base, AVFrameQueue *frame_queue, int video_width, int video_height)
    : avsync_(avsync), time_base_(time_base),
      frame_queue_(frame_queue), video_width_(video_width), video_height_(video_height)
{

}

int VideoOutput::Init(void * winId)
{
    if(SDL_Init(SDL_INIT_VIDEO))  {
        LogError("SDL_Init failed");
        return -1;
    }
    win_ = SDL_CreateWindowFrom(winId);

    if(!win_) {
        LogError("SDL_CreateWindow failed");
        return -1;
    }

    renderer_ = SDL_CreateRenderer(win_, -1, 0);
    if(!renderer_) {
        LogError("SDL_CreateRenderer failed");
        return -1;
    }
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_width_, video_height_);
    if(!texture_) {
        LogError("SDL_CreateRenderer failed");
        return -1;
    }

    yuv_buf_size_ = video_width_ * video_height_ * 1.5;
    yuv_buf_ = (uint8_t *)malloc(yuv_buf_size_);
    return 0;

}

int VideoOutput::Init()
{
    if(SDL_Init(SDL_INIT_VIDEO))  {
        LogError("SDL_Init failed");
        return -1;
    }

    win_ = SDL_CreateWindow("player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            video_width_, video_height_, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!win_) {
        LogError("SDL_CreateWindow failed");
        return -1;
    }

    renderer_ = SDL_CreateRenderer(win_, -1, 0);
    if(!renderer_) {
        LogError("SDL_CreateRenderer failed");
        return -1;
    }
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_width_, video_height_);
    if(!texture_) {
        LogError("SDL_CreateRenderer failed");
        return -1;
    }

    yuv_buf_size_ = video_width_ * video_height_ * 1.5;
    yuv_buf_ = (uint8_t *)malloc(yuv_buf_size_);
    return 0;

}


// VIDEO目前是在主线程输出
int VideoOutput::MainLoop()
{
    SDL_Event event;
    while (true) {
        // 读取事件
        RefreshLoopWaitEvent(&event);

        switch (event.type) {
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE) {
                LogInfo("esc key down");
                return 0;
            }
            break;
        case SDL_QUIT:
            LogInfo("SDL_QUIT");
            return 0;
            break;
        default:
            break;
        }
    }
}

#define REFRESH_RATE 0.01
void VideoOutput::RefreshLoopWaitEvent(SDL_Event *event)
{
    double remaining_time = 0.0;
    SDL_PumpEvents();
    while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        if (remaining_time > 0.0)
            std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(remaining_time * 1000.0)));
        remaining_time = REFRESH_RATE;
        // 尝试刷新画面
        videoRefresh(&remaining_time);
        SDL_PumpEvents();
    }
}
// 0.01秒
void VideoOutput::videoRefresh(double *remaining_time)
{
    AVFrame *frame = NULL;
    frame = frame_queue_->Front(); //获取准备要渲染的帧， 这里不出队列
    std::cout<< frame <<std::endl;
    if(frame) {
        double pts = frame->pts * av_q2d(time_base_); // 视频帧的时间戳

        LogInfo("video pts:%0.3lf\n", pts);

        double diff = pts - avsync_->GetClock();  // 对比音频时钟， 复杂的音视频同步 ffplay B站ijkplayer

        if(diff > 0) {
            *remaining_time = FFMIN(*remaining_time, diff);
            return;
        }

        // 有就渲染
        rect_.x = 0;
        rect_.y = 0;
        rect_.w = video_width_;
        rect_.h = video_height_;

        SDL_UpdateYUVTexture(texture_, &rect_, frame->data[0], frame->linesize[0],
                frame->data[1], frame->linesize[1],
                frame->data[2], frame->linesize[2]);
        SDL_RenderClear(renderer_);
        SDL_RenderCopy(renderer_, texture_, NULL, &rect_);
        SDL_RenderPresent(renderer_);
        frame = frame_queue_->Pop(1);  // 把帧出队列
        av_frame_free(&frame);
    }
}


void yuvStore(AVFrame *pFrame){
    if(NULL == pFrame){
        printf("NULL = pFrame !\n");
    }

    char file_name[128] = {0};
    time_t t;
    struct tm *timer;
    time(&t);
    timer = localtime(&t);
    std::cout<< "pFrame->format"<<pFrame->format <<std::endl;
    switch(pFrame->format){
        case AV_PIX_FMT_YUVJ420P:
        {
            pFrame->width,pFrame->height,timer->tm_year+1900,timer->tm_mon+1,timer->tm_mday,timer->tm_hour,timer->tm_min,timer->tm_sec);
            sprintf(file_name,"yuv420p.yuv");
            FILE *fp_yuv = fopen(file_name,"w");
            if(NULL == fp_yuv){
                printf("NULL == fp_yuv!\n");
            }
            printf("AV_PIX_FMT_YUV420P, width = %d, height = %d\n",pFrame->width,pFrame->height);
            printf("\n------------------%d,%d,%d------------------\n",pFrame->linesize[0],pFrame->linesize[1],pFrame->linesize[2]);


            int y_size = pFrame->width*pFrame->height;
            fwrite(pFrame->data[0],1,y_size,fp_yuv);    //Y
            fwrite(pFrame->data[1],1,y_size/4,fp_yuv);  //U
            fwrite(pFrame->data[2],1,y_size/4,fp_yuv);  //V
            fclose(fp_yuv);
            break;
        }

        default:
            printf("unsupport format !\n");
            break;
    }
}



void VideoOutput::videoRefresh(double *remaining_time,uint8_t * data)
{
    AVFrame *frame = NULL;
    frame = frame_queue_->Front(); //获取准备要渲染的帧， 这里不出队列
    if(frame) {
        double pts = frame->pts * av_q2d(time_base_); // 视频帧的时间戳
        double diff = pts - avsync_->GetClock();  // 对比音频时钟， 复杂的音视频同步 ffplay B站ijkplayer

        if(diff > 0) {
            *remaining_time = FFMIN(*remaining_time, diff);
            return;
        }
        int size = frame->width*frame->height;
        memcpy(data,frame->data[0],size);
        memcpy(data+size,frame->data[1],size/4);
        memcpy(data+size+size/4,frame->data[2],size/4);
        frame = frame_queue_->Pop(1);  // 把帧出队列
        av_frame_free(&frame);
    }
}







