#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QAction>
#include <QScrollBar>
#include <QMessageBox>
#define LAYERNUM 7

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setWindowFlag(Qt::Drawer);
    layer = new ImgLabel;
    layer->setMouseTracking(true);
    extra_layer =   new QLabel(this);
    extra_layer->setGeometry(0,0,10,10);
    extra_layer->setDisabled(true);
    navigator = new ImgLabel;
    QPixmap pixmap(":icon/red_zoomin.png");
    QSize picSize(24, 24);
    QPixmap scaledPixmap = pixmap.scaled(picSize, Qt::KeepAspectRatio);
    cursor_zoomin = QCursor(scaledPixmap, -1, -1);
    table_dcode = new DialogDcode(this);
    QPixmap pixmap2(":icon/red_zoomout.png");
    QPixmap scaledPixmap2 = pixmap2.scaled(picSize, Qt::KeepAspectRatio);
    cursor_zoomout = QCursor(scaledPixmap2, -1, -1);
    layer->setAlignment(Qt::AlignCenter);
    nowheelbar = new  MyScrollBar;
    ui->scrollArea->setBackgroundRole(QPalette::WindowText);
    ui->scrollArea->setVerticalScrollBar(nowheelbar);
    ui->scrollArea->setWidget(layer);
    ui->scrollArea_2->setBackgroundRole(QPalette::WindowText);
    ui->scrollArea_2->setWidget(navigator);
    ui->radioButton->setChecked(true);
    ui->radioButton_3->setChecked(true);
    ui->checkBox->setCheckable(false);
    ui->checkBox_2->setCheckable(false);
    this->init_layer_info = ui->scrollArea->geometry();
    this->navigator_pos =  ui->scrollArea_2->geometry();
    QScrollBar* bar =  this->ui->scrollArea->horizontalScrollBar();
    QScrollBar* bar2 =  this->ui->scrollArea->verticalScrollBar();


    globaladjy = this->ui->mainToolBar->geometry().y()+this->ui->mainToolBar->geometry().height()+this->ui->scrollArea->geometry().y()+1;
    globaladjx = this->ui->scrollArea->geometry().x()+1;
    globaladjy = 68;
    globaladjx = 11;
    //qDebug()<<this->ui->menuBar->geometry()<<this->ui->mainToolBar->geometry()<<this->ui->toolBar->geometry();
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(ClickOpenFileA()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(ClickOpenFileB()));
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(AnalysisStart()));
    connect(ui->checkBox,&QCheckBox::clicked,this,[this](){this->clear_fusiondata();this->process_finished();});
    connect(ui->checkBox_2,&QCheckBox::clicked,this,[this](){this->clear_fusiondata();this->process_finished();});
    connect(ui->radioButton,&QRadioButton::toggled,this,[this](){this->currentpage=0; this->clear_fusiondata();this->process_finished();});
    connect(ui->radioButton_2,&QRadioButton::toggled,this,[this](){this->currentpage=1;this->clear_fusiondata();this->process_finished();});
    connect(ui->radioButton_3,&QPushButton::clicked,this,[this](){this->clear_fusiondata();this->rotatechange(0);});
    connect(ui->radioButton_4,&QPushButton::clicked,this,[this](){this->clear_fusiondata();this->rotatechange(90);});
    connect(ui->radioButton_5,&QPushButton::clicked,this,[this](){this->clear_fusiondata();this->rotatechange(180);});
    connect(ui->radioButton_6,&QPushButton::clicked,this,[this](){this->clear_fusiondata();this->rotatechange(270);});
    connect(ui->action,&QAction::triggered,this,[this](){this->layer->setCursor(cursor_zoomin);this->LastQActionId = 1;});
    connect(ui->action_2,&QAction::triggered,this,[this](){this->layer->setCursor(cursor_zoomout);this->LastQActionId = 2;});
    connect(ui->action_3,&QAction::triggered,this,[this](){this->layer->setCursor(Qt::OpenHandCursor);this->LastQActionId = 3;});
    connect(ui->action_4,&QAction::triggered,this,[this](){this->layer->setCursor(Qt::ArrowCursor);this->LastQActionId = 4;
        zoomscale=0;this->PaintActionId = 4;if(this->pxmp[0]) basicrender();});
    connect(ui->action_5,&QAction::triggered,this,[this](){this->layer->setCursor(Qt::CrossCursor);this->LastQActionId = 5;});
    connect(ui->action_D,&QAction::triggered,this,[this](){
        if(parser_handle[0][0] == 0)
            QMessageBox::warning(NULL, "警告", "当前没有D码表供显示", QMessageBox::NoButton);
        else{
            Processor* p = parser_handle[0][0];
            this->table_dcode->table_update(&(p->aperture_dictionary),p->k_mm_or_inch);
        }
    });
    connect(ui->action_8,&QAction::triggered,this,[this](){this->layer->setCursor(Qt::CrossCursor);this->LastQActionId = 8;});
    connect(ui->action_9,&QAction::triggered,this,[this](){
        if((this->ui->radioButton->isChecked()&&this->ui->checkBox->isChecked()&&layer_existing[0]) ||
                (this->ui->radioButton_2->isChecked()&&this->ui->checkBox->isChecked()&&layer_existing[2])){
            this->layer->setCursor(Qt::CrossCursor);this->LastQActionId = 9;
        }
        else
            QMessageBox::critical(NULL, "警告", "未勾选丝印层或者丝印层无效！", QMessageBox::NoButton);

    });
    connect(ui->action_10,&QAction::triggered,this,[this](){
        if((this->ui->radioButton->isChecked()&&this->ui->checkBox->isChecked()&&layer_existing[0]) ||
                (this->ui->radioButton_2->isChecked()&&this->ui->checkBox->isChecked()&&layer_existing[2])){
            this->layer->setCursor(Qt::CrossCursor);this->LastQActionId = 10;
        }
        else
            QMessageBox::critical(NULL, "警告", "未勾选丝印层或者丝印层无效！", QMessageBox::NoButton);
    });
    connect(ui->action_11,&QAction::triggered,this,[this](){
        editopindex[currentpage] = qMax(-1,qMin(validopnum[currentpage],editopindex[currentpage]-1));
        basicrender();
    });
    connect(ui->action_12,&QAction::triggered,this,[this](){
        editopindex[currentpage] = qMax(-1,qMin(validopnum[currentpage],editopindex[currentpage]+1));
        basicrender();
    });
    connect(layer,SIGNAL(ImgPressed()),this,SLOT(onMousePress()));
    connect(layer,SIGNAL(ImgMove()),this,SLOT(onMouseMove()));
    connect(ui->checkBox_3,&QCheckBox::clicked,this,[this](){this->process_finished();});
    connect(ui->checkBox_4,&QCheckBox::clicked,this,[this](){this->process_finished();});
    connect(bar,&QScrollBar::rangeChanged,this,[this](){this->onZoomTouch();this->navigator_update();});
    connect(bar2,&QScrollBar::rangeChanged,this,[this](){this->onZoomTouch();this->navigator_update();});
    connect(bar,&QScrollBar::valueChanged,this,[this](){this->navigator_update();});
    connect(bar2,&QScrollBar::valueChanged,this,[this](){this->navigator_update();});
    connect(navigator,SIGNAL(ImgPressed()),this,SLOT(navigator_move()));

}

MainWindow::~MainWindow()
{
    if(layer)
        delete layer;
    if(nowheelbar)
        delete  nowheelbar;
    if(navigator)
        delete navigator;
    for(int i=0;i<LAYERNUM;++i)
    {
        if(pxmp[i])
            delete pxmp[i];
        if(gbl_img[i])
            delete gbl_img[i];
        if(gbo_img[i])
            delete gbo_img[i];
        if(gtl_img[i])
            delete gtl_img[i];
        if(gto_img[i])
            delete gto_img[i];
    }
    delete ui;
}
QRect rectintersection(QRect rc1,QRect rc2){
    QRect rc;
    int x11 = rc1.left();
    int x12 = rc1.right();
    int y11 = rc1.top();
    int y12 = rc1.bottom();

    int x21 = rc2.left();
    int x22 = rc2.right();
    int y21 = rc2.top();
    int y22 = rc2.bottom();
    int StartX = qMin(x11,x21);
    int EndX = qMax(x12,x22);
    int StartY = qMin(y11,y21);
    int EndY = qMax(y12,y22);
    int CurWidth=(x12-x11)+(x22-x21)-(EndX-StartX);
    int CurHeight=(y12-y11)+(y22-y21)-(EndY-StartY);
    if(CurWidth<=0 || CurHeight <= 0){
        rc.setX(-1);
        rc.setY(-1);
        rc.setWidth(-1);
        rc.setHeight(-1);
    }
    else{
        rc.setLeft(qMax(x11,x21));
        rc.setTop(qMax(y11,y21));
        rc.setRight(qMin(x12,x22));
        rc.setBottom(qMin(y12,y22));
    }
    return rc;
}
void MainWindow::fusion(QImage &A,QImage &B,QImage *res,QRect* cropArea){
    if(res ==  0)
        return;
    QElapsedTimer time;
    time.start();
    int width = A.width();
    int height = A.height();
    int left=0,top=0,right=width,bottom=height;
    if(cropArea != 0){
        left = cropArea->x();
        top =  cropArea->y();
        right = qMin(left+cropArea->width(),right);
        bottom = qMin(top+cropArea->height(),bottom);
    }
    if(editopindex[currentpage]>-1){
        Editor* op = 0;
        if(currentpage == 0)
            op = &EditOpA[0];
        else
            op = &EditOpB[0];
        QVector<QRect>rc_save;
        QVector<QRect>rc_delete;
        for(int i=0;i<=editopindex[currentpage];++i){
            QRect rc{qRound(op[i].editbox.left()*(zoomscale+1)),qRound(op[i].editbox.top()*(zoomscale+1)),\
                        qRound(op[i].editbox.width()*(zoomscale+1)),qRound(op[i].editbox.height()*(zoomscale+1))};
            if(op[i].edit == INBOX_SAVE)
                rc_save.push_back(rc);
            else
                rc_delete.push_back(rc);
        }
        QRect rcintsec_save{0,0,width,height};
        for(int i=0;i<rc_save.size();++i){
            rcintsec_save = rectintersection(rcintsec_save,rc_save[i]);
            if(rcintsec_save.x() == -1)
                break;
        }
        if(rcintsec_save.x()==-1){
            memset(res->bits(),0,res->bytesPerLine()*res->height());
        }
        else{
            memset(res->bits(),0,res->bytesPerLine()*res->height());
            int l=rcintsec_save.left(),t = rcintsec_save.top(),r=rcintsec_save.right(),b=rcintsec_save.bottom();
            for (int i = t; i < b; i++){
                uchar *imageScanLineA = A.scanLine(i);
                uchar *imageScanLineB = B.scanLine(i);//B is silk layer
                uchar *imageScanLineC = res->scanLine(i);
                for (int k = l; k < r; k++){
                    int index = k*3;
                    imageScanLineC[index] =  imageScanLineB[k];
                    imageScanLineC[index+1] =  imageScanLineA[k];
                    //imageScanLineC[index+2] =  imageScanLineA[index+2]+imageScanLineB[index+2];
                 }
            }
            for(int i=0;i<rc_delete.size();++i){
                QRect rc = rectintersection(rc_delete[i],rcintsec_save);
                if(rc.x() !=-1){
                    int l=rc.left(),t = rc.top(),r=rc.right(),b=rc.bottom();
                    for (int i = t; i < b; i++){
                        //uchar *imageScanLineA = A.scanLine(i);
                        //uchar *imageScanLineB = B.scanLine(i);//B is silk layer
                        uchar *imageScanLineC = res->scanLine(i);
                        for (int k = l; k < r; k++){
                            int index = k*3;
                            imageScanLineC[index] =  0;
                         }
                    }
                }
            }
        }
    }
    else{
        for (int i = top; i < bottom; i++){
            uchar *imageScanLineA = A.scanLine(i);
            uchar *imageScanLineB = B.scanLine(i);//B is silk layer
            uchar *imageScanLineC = res->scanLine(i);
            for (int k = left; k < right; k++){
                int index = k*3;
                imageScanLineC[index] =  imageScanLineB[k];
                imageScanLineC[index+1] =  imageScanLineA[k];
                //imageScanLineC[index+2] =  imageScanLineA[index+2]+imageScanLineB[index+2];
             }
        }
    }
    qDebug()<<"fusion time"<<time.elapsed()<<"ms"<<left<<top<<right<<bottom;
}
void  MainWindow::navigator_move(){
    int click_x = this->navigator->localPos.x();
    int click_y = this->navigator->localPos.y();
    int cw  = current_width;
    int ch  = current_height;
    int totalw  = navigator_pos.width()-4;
    int totalh = navigator_pos.height()-4;
    int x,y,w,h;
    if(1){
        if(cw*1.0/ch>totalw*1.0/totalh){//cut short  edge
            w = totalw-20;
            x = 10;
            h = w*ch*1.0/cw;
            y  = (totalh-h)/2;
        }
        else{
            y = 10;
            h  = totalh-20;
            w = h*cw*1.0/ch;
            x  = (totalw-w)/2;
        }
    }
    if(click_x<x || click_x>x+w || click_y<y || click_y>y+h)
        return;
    float horiportion,vertiportion;
    horiportion = this->init_layer_info.width()*1.0/current_width;
    vertiportion = this->init_layer_info.height()*1.0/current_height;

    QScrollBar* bar =  this->ui->scrollArea->horizontalScrollBar();
    QScrollBar* bar2 =  this->ui->scrollArea->verticalScrollBar();
    if(bar->isVisible()){
        vertiportion = (this->init_layer_info.height()-bar->height())*1.0/current_height;
    }
    if(bar2->isVisible()){
        horiportion = (this->init_layer_info.width()-bar2->width())*1.0/current_width;
    }
    int default_cx  = w*horiportion/2+x;
    int default_cy = h*vertiportion/2+y;
    float factor = current_width*1.0/w;
    if(bar->isVisible())
        bar->setValue((click_x-default_cx)*factor);
    if(bar2->isVisible())
        bar2->setValue((click_y-default_cy)*factor);
}
void MainWindow::navigator_update(){
    {
        int cw  = current_width;
        int ch  = current_height;
        int totalw  = navigator_pos.width()-4;
        int totalh = navigator_pos.height()-4;
        int x,y,w,h;
        if(1){
            if(cw*1.0/ch>totalw*1.0/totalh){//cut short  edge
                w = totalw-20;
                x = 10;
                h = w*ch*1.0/cw;
                y  = (totalh-h)/2;
            }
            else{
                y = 10;
                h  = totalh-20;
                w = h*cw*1.0/ch;
                x  = (totalw-w)/2;
            }
        }
        float horiportion,vertiportion;
        horiportion = this->init_layer_info.width()*1.0/current_width;
        vertiportion = this->init_layer_info.height()*1.0/current_height;

        int view_x = 0;
        int view_y = 0;
        QScrollBar* bar =  this->ui->scrollArea->horizontalScrollBar();
        QScrollBar* bar2 =  this->ui->scrollArea->verticalScrollBar();
        if(bar->isVisible()){
            view_x = bar->value()*1.0/current_width*w;
            vertiportion = (this->init_layer_info.height()-bar->height())*1.0/current_height;
        }
        if(bar2->isVisible()){
            view_y = bar2->value()*1.0/current_height*h;
            horiportion = (this->init_layer_info.width()-bar2->width())*1.0/current_width;
        }

        int view_w = w*horiportion;
        int view_h = h*vertiportion;
        view_x += x;
        view_y += y;
        QPixmap map = QPixmap(totalw,totalh);
        map.fill(Qt::black);
        QPainter paint;
        paint.begin(&map);
        paint.setPen(Qt::green);
        paint.drawRect(x,y,w,h);
        paint.setPen(Qt::red);
        paint.setBrush(QBrush(Qt::red));
        paint.drawRect(view_x,view_y,view_w,view_h);
        paint.end();
        navigator->setPixmap(map);
    }
}
void MainWindow::onZoomTouch()
{
    QScrollBar* bar =  this->ui->scrollArea->horizontalScrollBar();
    QScrollBar* bar2 =  this->ui->scrollArea->verticalScrollBar();
    if(this->PaintActionId == 1 || this->PaintActionId == 2){//zoom  in/out
        if(bar->isVisible()){
            int CurCx = this->cx*(this->zoomscale+1)*1.0/(this->zoomscale);
            if(this->PaintActionId == 2)
                CurCx = this->cx*(this->zoomscale+1)*1.0/(this->zoomscale+2);

            int gx = this->layer->globalPos.x()-this->geometry().x()-globaladjx;
            int val = CurCx-gx;
            val =  val<bar->minimum()?bar->minimum():val;
            val =  val>bar->maximum()?bar->maximum():val;
            bar->setValue(val);
        }
        if(bar2->isVisible()){
            int CurCy = cy*(zoomscale+1)*1.0/(zoomscale);
            if(this->PaintActionId == 2)
                CurCy = cy*(zoomscale+1)*1.0/(zoomscale+2);

            int gy = this->layer->globalPos.y()-this->geometry().y()-globaladjy;
            int val = CurCy-gy;
            val =  val<bar2->minimum()?bar2->minimum():val;
            val =  val>bar2->maximum()?bar2->maximum():val;
            bar2->setValue(val);
        }
    }
    else if(this->PaintActionId == 3)//rotate
    {
       bar->setValue((bar->minimum()+bar->maximum())/2);
       bar2->setValue((bar2->minimum()+bar2->maximum())/2);
    }
    else if(this->PaintActionId == 4){//global preview
        bar->setValue(0);
        bar2->setValue(0);
    }
    else if(this->PaintActionId == 5){
        int CurCx = this->cx*this->dragzoomfactor;
        int CurCy = cy*this->dragzoomfactor;
        QScrollBar* bar =  this->ui->scrollArea->horizontalScrollBar();
        QScrollBar* bar2 =  this->ui->scrollArea->verticalScrollBar();
        //if(bar->isVisible() && bar2->isVisible())
        {
            bar->setValue(CurCx-(this->ui->scrollArea->width()-bar2->width())/2);
            bar2->setValue(CurCy-(this->ui->scrollArea->height()-bar->height())/2);
        }
    }
}
void GrayToColorMap(QImage src,QImage* res,QColor forecolor,QRect* cropArea=0){
    if(src.isNull())
        return;
    QElapsedTimer timer;
    timer.start();
    //QImage dd(src.width(),src.height(),QImage::Format_RGB888);
    //dd.fill(Qt::black);
    //memset(dd.bits(),0,dd.bytesPerLine()*dd.height());
    //qDebug()<<"fill time"<<timer.elapsed()<<"ms";
    int height = src.height();
    int width = src.width();
    int left=0,top=0,right=width,bottom=height;
    if(cropArea != 0){
        left = cropArea->x();
        top =  cropArea->y();
        right = qMin(left+cropArea->width(),right);
        bottom = qMin(top+cropArea->height(),bottom);
    }
    timer.start();
    if(forecolor == Qt::green){
        for (int i = top; i < bottom; ++i){
            uchar *imageScanLineA = src.scanLine(i);
            uchar *imageScanLineB = res->scanLine(i);
            for (int k = left; k < right; ++k){
                imageScanLineB[(k<<2)-k+1] =  imageScanLineA[k];
             }
        }
    }
    if(forecolor == Qt::red){
        for (int i = top; i < bottom; ++i){
            uchar *imageScanLineA = src.scanLine(i);
            uchar *imageScanLineB = res->scanLine(i);
            for (int k = left; k < right; k++){
                imageScanLineB[k*3] =  imageScanLineA[k];
             }
        }
    }
    if(forecolor == Qt::blue){
        for (int i = top; i < bottom; ++i){
            uchar *imageScanLineA = src.scanLine(i);
            uchar *imageScanLineB = res->scanLine(i);
            for (int k = left; k < right; k++){
                imageScanLineB[k*3+2] =  imageScanLineA[k];
             }
        }
    }
    qDebug()<<"convert time"<<timer.elapsed()<<"ms";
}
void MainWindow::onMouseMove(){
    if(this->LastQActionId == 3){
        if(layer->isPressed == false){
            layer->setCursor(Qt::OpenHandCursor);
        }
        else{
            layer->setCursor(Qt::ClosedHandCursor);
            QScrollBar* bar =  this->ui->scrollArea->horizontalScrollBar();
            QScrollBar* bar2 =  this->ui->scrollArea->verticalScrollBar();
            int value = bar->value();
            bar->setValue(value-layer->offset[0]);
            value = bar2->value();
            bar2->setValue(value-layer->offset[1]);
        }
    }
    if(this->LastQActionId == 8 || this->LastQActionId == 5  || this->LastQActionId == 9 || this->LastQActionId == 10){//measure,drag zoom,delete,save
        if(layer->isPressed == false){
            if(this->LastQActionId == 5){
                if(this->DragZoomAction){
                    this->DragZoomAction = false;
                    if(zoomscale == LAYERNUM - 1){
                        basicrender();
                        return;
                    }
                    QScrollBar* barhorizen =  this->ui->scrollArea->horizontalScrollBar();
                    QScrollBar* barvert =  this->ui->scrollArea->verticalScrollBar();
                    int gx = this->dragcx;
                    int gy =this->dragcy;
                    int tcx,tcy;
                    if(barhorizen->isVisible() && barvert->isVisible()){
                        tcx = gx,tcy = gy;
                    }
                    if(barhorizen->isVisible() && !barvert->isVisible()){
                        tcx = gx;
                        tcy = gy -  (this->ui->scrollArea->height()-barhorizen->height()-current_height)/2;
                    }
                    if(!barhorizen->isVisible()  && barvert->isVisible()){
                        tcy =gy;
                        tcx = gx -  (this->ui->scrollArea->width()-barvert->width()-current_width)/2;
                    }
                    if(!barhorizen->isVisible()  && !barvert->isVisible()){
                        tcx = gx -  (this->ui->scrollArea->width()-barvert->width()-current_width)/2;
                        tcy = gy -  (this->ui->scrollArea->height()-barhorizen->height()-current_height)/2;
                    }
                    if(tcx > 0 && tcx <  current_width && tcy>0 && tcy<current_height){
                        cx = tcx,cy = tcy;
                        zoomscale = dragzoomscale;
                        PaintActionId = 5;

                        if(zoomscale>LAYERNUM-1){
                            zoomscale = LAYERNUM-1;
                            return;
                        }
                        if(zoomscale<0){
                            zoomscale  =  0;
                            return;
                        }
                        basicrender();
                    }
                }
            }
            else if(this->LastQActionId == 9 || this->LastQActionId == 10){
                if(ui->checkBox_3->isChecked()&&ui->checkBox_4->isChecked())
                    _mirr = XY;
                else if(ui->checkBox_3->isChecked())
                    _mirr = X;
                else if(ui->checkBox_4->isChecked())
                    _mirr = Y;
                else
                    _mirr = NO_MIRRORING;
                edittype et = INBOX_DELETE;
                if(this->LastQActionId == 10)
                    et = INBOX_SAVE;
                int k=currentpage;
                Editor op{et,editbox[k],zoomscale,current_angle,_mirr};
                if(editbox[k].width()>0 && (EditAction == true)){
                    EditAction = false;
                    editopindex[k]++;
                    validopnum[k] = editopindex[k];
                    if(editopindex[k]>19){
                        editopindex[k] = 19;
                        for(int i=1;i<20;++i){
                            if(k == 0)
                                EditOpA[i] = EditOpA[i+1];
                            else
                                EditOpB[i] = EditOpB[i+1];
                        }
                    }
                    if(k==0)
                        EditOpA.replace(editopindex[k],op);
                    else
                        EditOpB.replace(editopindex[k],op);
                    basicrender();
                }
            }
        }
        else{
            QPixmap map =layer->pixmap(Qt::ReturnByValue);
            if(map.isNull())
                return;

            QRect MainArea;
            qDebug()<<"main crop area before"<<MainArea.left()<<MainArea.top()<<MainArea.width()<<MainArea.height()<<this->ui->scrollArea->width()<<this->ui->scrollArea->height();
            QPointF startpoint = this->layer->FirstPressDot;
            QPointF endpoint = this->layer->CurrentPressDot;
            QScrollBar* barhorizen =  this->ui->scrollArea->horizontalScrollBar();
            QScrollBar* barvert =  this->ui->scrollArea->verticalScrollBar();
            int gx = startpoint.x();
            int gy = startpoint.y();
            int tcx,tcy;
            if(barhorizen->isVisible() && barvert->isVisible()){
                tcx = gx,tcy = gy;
                MainArea.setX(barhorizen->value());
                MainArea.setY(barvert->value());
            }
            if(barhorizen->isVisible() && !barvert->isVisible()){
                tcx = gx;
                tcy = gy -  (this->ui->scrollArea->height()-barhorizen->height()-current_height)/2;
                MainArea.setX(barhorizen->value());
                MainArea.setY(0);
            }
            if(!barhorizen->isVisible()  && barvert->isVisible()){
                tcy =gy;
                tcx = gx -  (this->ui->scrollArea->width()-barvert->width()-current_width)/2;
                MainArea.setX(0);
                MainArea.setY(barvert->value());
            }
            if(!barhorizen->isVisible()  && !barvert->isVisible()){
                tcx = gx -  (this->ui->scrollArea->width()-barvert->width()-current_width)/2;
                tcy = gy -  (this->ui->scrollArea->height()-barhorizen->height()-current_height)/2;
                MainArea.setX(0);
                MainArea.setY(0);
            }
            MainArea.setWidth(this->ui->scrollArea->width());
            MainArea.setHeight(this->ui->scrollArea->height());
//            qDebug()<<"main crop area"<<MainArea.left()<<MainArea.top()<<MainArea.width()<<MainArea.height();
            QRect* cropArea = &MainArea;

                QImage* CurrentImg = pxmp[zoomscale];
                if(pxmp[zoomscale] == 0) return;
                QImage FusionRes(FusionData,CurrentImg->width(),CurrentImg->height(),CurrentImg->width()*3,QImage::Format_RGB888);

                QColor ForeGround = Qt::white;
                if(zoomscale<LAYERNUM){
                    if(this->ui->radioButton->isChecked()){//top level
                        if(this->ui->checkBox->isChecked() && this->ui->checkBox_2->isChecked()){
                            if(layer_existing[0] && layer_existing[1]){
                                fusion(*gtl_img[zoomscale],*gto_img[zoomscale],&FusionRes,cropArea);
                                CurrentImg = &FusionRes;
                            }
                            else if(layer_existing[0]){
                                *pxmp[zoomscale] = gto_img[zoomscale]->copy(QRect());
                                ForeGround =Qt::red;
                            }
                            else if(layer_existing[1]){
                                *pxmp[zoomscale] = gtl_img[zoomscale]->copy(QRect());
                                ForeGround =Qt::green;
                            }
                        }
                        else if(this->ui->checkBox->isChecked() && layer_existing[0]){
                            *pxmp[zoomscale] = gto_img[zoomscale]->copy(QRect());
                            ForeGround =Qt::red;
                        }
                        else if(this->ui->checkBox_2->isChecked() && layer_existing[1]){
                            *pxmp[zoomscale] = gtl_img[zoomscale]->copy(QRect());
                            ForeGround =Qt::green;
                        }
                    }
                    if(this->ui->radioButton_2->isChecked()){//top level
                        if(this->ui->checkBox->isChecked() && this->ui->checkBox_2->isChecked()){
                            if(layer_existing[2] && layer_existing[3]){
                                fusion(*gbl_img[zoomscale],*gbo_img[zoomscale],&FusionRes,cropArea);
                                CurrentImg = &FusionRes;
                            }
                            else if(layer_existing[2]){
                                *pxmp[zoomscale] = gbo_img[zoomscale]->copy(QRect());
                                ForeGround =Qt::red;
                            }
                            else if(layer_existing[3]){
                                *pxmp[zoomscale] = gbl_img[zoomscale]->copy(QRect());
                                ForeGround =Qt::green;
                            }
                        }
                        else if(this->ui->checkBox->isChecked() && layer_existing[2]){
                            *pxmp[zoomscale] = gbo_img[zoomscale]->copy(QRect());
                            ForeGround =Qt::red;
                        }
                        else if(this->ui->checkBox_2->isChecked() && layer_existing[3]){
                            *pxmp[zoomscale] = gbl_img[zoomscale]->copy(QRect());
                            ForeGround =Qt::green;
                        }
                    }
                }
                QElapsedTimer timer;
                if(CurrentImg->format() != QImage::Format_RGB888){
                    GrayToColorMap(*CurrentImg,&FusionRes,ForeGround,cropArea);
                    CurrentImg = &FusionRes;
                }
                timer.start();
                QTransform ts;
                ts.rotate(current_angle);
                bool mirrorH = this->ui->checkBox_3->isChecked();
                bool mirrorV = this->ui->checkBox_4->isChecked();
                QImage tmp = (CurrentImg->transformed(ts)).mirrored(mirrorH,mirrorV);
                current_width = tmp.width();
                current_height = tmp.height();

                QRectF r_Crop(cropArea->left(),cropArea->top(),cropArea->width(),cropArea->height());
                QPainter paint;
                paint.begin(&map);
                paint.drawImage(r_Crop,tmp,r_Crop);
            //}

            int w = endpoint.x()-startpoint.x();
            int h =  endpoint.y()-startpoint.y();
            float xlength = w/dpi_scale[zoomscale]*25.4;
            float ylength = h/dpi_scale[zoomscale]*25.4;
            if(this->LastQActionId == 8){
                QString xtext = "x="  + QString::number(xlength,'f',2)+"mm";
                QString ytext = "y="  + QString::number(ylength,'f',2)+"mm";
                this->ui->textBrowser_3->setPlainText(xtext+'\n'+ytext);
                QFont serifFont("Times", 14, QFont::Bold);
                QPen  pen;
                pen.setColor(Qt::white);
                pen.setStyle(Qt::DashLine);
                pen.setWidthF(1.5);
                paint.setPen(pen);
                paint.drawRect(tcx,tcy,w,h);
                paint.setFont(serifFont);
                paint.drawText(tcx+w/4,tcy+h+20,xtext);
                paint.drawText(tcx+w+2,tcy+h/2,ytext);
                paint.end();
            }
            else if(this->LastQActionId == 5){
                double scale = map.width()*1.0/map.height();
                int  tcx_ext,tcy_ext,w_ext,h_ext;
                if(w*1.0/h>scale){
                    w_ext = w;
                    h_ext = w/scale;
                    tcx_ext = tcx;
                    tcy_ext = tcy - (h_ext-h)/2;
                }
                else{
                    w_ext = h*scale;
                    h_ext = h;
                    tcy_ext = tcy;
                    tcx_ext = tcx  - (w_ext-w)/2;
                }
                QPen  pen;
                pen.setColor(Qt::white);
                pen.setStyle(Qt::SolidLine);
                pen.setWidthF(1.2);
                paint.setPen(pen);
                paint.drawRect(tcx,tcy,w,h);
                pen.setStyle(Qt::DashLine);
                paint.setPen(pen);
                paint.drawRect(tcx_ext,tcy_ext,w_ext,h_ext);
                paint.end();
                this->DragZoomAction = true;
                this->dragcx = (startpoint.x()+endpoint.x())/2;
                this->dragcy = (startpoint.y()+endpoint.y())/2;
/*                if(zoomscale == LAYERNUM-1)
                    this->DragZoomAction = false;
                else*/{
                    int oldscale = zoomscale;
                    if(w_ext == 0)
                        dragzoomscale = LAYERNUM-1;
                    else
                        dragzoomscale = (zoomscale+1)*map.width()/w_ext-1;
                    dragzoomscale = qMin(dragzoomscale,LAYERNUM-1);
                    dragzoomfactor = (dragzoomscale+1)*1.0/(oldscale+1);
                }
            }
            else{
                QPen  pen;
                pen.setColor(Qt::white);
                pen.setStyle(Qt::SolidLine);
                pen.setWidthF(1.5);
                paint.setPen(pen);
                paint.drawRect(tcx,tcy,w,h);
                paint.end();
                float  left = tcx,top=tcy,width=w,height=h;
                if(ui->checkBox_3->isChecked()){
                    left = map.width()-left-w;
                }
                if(ui->checkBox_4->isChecked()){
                    top = map.height()-top-h;
                }
                if(current_angle == 90){
                    float lefttmp = left;
                    left = top;
                    top = lefttmp+w;
                    width = h;
                    height = w;
                }
                if(current_angle == 180){
                    left = map.width()-left-w;
                    top = map.height()-top-h;
                }
                if(current_angle == 270){
                    float toptmp = top;
                    top = left;
                    left = map.height()-toptmp-h;
                    width = h;
                    height = w;
                }
                editbox[currentpage].setX(left*1.0/(zoomscale+1));
                editbox[currentpage].setY(top*1.0/(zoomscale+1));
                editbox[currentpage].setWidth(width*1.0/(zoomscale+1));
                editbox[currentpage].setHeight(height*1.0/(zoomscale+1));
                EditAction = true;
            }
            qDebug()<<"draw box time"<<timer.elapsed()<<"ms";
            layer->setPixmap(map);
        }
    }
}
void MainWindow::onMousePress()
{
    if(this->LastQActionId == 1 || this->LastQActionId == 2 || this->layer->iswheelzoom){
        QScrollBar* barhorizen =  this->ui->scrollArea->horizontalScrollBar();
        QScrollBar* barvert =  this->ui->scrollArea->verticalScrollBar();
        int gx = this->layer->localPos.x();
        int gy = this->layer->localPos.y();
        if(this->layer->iswheelzoom){
            gx  = this->layer->globalPos.x()-this->geometry().x() - globaladjx + barhorizen->value();
            gy = this->layer->globalPos.y()-this->geometry().y() - globaladjy + barvert->value();
        }
        int tcx,tcy;
        if(barhorizen->isVisible() && barvert->isVisible()){
            tcx = gx,tcy = gy;
        }
        if(barhorizen->isVisible() && !barvert->isVisible()){
            tcx = gx;
            tcy = gy -  (this->ui->scrollArea->height()-barhorizen->height()-current_height)/2;
        }
        if(!barhorizen->isVisible()  && barvert->isVisible()){
            tcy =gy;
            tcx = gx -  (this->ui->scrollArea->width()-barvert->width()-current_width)/2;
        }
        if(!barhorizen->isVisible()  && !barvert->isVisible()){
            tcx = gx -  (this->ui->scrollArea->width()-barvert->width()-current_width)/2;
            tcy = gy -  (this->ui->scrollArea->height()-barhorizen->height()-current_height)/2;
        }
        if(tcx > 0 && tcx <  current_width && tcy>0 && tcy<current_height){
            cx = tcx,cy = tcy;
            if(this->layer->iswheelzoom){
                if(this->layer->wheelzoomin){
                    PaintActionId = 1;
                    zoomscale++;
                }
                else{
                    PaintActionId = 2;
                    zoomscale--;
                }
                this->layer->iswheelzoom = false;
            }
            else{
                if(this->LastQActionId == 1){
                    PaintActionId = 1;
                    zoomscale++;
                }
                else if(LastQActionId == 2){
                    zoomscale--;
                    PaintActionId = 2;
                }
            }

            if(zoomscale>LAYERNUM-1){
                zoomscale = LAYERNUM-1;
                return;
            }
            if(zoomscale<0){
                zoomscale  =  0;
                return;
            }
            basicrender();
        }
        else
        {

            //this->layer->iswheelzoom = false;
        }

        this->ui->textBrowser_3->setPlainText(QString::number(this->layer->localPos.x())+','+QString::number(this->layer->localPos.y()));
    }
}

void MainWindow::basicrender(QRect* cropArea){
    QImage* CurrentImg = pxmp[zoomscale];
    if(CurrentImg==0 || CurrentImg->isNull())
            return;
    QImage FusionRes(FusionData,CurrentImg->width(),CurrentImg->height(),CurrentImg->width()*3,QImage::Format_RGB888);

    QColor ForeGround = Qt::white;
    if(zoomscale<LAYERNUM){
        if(this->ui->radioButton->isChecked()){//top level
            if(this->ui->checkBox->isChecked() && this->ui->checkBox_2->isChecked()){
                if(layer_existing[0] && layer_existing[1]){
                    fusion(*gtl_img[zoomscale],*gto_img[zoomscale],&FusionRes,cropArea);
                    CurrentImg = &FusionRes;
                }
                else if(layer_existing[0]){
                    *pxmp[zoomscale] = gto_img[zoomscale]->copy(QRect());
                    ForeGround =Qt::red;
                }
                else if(layer_existing[1]){
                    *pxmp[zoomscale] = gtl_img[zoomscale]->copy(QRect());
                    ForeGround =Qt::green;
                }
            }
            else if(this->ui->checkBox->isChecked() && layer_existing[0]){
                *pxmp[zoomscale] = gto_img[zoomscale]->copy(QRect());
                ForeGround =Qt::red;
            }
            else if(this->ui->checkBox_2->isChecked() && layer_existing[1]){
                *pxmp[zoomscale] = gtl_img[zoomscale]->copy(QRect());
                ForeGround =Qt::green;
            }
        }
        if(this->ui->radioButton_2->isChecked()){//top level
            if(this->ui->checkBox->isChecked() && this->ui->checkBox_2->isChecked()){
                if(layer_existing[2] && layer_existing[3]){
                    fusion(*gbl_img[zoomscale],*gbo_img[zoomscale],&FusionRes,cropArea);
                    CurrentImg = &FusionRes;
                }
                else if(layer_existing[2]){
                    *pxmp[zoomscale] = gbo_img[zoomscale]->copy(QRect());
                    ForeGround =Qt::red;
                }
                else if(layer_existing[3]){
                    *pxmp[zoomscale] = gbl_img[zoomscale]->copy(QRect());
                    ForeGround =Qt::green;
                }
            }
            else if(this->ui->checkBox->isChecked() && layer_existing[2]){
                *pxmp[zoomscale] = gbo_img[zoomscale]->copy(QRect());
                ForeGround =Qt::red;
            }
            else if(this->ui->checkBox_2->isChecked() && layer_existing[3]){
                *pxmp[zoomscale] = gbl_img[zoomscale]->copy(QRect());
                ForeGround =Qt::green;
            }
        }
    }
    QElapsedTimer timer;
    timer.start();
    QTransform ts;
    ts.rotate(current_angle);
    bool mirrorH = this->ui->checkBox_3->isChecked();
    bool mirrorV = this->ui->checkBox_4->isChecked();
    if(CurrentImg->format() != QImage::Format_RGB888){
        GrayToColorMap(*CurrentImg,&FusionRes,ForeGround,cropArea);
        CurrentImg = &FusionRes;
    }
    QImage tmp = (CurrentImg->transformed(ts)).mirrored(mirrorH,mirrorV);
    qDebug()<<"tansform time"<<timer.elapsed()<<"ms";

    timer.start();
    current_width = tmp.width();
    current_height = tmp.height();

    {
        if(cropArea != 0){
            QRectF r_Crop(cropArea->left(),cropArea->top(),cropArea->width(),cropArea->height());
            qDebug()<<"crop area"<<r_Crop.left()<<r_Crop.top()<<r_Crop.width()<<r_Crop.height();
            QElapsedTimer timer1,timer2;
            timer1.start();
            QPixmap map =layer->pixmap(Qt::ReturnByValue);
            QPainter paint;
            timer2.start();
            paint.begin(&map);
            qDebug()<<"draw image start time"<<timer2.elapsed();
            timer2.start();
            paint.drawImage(r_Crop,tmp,r_Crop);
            qDebug()<<"draw image time"<<timer2.elapsed();
            paint.end();
            layer->setPixmap(map);
            qDebug()<<"draw crop image time"<<timer1.elapsed();
        }
        else
            ImgShow.convertFromImage(tmp);
        qDebug()<<"in from image";
    }

    qDebug()<<"color  transfer time"<<timer.elapsed()<<"ms"<<"image size is "<<current_width<<current_height;
    if(cropArea == 0)
        layer->setPixmap(ImgShow);
}
void MainWindow::rotatechange(int degree)
{
    if(pxmp[zoomscale] !=  0) {
        current_angle = degree;
        basicrender();
        PaintActionId = 3;
    }
}

void MainWindow::process_finished()
{
    qDebug()<<"finished";
    if(pxmp[zoomscale]!=0){
        basicrender();
        navigator_update();
        PaintActionId = 0;
    }
}
void MainWindow::processall_finished()
{
    this->ui->textBrowser_3->setPlainText("Parse done!");
}
void MainWindow::MultiScaleParser(){

}
double load_file(QString name_of_gerber_file,QStringList& list_of_strings_of_gerber){

    QFile file(name_of_gerber_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Gerber files " << name_of_gerber_file << " Can't open it! Maybe it doesn't exist";
        return -1;
    }
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        list_of_strings_of_gerber.append(line);
    }
    file.close();
    for (int i=0;i<list_of_strings_of_gerber.size();i++) {
        if (list_of_strings_of_gerber.at(i).contains("%MO")) {
            if (list_of_strings_of_gerber.at(i).mid(3,2).contains("MM")) {
                return 25.4;
            }
            break;
        }
    }
    return 1.0;
}
void MainWindow::resetconfigure(){
    zoomscale = 0;
    for(int i=0;i<2;++i){
        validopnum[i] = -1;
        editopindex[i] = -1;
    }
}
void  MainWindow::AnalysisStart()
{
    if(list_of_gerbers.length()<1)
        return;
    if(!newfile){
        resetconfigure();
        basicrender();
        return;
    }
    zoomscale = 0;
    this->ui->radioButton_3->setChecked(true);
    this->ui->checkBox_3->setChecked(false);
    this->ui->checkBox_4->setChecked(false);
    list_of_gerbers.removeDuplicates();

    if ((QDir(m_save_path_ini).exists())&&(!m_save_path_ini.isEmpty())){

        for (int i=0; i<threads.size(); i++){
            delete threads.at(i).future_handle;
            delete threads.at(i).processor_handle;
        }
        //  Flow meter reset
        count_of_finished_processes = 0;
        // Clear line
        threads.clear();
        for (int i=0;i<list_of_gerbers.length();i++) {
                QString curfilename = list_of_gerbers.at(i);
                gerber_file[i].clear();
                k_mm[i] = load_file(curfilename,gerber_file[i]);
        }
        for(int i=0;i<4;++i)
            for(int j=0;j<LAYERNUM;++j)
                parser_handle[i][j] = new Processor(1);


        for(int k=0;k<LAYERNUM;++k){
            thread_struct thread;
            thread.widget_index = 0;
            thread.future_handle = nullptr;
            thread.processor_handle = nullptr;
            Processor* p = nullptr;
            QFuture<int>* future = nullptr;
            Processor* pall[4] = {0};

            everything_was_ok = true;
            at_least_one_done = false;
            QString outline_name;
            double w = m_default_image_width_ini.toDouble(), h = m_default_image_height_ini.toDouble(), dx = m_default_dx_ini.toDouble(), dy = m_default_dy_ini.toDouble();   //Board size and default coordinate start offset...
            int init_width = this->init_layer_info.width()*(k+1);
            int init_height = 0;
            int  dpi = 1000000;
            double dpi_layer[4] = {0};
            double oriwidth[4] =  {0};
            double oriheight[4] = {0};
            double dxx = 0;
            double dyy = 0;
            for (int i=0;i<list_of_gerbers.length();i++) {
                    QString curfilename = list_of_gerbers.at(i);
                    //pall[i] = new Processor(1);
                    pall[i]  =  parser_handle[i][k];
                    pall[i]->set_frame_thickness(m_frame_thickness_ini.toDouble());
                    pall[i]->set_image_format(m_image_format_ini);
                    pall[i]->set_paths(curfilename,m_save_path_ini);
                    pall[i]->set_dpi(m_dpi_ini.toInt());
                    //pall[i]->load_file();      //  Loading files into memory
                    pall[i]->set_parsed_file_strings(gerber_file[i]);
                    pall[i]->k_mm_or_inch  =  k_mm[i];
                    if (m_image_size_ini=="by_outline"){
                        pall[i]->get_outline_size(&w, &h, &dx, &dy);           //  The calculation of the width and height of the graph, and the displacement of the coordinate origin (for example, it is in the center of a circular plate)
                    }
                    if ((m_opacity_mode_ini=="on")&&(m_image_format_ini=="png")){
                        pall[i]->set_opacity_value(m_opacity_value_ini.toFloat());
                    }
                    outline_name = pall[i]->get_outline_filename();

                    double frame_thickness = pall[i]->frame_thickness*pall[i]->k_mm_or_inch;
//                    frame_thickness = 0;
                    double board_width = w + abs(dx)+frame_thickness*2;
                    double board_height = h + abs(dy) + frame_thickness*2;
                    init_height = qRound((init_height>init_width*board_height/board_width)?init_height:(init_width*board_height/board_width));
                    double dpit = (init_width*pall[i]->k_mm_or_inch-w)/board_width;
                    oriwidth[i] = (dpit*board_width+w)/pall[i]->k_mm_or_inch;
                    oriheight[i] = (dpit*board_height+h)/pall[i]->k_mm_or_inch;
                    dpi_layer[i] = dpit;
                    dpi = dpi>dpit?dpit:dpi;
                    pall[i]->set_w_h_dx_dy(w,h,dx,dy);
                    dxx = dxx>abs(dx)?dxx:abs(dx);
                    dyy = dxx>abs(dy)?dyy:abs(dy);
            }
            dpi_scale[k] = dpi;
            qDebug()<<"current layer dpi"<<dpi;
            for(int i=0;i<list_of_gerbers.length();++i){
                oriwidth[i] = oriwidth[i]*dpi/dpi_layer[i];
                oriheight[i] = oriheight[i]*dpi/dpi_layer[i];

                if(k==0){
                    QString curfilename = list_of_gerbers.at(i);
                    QStringList splitname = curfilename.split('.');
                    QString extname = splitname[splitname.length()-1];
                    if(extname == "gto" || extname ==  "GTO"){
                        layer_position[0] = i;
                    }
                    if(extname == "gtl" || extname ==  "GTL"){
                        layer_position[1] = i;
                    }
                    if(extname == "gbo" || extname ==  "GBO"){
                        layer_position[2] = i;
                    }
                    if(extname == "gbl" || extname ==  "GBL"){
                        layer_position[3] = i;
                    }
                }
            }
            //  In order to calculate the image size.
            for (int i=0;i<list_of_gerbers.length();i++) {
                if (1){
                    QString curfilename = list_of_gerbers.at(i);
                    p = pall[i];
//                    if(k!=0)
//                        p->set_clearflag(true);
                    p->set_board_w_h(init_width*p->k_mm_or_inch/dpi,init_height*p->k_mm_or_inch/dpi);
                    p->set_dx_dy(dxx,dyy);
                    QImage** tmp = 0;
                    QStringList splitname = curfilename.split('.');
                    QString extname = splitname[splitname.length()-1];
                    if(extname == "gto" || extname ==  "GTO"){
                        tmp = &gto_img[k];
                        p->set_pregroudcolor(Qt::white);
//                        double x=0,y=0;
//                        if(oriwidth[i]<oriwidth[layer_position[1]])
//                            x = (oriwidth[layer_position[1]]-oriwidth[i])/2;
//                        if(oriheight[i]<oriheight[layer_position[1]])
//                            y = (oriheight[layer_position[1]] - oriheight[i])/2;
                        //p->set_offset(x,y);
                    }
                    if(extname == "gtl" || extname ==  "GTL"){
                        tmp = &gtl_img[k];
                    }
                    if(extname == "gbo" || extname ==  "GBO"){
                        tmp = &gbo_img[k];
                        p->set_pregroudcolor(Qt::white);
//                        double x=0,y=0;
//                        if(oriwidth[i]<oriwidth[layer_position[3]])
//                            x = (oriwidth[layer_position[3]]-oriwidth[i])/2;
//                        if(oriheight[i]<oriheight[layer_position[3]])
//                            y = (oriheight[layer_position[3]] - oriheight[i])/2;
                        //p->set_offset(x,y);
                    }
                    if(extname == "gbl" || extname ==  "GBL"){
                        tmp = &gbl_img[k];
                    }
                    QImage::Format f = QImage::Format_Grayscale8;
                    if(*tmp == 0)
                    {
                        *tmp = new QImage(init_width,init_height, f);
                    }
                    else
                    {
                        delete *tmp;
                        *tmp = new QImage(init_width, init_height, f);
                    }

                    if(i==0){
                        if(pxmp[k] == 0)
                            pxmp[k] = new QImage(init_width, init_height, f);
                        else{
                            delete pxmp[k];
                            pxmp[k] = new QImage(init_width, init_height, f);
                        }
                    }

                    p->set_dpi(dpi);     //  dpi  init
                    p->set_canvas(*tmp);
                    // Processing method:
                    future = new QFuture<int>;
                    thread.future_handle = future;
                    thread.processor_handle = p;
                    thread.widget_index = i+k*list_of_gerbers.length();
                    threads.append(thread);
                    if(i==list_of_gerbers.length()-1 && k == 0)
                        connect(p, SIGNAL(finished()), this, SLOT(process_finished()));
                    *future = QtConcurrent::run(p,&Processor::process);
                }
            }
        }
    } // end of if
    if(FusionData != 0)
        delete FusionData;
    FusionData = new unsigned char[pxmp[LAYERNUM-1]->width()*pxmp[LAYERNUM-1]->height()*3];
    DataSize = pxmp[LAYERNUM-1]->width()*pxmp[LAYERNUM-1]->height()*3;
    newfile = false;
    qDebug()<<"in analysis";
}
void MainWindow::clear_fusiondata(){
    if(FusionData !=0 )
        memset(FusionData,0,DataSize);
}
void MainWindow::ClickOpenFileA()
{
    QStringList file_name = QFileDialog::getOpenFileNames(NULL,"选择丝印文件","C:\\","Gerber file(*.gto *.GTO *.gbo *.GBO)");
    if(file_name.length()<1 || file_name.length()>2)
        return;
    if(list_of_gerbers.size()>0){
        newfile = true;
        for(int i=0;i<file_name.size();++i)
            for(int j=0;j<list_of_gerbers.size();++j){
                if(file_name.at(i)==list_of_gerbers.at(i))
                    newfile = false;
            }
    }
    if(!newfile)
        return;
    QString showname = "";
    list_of_gerbers.clear();
    for(int i=0;i<file_name.length();++i){
        showname += file_name.at(i);
        list_of_gerbers.push_back(file_name.at(i));
        showname += '\n';
        QStringList splitName = file_name.at(i).split('.');
        QString extName = splitName.at(splitName.length()-1);
        if(extName == "gto" || extName  ==  "GTO")
            layer_existing[0] = true;
        if(extName == "gbo" || extName  ==  "GBO")
            layer_existing[2] = true;
    }
    ui->textBrowser->setText(showname);
    m_open_path_ini = file_name.at(0).left(file_name.at(0).lastIndexOf('/'));
    this->ui->checkBox->setCheckable(true);
    this->ui->checkBox->setChecked(true);
}

void MainWindow::ClickOpenFileB()
{
    QStringList file_name = QFileDialog::getOpenFileNames(NULL,"选择定位文件","C:\\","Gerber file(*.gtl *.GTL *.gbl *.GBL)");
    if(file_name.length()<1 || file_name.length()>2)
        return;
    if(list_of_gerbers.size()>0){
        newfile = true;
        for(int i=0;i<file_name.size();++i)
            for(int j=0;j<list_of_gerbers.size();++j){
                if(file_name.at(i)==list_of_gerbers.at(i))
                    newfile = false;
            }
    }
    if(!newfile)
        return;
    QString showname = "";
    for(int i=0;i<file_name.length();++i){
        showname += file_name.at(i);
        list_of_gerbers.push_back(file_name.at(i));
        showname += '\n';
        QStringList splitName = file_name.at(i).split('.');
        QString extName = splitName.at(splitName.length()-1);
        if(extName == "gtl" || extName  ==  "GTL")
            layer_existing[1] = true;
        if(extName == "gbl" || extName  ==  "GBL")
            layer_existing[3] = true;
    }
    ui->textBrowser_2->setText(showname);
    this->ui->checkBox_2->setCheckable(true);
    this->ui->checkBox_2->setChecked(true);
}
