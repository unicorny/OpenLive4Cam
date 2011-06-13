#include "videoview.h"

VideoView::VideoView(QWidget *parent) :
    QWidget(parent), mImage(NULL)
{
    //setBackgroundRole(QPalette::Base);
    //setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    //mImage = new QImage("test.jpg");

    update();
    //paint_new(&mImage);

}

VideoView::~VideoView()
{
    delete mImage;
}

void VideoView::newPicture(QImage *image)
{    
    mImage = image;
    update();
}

void VideoView::paintEvent(QPaintEvent * /* event */)
//void VideoView::paint_new(QImage* image)
{
    if(!mImage) return;

    QPainter painter(this);
    QWidget* area = this->parentWidget()->parentWidget();
    int x = 0;
    int y = 0;

    painter.setBrush(*mImage);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if(area && area->width() > mImage->width())
        x = (area->width() - mImage->width())/2;
    if(area && area->height() > mImage->height())
        y = (area->height() - mImage->height())/2;

    if(x == 0 && y == 0)
    {
        this->parentWidget()->resize(mImage->width(), mImage->height());
        this->resize(mImage->width(), mImage->height());
    }
    painter.drawRect(QRect(x, y, mImage->width(), mImage->height()));

}
