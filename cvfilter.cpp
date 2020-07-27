#include <QDebug>

#include "cvfilter.h"

CVFilter::CVFilter(QObject *parent) : QAbstractVideoFilter(parent)
{

    QFile xml(":/assets/classifiers/haarcascade_frontalface_default.xml");

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

CVFilter::~CVFilter()
{
    if(!processThread.isFinished()) {
        processThread.cancel();
        processThread.waitForFinished();
    }
}

QVideoFilterRunnable *CVFilter::createFilterRunnable()
{    
    return new CVFilterRunnable(this);
}

void CVFilter::registerQMLType()
{    
    qmlRegisterType<CVFilter>("CVFilter", 1, 0, "CVFilter");
}

QImage CVFilter::videoFrameToImage(QVideoFrame *frame)
{
    if(frame->handleType() == QAbstractVideoBuffer::NoHandle){

        QImage image = qt_imageFromVideoFrame(*frame);

        if(image.isNull()){
            qDebug() << "-- null image from qt_imageFromVideoFrame";
            return QImage();
        }

        if(image.format() != QImage::Format_RGB32){
            image = image.convertToFormat(QImage::Format_RGB32);
        }

        return image;
    }

    if(frame->handleType() == QAbstractVideoBuffer::GLTextureHandle){
        QImage image(frame->width(), frame->height(), QImage::Format_RGB32);
        GLuint textureId = frame->handle().toUInt();//static_cast<GLuint>(frame.handle().toInt());
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        QOpenGLFunctions *f = ctx->functions();
        GLuint fbo;
        f->glGenFramebuffers(1,&fbo);
        GLint prevFbo;
        f->glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prevFbo);
        f->glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
        f->glReadPixels(0, 0, frame->width(), frame->height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        f->glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));
        return image.rgbSwapped();
    }

    qDebug() << "-- Invalid image format...";
    return QImage();
}

CVFilterRunnable::CVFilterRunnable(CVFilter *filter) : QObject(nullptr), filter(filter)
{

}

CVFilterRunnable::~CVFilterRunnable()
{
    filter = nullptr;
}

QVideoFrame CVFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(surfaceFormat);
    Q_UNUSED(flags);

    if(!input || !input->isValid()){
        return QVideoFrame();
    }

    if(filter->isProcessing){
        return * input;
    }

    if(!filter->processThread.isFinished()){
        return * input;
    }

    filter->isProcessing = true;   

    QImage image = filter->videoFrameToImage(input);    

    // All processing has to happen in another thread, as we are now in the UI thread.
    filter->processThread = QtConcurrent::run(this, &CVFilterRunnable::processImage, image);

    return * input;
}

void CVFilterRunnable::processImage(QImage &image)
{    

    //if android, make image upright
#ifdef Q_OS_ANDROID
    QPoint center = image.rect().center();
    QMatrix matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotate(90);
    image = image.transformed(matrix);
#endif

    if(!image.isNull()){        
        detect(image);
    }

//    QString filename = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/" + "my_image.png";

//    if(!QFile::exists(filename)){
//        image.save(filename);
//    }

}

void CVFilterRunnable::detect(QImage image)
{

    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat frame(image.height(),
                image.width(),
                CV_8UC3,
                image.bits(),
                image.bytesPerLine());

    //cv::flip(frame, frame, 0);

    Mat frameGray;
    cvtColor( frame, frameGray, COLOR_BGR2GRAY );
    equalizeHist( frameGray, frameGray );

    std::vector<cv::Rect> detected;

    //resize the frame
    double imageWidth = image.size().width();
    double imageHeight = image.size().height();

    double resizedWidth = 320;
    double resizedHeight = (imageHeight/imageWidth) * resizedWidth;

    cv::resize(frameGray, frameGray, cv::Size((int)resizedWidth, (int)resizedHeight));

    filter->classifier.detectMultiScale(frameGray, detected, 1.1, 10);

    QJsonArray rects;
    QJsonObject rect;

    double rX, rY, rWidth, rHeight;
    Size frameSize = frameGray.size();

    for(size_t i = 0; i < detected.size(); i++){

        rX = double(detected[i].x) / double(frameSize.width);
        rY = double(detected[i].y) / double(frameSize.height);
        rWidth = double(detected[i].width) / double(frameSize.width);
        rHeight = double(detected[i].height) / double(frameSize.height);

//        Point center( detected[i].x + detected[i].width/2, detected[i].y + detected[i].height/2 );
//        ellipse( frameGray, center, Size( detected[i].width/2, detected[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4 );

        //qDebug() << rX << rY << rWidth << rHeight;

        rect.insert("rX",rX);
        rect.insert("rY",rY);
        rect.insert("rWidth",rWidth);
        rect.insert("rHeight",rHeight);

        rects.append(rect);
    }

    //qDebug() << "Count: " << detected.size();

    if(rects.count() > 0){
        emit filter->objectDetected(QString::fromStdString(QJsonDocument(rects).toJson().toStdString()));
    }

    //saving processed image to disk after every 5 sec
//    qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();
//    qint64 diff = now - filter->lastProcessedImageAt;

//    if(diff >= 5){

//        cvtColor(frameGray, frameGray, COLOR_BGR2RGB);
//        QImage processedImage((uchar*)frameGray.data,frameGray.cols,frameGray.rows,QImage::Format_RGB888);

//        QString filename = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/" + "processed_image_" + QString::number(now) + ".png";

//        processedImage.save(filename);

//        filter->lastProcessedImageAt = now;
//    }

    filter->isProcessing = false;

}

