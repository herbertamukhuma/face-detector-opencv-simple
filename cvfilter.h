#ifndef MYFILTER_H
#define MYFILTER_H

#include <QVideoFilterRunnable>
#include <QDebug>
#include <QQmlEngine>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QOpenGLFunctions>
#include <QOpenGLContext>

#include <private/qvideoframe_p.h>

#include "opencv2/core.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;

class CVFilter : public QAbstractVideoFilter {
    Q_OBJECT
    Q_PROPERTY(int videoOutputOrientation MEMBER m_videoOutputOrientation NOTIFY videoOutputOrientationChanged)
friend class CVFilterRunnable;

public:
    explicit CVFilter(QObject *parent = nullptr);
    virtual ~CVFilter();

    QVideoFilterRunnable *createFilterRunnable();

    void static registerQMLType();

signals:
    void objectDetected(QString rects);
    void videoOutputOrientationChanged(int orientation);

private:

    QVideoFrame frame;
    CascadeClassifier classifier;
    QFuture<void> processThread;
    bool isProcessing = false;   

    int m_videoOutputOrientation;

    qint64 lastProcessedImageAt = 0;

    QImage videoFrameToImage(QVideoFrame *frame);
    QImage makeImageUpright(QImage image);
};




class CVFilterRunnable : public QObject, public QVideoFilterRunnable {

public:
    explicit CVFilterRunnable(CVFilter *filter);
    virtual ~CVFilterRunnable();

    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);

    void processVideoFrameProbed(QImage &image);
    void detect(QImage image);

private:
    CVFilter *filter;
};


#endif // MYFILTER_H
