#ifndef OPENCVFILTER_H
#define OPENCVFILTER_H

#include <QVideoFilterRunnable>
#include <QDebug>
#include <QTemporaryFile>

#include "opencv2/opencv.hpp"

cv::CascadeClassifier classifier;

class OpenCVFilter : public QAbstractVideoFilter {

public:

    QVideoFilterRunnable *createFilterRunnable();

signals:

    void finished(QObject *result);
};

class OpenCVFilterRunnable : public QVideoFilterRunnable {

public:
    explicit OpenCVFilterRunnable(OpenCVFilter * filter);

    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);

private:
    OpenCVFilter *filter;
};

#endif // OPENCVFILTER_H
