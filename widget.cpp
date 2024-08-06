#include "widget.h"
#include "ui_widget.h"



#include <iostream>
#include "log.h"
#include "demuxthread.h"
#include "avframequeue.h"
#include "decodethread.h"
#include "audiooutput.h"
#include "videooutput.h"
#include<QFile>
using namespace std;



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);


    //    //初始化stream
    audio_frame_queue = new AVFrameQueue;
    video_frame_queue = new AVFrameQueue;
    video1 = new stream("rtsp://admin:admin@192.168.1.30:554/h264/ch1/main/av_stream",audio_frame_queue,video_frame_queue);
    video1->Init();



    yuvwidget0 = new GLYuvWidget();
    yuvwidget1= new GLYuvWidget();
    yuvwidget2= new GLYuvWidget();
    yuvwidget3= new GLYuvWidget();




    auto t = new QTimer(this);
    auto f = new QFile("/home/py-qz/下载/test.yuv",this);
    auto ba = new QByteArray(1280*720*1.5,0);
    int w = 1280;
    int h = 720;
    int yuv_buf_size_ = 1280 * 720 * 1.5;
    ui->gridLayout->addWidget(yuvwidget0,0,0);
    ui->gridLayout->addWidget(yuvwidget1,0,1);
    ui->gridLayout->addWidget(yuvwidget2,1,0);
    ui->gridLayout->addWidget(yuvwidget3,1,1);
    u_char * yuv_buf_ = (u_char *)malloc(yuv_buf_size_);

    connect(t,&QTimer::timeout,this,[=](){


        double remaining_time = 0.01;

        video1->video_output_->videoRefresh(&remaining_time,yuv_buf_);

        yuvwidget0->slotShowYuv((uchar*)yuv_buf_,w,h);

        yuvwidget1->slotShowYuv((uchar*)(yuv_buf_),w,h);
        yuvwidget2->slotShowYuv((uchar*)(yuv_buf_),w,h);
        yuvwidget3->slotShowYuv((uchar*)(yuv_buf_),w,h);

    });
    t->setInterval(25);
    t->start();





    video1->Start();










}

Widget::~Widget()
{
    delete ui;
}

