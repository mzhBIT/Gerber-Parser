#include "imglabel.h"
#include <QDebug>
ImgLabel::ImgLabel()
{
}
void ImgLabel::mouseMoveEvent(QMouseEvent* e)
{
    if(isPressed){
        offset[0] = e->globalPos().x() - PreDot.x();
        offset[1] = e->globalPos().y() - PreDot.y();
        PreDot =  e->globalPos();
        CurrentPressDot = e->localPos();
    }
    //localPos = e->localPos();
    //qDebug()<<"in mouse moving"<<localPos;
    emit ImgMove();
}
void ImgLabel::mouseReleaseEvent(QMouseEvent* e){
    isPressed  = false;
    emit ImgMove();
}
void ImgLabel::mousePressEvent(QMouseEvent* event)
{
    CurrentPressDot = FirstPressDot = event->localPos();
    isPressed = true;
    PreDot = event->globalPos();
    localPos = event->localPos();
    globalPos =  event->screenPos();
    qDebug()<<"in mouse press"<<localPos;
    emit ImgPressed();
}
void ImgLabel::wheelEvent(QWheelEvent* e)
{
    if(e->delta()>0)
        wheelzoomin = true;
    else
        wheelzoomin = false;
    iswheelzoom = true;
    globalPos = e->globalPosition();
    emit ImgPressed();
}
MyScrollBar::MyScrollBar()
{

}
void MyScrollBar::wheelEvent(QWheelEvent* e)
{

}
