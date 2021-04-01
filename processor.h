#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QString>
#include <QStringList>
#include <QPainter>
#include "aperture.h"
//#define PARSER_DEBUG
class Processor : public QObject
{    
    Q_OBJECT

    bool is_outline_flag;                   //  showing the outline of the file

    double w, h, dx, dy;                    //  Circuit board dimensions
    double board_width;
    double board_height;
    int    dpi;
    float  opacity_value=1;                 //  transparency
    QColor background =  Qt::black;
    QColor preground = Qt::white;
    QImage* pxmp;
    QString image_format = "png";
    QString name_of_gerber_file;
    QString output_folder_path;
    QString name_of_output_file;
    QString name_of_outline_file="";
    QStringList list_of_strings_of_gerber;

public:
    double frame_thickness = 0.05;          //  One inch profile free field thickness
    double k_mm_or_inch = 1;  //  coefficient If the unit is millimeter, this is one inch.
    double offset[2] = {0};
    QMap<int,Aperture*> aperture_dictionary;

    Processor(const bool is_outline);
    void parser_debug();
    void set_image_format(const QString format);
    void set_paths(const QString gerber_file, const QString output_folder);
    int load_file();
    void get_outline_size(double *width, double *height, double *dx, double *dy);   //  Calculate the board size and coordinate start offset for profile file only
    QString get_outline_filename();
    void set_outline_file_name(const QString);
    void set_opacity_value(const float val);
    void set_dpi(const int new_scale);
    void set_canvas(QImage *img);
    void set_frame_thickness(const double);
    void set_w_h_dx_dy(const double wdt, const double hgt, const double dxx, const double dyy);
    void set_dx_dy(const double dxx, const double dyy);
    void set_board_w_h(const double w, const double h);
    void set_pregroudcolor(QColor color);
    void set_offset(double x,double  y);
    void set_clearflag(bool b);
    void set_parsed_file_strings(QStringList list);
    void set_scale(float scale);
    bool is_outline(){
        if (is_outline_flag) return 1;
        else return 0;
    }
    void clear_data();
    int process();      //  The main function is to process and form the image and coordinate origin displacement according to the given size.
    int processs_repaint();
signals:

    void finished();

private:    

    const double pi = 3.1415926;
    const double mm_in_inch = 25.4;
    enum commands{
        D01 = 1,
        D02 = 2,
        D03 = 3,
        G01 = 4,
        G02 = 5,
        G03 = 6,
        G74 = 7,
        G75 = 8,
        G36 = 9,
        G37 = 10,
        G04 = 11,
        M02 = 12,
        Dnn = 13
    };
    enum extended_commands{
        FS = 1,
        MO = 2,
        AD = 3,
        AM = 4,
        AB = 5,
        LP = 6,
        LM = 7,
        LR = 8,
        LS = 9,
        TF = 10,
        TA = 11,
        TO = 12,
        TD = 13,
        LN = 14
    };

    //  Graphic state:
    //  Coordinate parameters:
    int frmt_x_int = 1;
    int frmt_x_dec = 1;
    int frmt_y_int = 1;
    int frmt_y_dec = 1;
    int zeropos = 0;//o means  leading zero,1 means tail zero
    enum unit{MM,IN};
    unit unit;

    //  Generation parameters:
    int current_x = 0, current_y = 0;
    Aperture* current_aperture;
    int current_d_code;


    enum interpolation_mode{LINEAR, CLOCKWISE_CIRCULAR, COUNTERCLOCKWISE_CIRCULAR};
    interpolation_mode interpolation_mode = LINEAR;
    enum quadrant_mode{SINGLE_QUADRANT, MULTI_QUADRANT};
    quadrant_mode quadrant_mode;

    //  Aperture transformation parameters:
    enum polarity{C,D};
    polarity polarity = D;
    enum mirroring{NO_MIRRORING,X,Y,XY};
    mirroring mirroring = NO_MIRRORING;
    float rotation = -1;
    float scaling = -1;
    bool clearflag = false;

    int string_to_command(const QString);           //  Convert the command line to the enum commands type
    int string_to_extended_command(const QString);
    int string_to_units(const QString);
    int string_to_mirroring(const QString);
    int trim_D_argument(QString, const int int_format, const int dec_format, const bool minus);     //  Read the number of the coordinate line and cause it to have a decimal point.
    int trim_D_argument_nodpi(QString, const int int_format, const int dec_format, const bool minus);
    int radius_from_big_I_J(const long long int,const long long int);   //  Calculation of displacement radius of arc center along Pythagorean theorem
    void norm_angle(int*);
    void check_for_G_in_D(const QString, enum interpolation_mode*);     //  When supporting obsolete structures, G code is included in D code instructions
    int sqrt_from_big_int(const long long int);

};

#endif // PROCESSOR_H
