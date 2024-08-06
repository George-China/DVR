#ifndef STREAM_H
#define STREAM_H
#include <iostream>
#include "log.h"
#include "demuxthread.h"
#include "avframequeue.h"
#include "decodethread.h"
#include "audiooutput.h"
#include "videooutput.h"
using namespace std;

class stream
{
public:
    stream( char * url,
            AVFrameQueue * audio_frame_queue,
            AVFrameQueue * video_frame_queue);
    ~stream();
    int Init();
    int Start();
    int Stop();
    int getframe();
    VideoOutput *video_output_;
    AVSync * avsync_;
    AVPacketQueue * audio_packet_queue_;
    AVPacketQueue * video_packet_queue_;
    AVFrameQueue * audio_frame_queue_;
    AVFrameQueue * video_frame_queue_;
    DemuxThread *demux_thread_;
    DecodeThread *video_decode_thread_;

private:
        char * url_;





};

#endif // STREAM_H
