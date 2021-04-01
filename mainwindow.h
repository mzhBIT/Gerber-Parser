#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QProcess>
#include <QPixmap>
#include "processor.h"
#include "imglabel.h"
#include "dialogdcode.h"
namespace Ui {
class MainWindow;
}
enum mirrortype{NO_MIRRORING,X,Y,XY};
enum edittype{INBOX_SAVE,INBOX_DELETE};

class Editor{
public:
    Editor(edittype et,QRectF rc,int s,int r,mirrortype mt){
        edit = et;
        editbox = rc;
        curscale = s;
        roatation = r;
        mirror = mt;
    }
    Editor(){};
    int curscale;
    int roatation;
    mirrortype mirror;
    edittype edit;
    QRectF editbox;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QStringList list_of_gerbers;

    QString m_open_path_ini="";
    QString m_save_path_ini="E:\\";
    QString m_image_format_ini="png";
    QString m_dpi_ini="300";
    QString m_quick_translation_ini="off";
    QString m_open_folder_after_processing_ini="on";
    QString m_image_size_ini="by_outline";
    QString m_opacity_mode_ini="off";
    QString m_opacity_value_ini="0.8";
    QString m_default_image_width_ini="100";
    QString m_default_image_height_ini="100";
    QString m_default_dx_ini="0";
    QString m_default_dy_ini="0";
    QString m_frame_thickness_ini="1";
    QImage* pxmp[11] = {0};
    QImage* highscalepxmp = 0;
    QImage* gbl_img[11] = {0};
    QImage* gbo_img[11] = {0};
    QImage* gtl_img[11] = {0};
    QImage* gto_img[11] = {0};
    QPixmap ImgShow;
    QStringList gerber_file[4] = {};
    ImgLabel* layer = 0;
    MyScrollBar* nowheelbar = 0;
    QLabel* extra_layer =  0;
    ImgLabel* navigator = 0;
    QCursor cursor_zoomin;
    QCursor cursor_zoomout;
    Processor* parser_handle[4][20] = {0};
    DialogDcode* table_dcode;
    QRect LastBoxPos{0,0,100,100};
    QVector<Editor>EditOpA = QVector<Editor>(20);
    QVector<Editor>EditOpB = QVector<Editor>(20);
    QRectF  editbox[2];

    int editopindex[2] = {-1,-1};  //current operation index in editop vector
    int validopnum[2] = {-1,-1};
    int currentpage = 0;//A or B page
    unsigned char* FusionData = 0;
    long  DataSize = 0;
    int  LastQActionId = 0;
    int  zoomscale = 0;
    int cx = 0;     //current mouse click position on image plane
    int cy = 0;
    int globaladjx = 0;
    int globaladjy = 0;
    int PaintActionId =  0;
    bool DragZoomAction = false;
    bool EditAction = false;
    float dragcx = 0;
    float dragcy = 0;
    double dragzoomfactor = 0;
    int dragzoomscale = 0;
    bool layer_existing[4] = {0};//to,tl,bo,bl
    int layer_position[4] = {0};
    double  k_mm[4] = {0};
    double dpi_scale[20] = {0};
public: signals:
    void run_processing();

public slots:
    void ClickOpenFileA();
    void ClickOpenFileB();
    void AnalysisStart();
    void process_finished();
    void processall_finished();
    void rotatechange(int);
    void basicrender(QRect* cropArea = 0);
    void onMousePress();
    void onMouseMove();
    void onZoomTouch();
    void navigator_update();
    void navigator_move();
    void MultiScaleParser();
    void  clear_fusiondata();
    void fusion(QImage &A,QImage &B,QImage *res,QRect* cropArea=0);
    void resetconfigure();
private:
    Ui::MainWindow *ui;
    struct thread_struct{
        QFuture<int>* future_handle;    // Flow indicator
        Processor* processor_handle;    // Project index
        int widget_index;
    };

    QList<thread_struct> threads;       // Thread list
    int count_of_finished_processes = 0;
    int count_of_gerbers = 0;
    int return_code=-1;

    bool newfile = true;
    bool everything_was_ok;             // Flag: all files are processed correctly
    bool at_least_one_done;             // Flag: at least one file has been processed
    int current_angle = 0;
    int current_width = 0;
    int current_height = 0;
    mirrortype _mirr = NO_MIRRORING;
    QRect init_layer_info;
    QRect navigator_pos;
};

#endif // MAINWINDOW_H
