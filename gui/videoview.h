#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QPainter>
#include <QScrollArea>

class VideoView : public QWidget
{
Q_OBJECT
public:
    explicit VideoView(QWidget *parent = 0);
    ~VideoView();

signals:

public slots:
    void newPicture(QImage* image);

protected:
   virtual void paintEvent(QPaintEvent *event);

private:
    void paint_new(QImage* image);

    QImage* mImage;

};

#endif // VIDEOVIEW_H
