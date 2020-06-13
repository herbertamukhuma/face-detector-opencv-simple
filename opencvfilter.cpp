#include "opencvfilter.h"

OpenCVFilterRunnable::OpenCVFilterRunnable(OpenCVFilter *filter)
{
    this->filter = filter;
}

QVideoFrame OpenCVFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{

    Q_UNUSED(flags);
    input->map(QAbstractVideoBuffer::ReadOnly);

    if(surfaceFormat.handleType() == QAbstractVideoBuffer::NoHandle)
    {
        QImage image(input->bits(),
                     input->width(),
                     input->height(),
                     QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat()));
        image = image.convertToFormat(QImage::Format_RGB888);
        cv::Mat mat(image.height(),
                    image.width(),
                    CV_8UC3,
                    image.bits(),
                    image.bytesPerLine());

        cv::flip(mat, mat, 0);

        if(classifier.empty())
        {
            QFile xml(":/faceclassifier.xml");
            if(xml.open(QFile::ReadOnly | QFile::Text))
            {
                QTemporaryFile temp;
                if(temp.open())
                {
                    temp.write(xml.readAll());
                    temp.close();
                    if(classifier.load(temp.fileName().toStdString()))
                    {
                        qDebug() << "Successfully loaded classifier!";
                    }
                    else
                    {
                        qDebug() << "Could not load classifier.";
                    }
                }
                else
                {
                    qDebug() << "Can't open temp file.";
                }
            }
            else
            {
                qDebug() << "Can't open XML.";
            }
        }
        else
        {
            std::vector<cv::Rect> detected;

            /*
             * Resize in not mandatory but it can speed up things quite a lot!
             */
            QSize resized = image.size().scaled(320, 240, Qt::KeepAspectRatio);
            cv::resize(mat, mat, cv::Size(resized.width(), resized.height()));

            classifier.detectMultiScale(mat, detected, 1.1);

            // We'll use only the first detection to make sure things are not slow on the qml side
            if(detected.size() > 0)
            {
                // Normalize x,y,w,h to values between 0..1 and send them to UI
                emit filter->objectDetected(float(detected[0].x) / float(mat.cols),
                        float(detected[0].y) / float(mat.rows),
                        float(detected[0].width) / float(mat.cols),
                        float(detected[0].height) / float(mat.rows));
            }
            else
            {
                emit filter->objectDetected(0.0,
                                            0.0,
                                            0.0,
                                            0.0);
            }
        }
    }
    else
    {
        qDebug() << "Other surface formats are not supported yet!";
    }

    input->unmap();
    return *input;

}

QVideoFilterRunnable *OpenCVFilter::createFilterRunnable()
{
    return new OpenCVFilterRunnable(this);
}
