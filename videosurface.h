//提取视频帧

#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QObject>
#include <QAbstractVideoSurface>

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit VideoSurface(QObject *parent = 0);
    ~VideoSurface();

signals:
    void showImage(QVideoFrame frame);

protected:
    bool present(const QVideoFrame &frame);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType =
            QAbstractVideoBuffer::NoHandle) const;
};

#endif // VIDEOSURFACE_H

