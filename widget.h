#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QVideoFrame>
#include "videosurface.h"
#include "glyuvwidget.h"
#include <iostream>
#include "log.h"
#include "demuxthread.h"
#include "avframequeue.h"
#include "decodethread.h"
#include "audiooutput.h"
#include "videooutput.h"
#include "stream.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;


    QMediaPlayer m_player;
    QVideoWidget m_widget;

    VideoSurface m_videoSurface;
    GLYuvWidget * yuvwidget0;
    GLYuvWidget * yuvwidget1;
    GLYuvWidget * yuvwidget2;
    GLYuvWidget * yuvwidget3;

//    AVPacketQueue audio_packet_queue;
//    AVPacketQueue video_packet_queue;

    AVFrameQueue * audio_frame_queue;
    AVFrameQueue * video_frame_queue;
    stream * video1;
    stream * video2;
    stream * video3;
    stream * video4;
};
#endif // WIDGET_H月
