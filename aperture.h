#ifndef APERTURE_H
#define APERTURE_H

#include "am_template.h"
#include <QString>
#include <QPainter>
#include <QTextStream>
#include <QPainterPath>
class Aperture
{

    am_template* my_am_template;    //  Macro indicator defined by am command

    //  Aperture Options:
    int d_code = -1;                //  Aperture number
    QString name_of_template = "";  //  Aperture template name
    QString type_of_template = "";  //  STANDARD_C, R, O, P or MACRO
    QString modifiers = "";         //  Figure required parameter line,size,etc.
    QStringList mod_list;           //

    //  Coordinate Format:
    int x_int;  //  Number of whole blocks
    int x_dec;  //  Decimal places
    int y_int;
    int y_dec;

    struct primitive_struct{
        QPainterPath path;                  //  original image
        float rotation = 0;                 //  Rotation angle
        bool std_aperture = 1;
    };

    QList <primitive_struct> primitives;    //Macro template list

    struct variable{
        int index=0;
        float value=0;
    };

    //  Modify the expression calculation from the original description of the macro pattern
    float calculate_expression(const QString expression, QList<variable>* dict);

public:
    Aperture(const int d_code_number_of_aperture = 0, const QString name_of_temp = "", const QString type_of_temp = "", const QString modifs = "", am_template* am_temp = nullptr);

    int get_d_code();                   // Get aperture number
    QString get_name();
    QString get_type();

    void create(const int dpi);
    int draw_me(const int x_pos, const int y_pos, QPainter* painter);
    int get_std_circ_dia_in_px(const int dpi);                          //  Obtain the diameter of the drawing line of the standard circular aperture
    double line_width = 0;
    int number = 0;
};

#endif // APERTURE_H
