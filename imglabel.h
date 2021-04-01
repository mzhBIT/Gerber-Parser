#ifndef IMGLABEL_H
#define IMGLABEL_H
#include<QLabel>
#include<QMouseEvent>
#include<QScrollBar>
#include<QWheelEvent>
class ImgLabel : public QLabel
{
    Q_OBJECT
public:
    ImgLabel();
    bool isPressed = false;
    bool wheelzoomin = false;
    bool iswheelzoom = false;
    double offset[2] = {0};
    QPoint PreDot;
    QPointF FirstPressDot;
    QPointF CurrentPressDot;
    QPointF localPos;
    QPointF globalPos;
signals:
    void ImgPressed();
    void ImgMove();
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent* e);
};

class MyScrollBar:public QScrollBar
{
    Q_OBJECT
public:
    MyScrollBar();
protected:
    void wheelEvent(QWheelEvent* e);
};
#endif // IMGLABEL_H
