#include "stream.h"

stream::stream(char *url, AVFrameQueue *audio_frame_queue, AVFrameQueue *video_frame_queue)
    : url_(url),audio_frame_queue_(audio_frame_queue),video_frame_queue_(video_frame_queue)

{

    avsync_ = new AVSync;
    avsync_->InitClock();
    audio_packet_queue_ = new AVPacketQueue;
    video_packet_queue_ = new AVPacketQueue;
    demux_thread_ = new DemuxThread(audio_packet_queue_, video_packet_queue_);
    video_decode_thread_ = new DecodeThread(video_packet_queue_, video_frame_queue_);



}

stream::~stream()
{

}

int stream::Init()
{
    int ret=0;
    ret = demux_thread_->Init(url_); // 文件路径传入，解析文件
    if(ret < 0)
    {
        LogError("demux_thread.Init failed");
        return -1;
    }


    ret = video_decode_thread_->Init(demux_thread_->VideoCodecParameters());// 初始化视频解码器
    if(ret < 0) {
        LogError("video_decode_thread->Init() failed");
        return -1;
    }
    video_output_ = new VideoOutput(avsync_, demux_thread_->VideoStreamTimebase(),
                                                    video_frame_queue_, demux_thread_->VideoCodecParameters()->width,
                                                    demux_thread_->VideoCodecParameters()->height);
//    ret = video_output_->Init();
//    // 初始化
//    if(ret < 0) {
//        LogError("video_output->Init() failed");
//        return -1;
//    }
    return 0;

}

int stream::Start()
{
    // 启动线程
    int ret;
    ret = demux_thread_->Start(); // 在线程里面读取音视频包
    if(ret < 0) {
        LogError("demux_thread.Start() failed");
        return -1;
    }
    ret = video_decode_thread_->Start(); // 启动解码线程
    if(ret < 0) {
        LogError("video_decode_thread->Start() failed");
        return -1;
    }

//    video_output_->MainLoop();  // 在主线程显示视频
    return  0;
}

int stream::Stop()
{
    demux_thread_->Stop();
    video_decode_thread_->Stop();
    delete video_decode_thread_;
    delete demux_thread_;
    delete avsync_;
}

int stream::getframe()
{
//    video_output_->videoRefresh(0.01);
return 0;
}
