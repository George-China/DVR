#include "decodethread.h"
#include "log.h"
DecodeThread::DecodeThread(AVPacketQueue *packet_queue, AVFrameQueue *frame_queue)
    : packet_queue_(packet_queue), frame_queue_(frame_queue)
{
}

DecodeThread::~DecodeThread()
{
    if(thread_) {
        Stop();
    }
    if(codec_ctx_) {
        avcodec_close(codec_ctx_);
    }
}

int DecodeThread::Init(AVCodecParameters *par)
{
    if(!par) {
        LogError("Init par is null");
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(NULL);
    int ret = avcodec_parameters_to_context(codec_ctx_, par);
    if(ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        LogError("avcodec_parameters_to_context failed, ret:%d, err2str:%s", ret, err2str);
        return -1;
    }

    AVCodec *codec = avcodec_find_decoder(codec_ctx_->codec_id); //作业： 硬件解码
    if(!codec) {
        LogError("avcodec_find_decoder failed");
        return -1;
    }
    ret = avcodec_open2(codec_ctx_, codec, NULL); // 把解码器上下文和解码器绑定起来
    if(ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        LogError("avcodec_open2 failed, ret:%d, err2str:%s", ret, err2str);
        return -1;
    }
    LogInfo("Init finish");
    return 0;
}

int DecodeThread::Start()
{
    thread_ = new std::thread(&DecodeThread::Run, this);
    if(!thread_) {
        LogError("new std::thread(&DecodeThread::Run, this) failed");
        return -1;
    }
    return 0;
}

int DecodeThread::Stop()
{
    Thread::Stop();
}

void DecodeThread::Run()
{
    AVFrame *frame = av_frame_alloc();
    LogInfo("DecodeThread::Run info");
    while (abort_ != 1 ) {
        if(frame_queue_->Size() > 100) { //控制帧队列的大小 推荐大家 ffplay
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 考虑支持notify 唤醒
            continue;
        }
        AVPacket *pkt = packet_queue_->Pop(10);  // 从队列读取数据
        if(pkt) {
            int ret = avcodec_send_packet(codec_ctx_, pkt); // 送给解码器
            //            LogInfo("ret = %d", ret);
            if(ret < 0) {
                av_strerror(ret, err2str, sizeof(err2str));
//                LogError("%s avcodec_send_packet failed, ret:%d, err2str:%s", codec_ctx_->codec->name, ret, err2str);
                av_packet_free(&pkt);
                continue;
                //                break;
            }
            av_packet_free(&pkt);
            // 读取解码后的frame
            while (true) {
                ret = avcodec_receive_frame(codec_ctx_, frame); // 读取解码后的数据
                if(ret == 0) {
                    frame_queue_->Push(frame); // 发给帧队列
//                    LogInfo("%s frame queue size %d", codec_ctx_->codec->name, frame_queue_->Size());
                    continue;
                } else if(AVERROR(EAGAIN) == ret) {
                    //                    av_strerror(ret, err2str, sizeof(err2str));
                    //                    LogNotice("avcodec_receive_frame failed, ret:%d, err2str:%s", ret, err2str);
                    break;
                } else {
                    abort_ = 1;
                    av_strerror(ret, err2str, sizeof(err2str));
                    LogError("avcodec_receive_frame failed, ret:%d, err2str:%s", ret, err2str);
                    break;
                }
            }
        } else {
            //            LogInfo("not got packet");
        }
    }
    LogInfo("DecodeThread::Run Finish");
}
