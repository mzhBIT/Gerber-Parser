#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QtMath>
#include <cstdio>
#include <QDebug>
#include <QMap>
#include "processor.h"

Processor::Processor(const bool is_outline)
{    
    is_outline_flag = is_outline;
}

void Processor::set_paths(const QString gerber_file, const QString output_folder){

    QString extension = "." + image_format;
    name_of_gerber_file = gerber_file;
    output_folder_path = output_folder;
    name_of_output_file = output_folder + gerber_file.right(gerber_file.size() - gerber_file.lastIndexOf('/')) + extension;

}

int Processor::sqrt_from_big_int(const long long int value){

    double const eps = 0.1;
    double const start_value = 1;
    double height = start_value;
    double width = value;
    while (abs(width-height)>eps){
        height = (height+width)/2;
        width = value/height;
    }
    return int(height);

}

int Processor::radius_from_big_I_J(const long long i, const long long j){

    return sqrt_from_big_int(abs(i*i+j*j));

}

void Processor::set_dpi(const int new_scale){

    dpi = new_scale;

}

void Processor::set_frame_thickness(const double fr_in_mm){

    frame_thickness = fr_in_mm/mm_in_inch;

}
void Processor::set_canvas(QImage *img)
{
    pxmp = img;
}
void Processor::set_board_w_h(const double w, const double h){
    board_width = w;
    board_height = h;
}
void Processor::set_dx_dy(const double dxx, const double dyy){
    dx=dxx;
    dy=dyy;
}
void Processor::set_w_h_dx_dy(const double wdt, const double hgt, const double dxx, const double dyy){
    w=wdt;
    h=hgt;
    dx=dxx;
    dy=dyy;
}

void Processor::set_opacity_value(const float val){

    opacity_value = val;

}

void Processor::set_image_format(const QString format){

    image_format = format;

}
void Processor::set_clearflag(bool b){
    clearflag = b;
}
QString Processor::get_outline_filename(){

    return name_of_output_file;

}
void Processor::clear_data(){
    aperture_dictionary.clear();
    list_of_strings_of_gerber.clear();
}
void Processor::set_outline_file_name(const QString filename){

    name_of_outline_file = filename;

}
void Processor::set_pregroudcolor(QColor color){
    preground = color;
}
void Processor::set_offset(double x, double y){
    offset[0] =  x;
    offset[1] = y;
}
void Processor::set_parsed_file_strings(QStringList list){
    list_of_strings_of_gerber.clear();
    for(int i=0;i<list.length();++i){
        list_of_strings_of_gerber.append(list.at(i));
    }
}
int Processor::load_file(){

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
                k_mm_or_inch = mm_in_inch;

            }
            break;
        }
    }
    return list_of_strings_of_gerber.size();

}

int Processor::process(){

//  return 1    : File ended successfully
//  return -1   : Failed to open file log
//  return -2   : File is invalid
//  return -3   : Painter did not start. Maybe one file is too big. Help for 64 bit version
//  return -4   : The image was not saved. Maybe one file is too big.

    if (list_of_strings_of_gerber.isEmpty()){
        qDebug()<<"Gerber files " << name_of_gerber_file << " Cannot process because it cannot be processed. The container for file lines is empty!";
        finished();
        return -1;
    }
    //
    //  Check the appearance of single FS and Mo, otherwise Gerber is wrong.
    //
    int count_of_FS=0, count_of_MO=0;

    for (int i=0;i<list_of_strings_of_gerber.size();i++) {
        if (list_of_strings_of_gerber.at(i).contains("%FS")){
            count_of_FS++;
        }
        if (list_of_strings_of_gerber.at(i).contains("%MO")) {
            count_of_MO++;
            if (list_of_strings_of_gerber.at(i).mid(3,2).contains("MM")) {
                k_mm_or_inch = mm_in_inch;
            }            
        }
        if((count_of_FS==1)&&(count_of_MO==1))
            break;
    }
    if (!((count_of_FS==1)&&(count_of_MO==1))){
        qDebug()<<"Gerber files " << name_of_gerber_file << " The command that does not contain%FS or% Mo, or contains more than one command, cannot be processed!";
        finished();
        return -2;
    }

    frame_thickness = frame_thickness*k_mm_or_inch;
    //board_width = w + frame_thickness*2;
    //board_height = h + frame_thickness*2;

    //  Create a qimage size to match. Solutions and charges, unit: mm or inch
    //QImage pxmp(qRound(board_width*dpi/k_mm_or_inch), qRound(board_height*dpi/k_mm_or_inch), QImage::Format_RGB32);
    //pxmp = QImage(qRound(board_width*dpi/k_mm_or_inch), qRound(board_height*dpi/k_mm_or_inch), QImage::Format_RGB32);

    //  If you have a circuit diagram, you can draw an image on it, otherwise we will create a pure pixmap.
    if (name_of_outline_file!=""){
        if (pxmp->load(name_of_outline_file)){
        }
        else {
            pxmp->fill(background);
        }
    }
    else {
        pxmp->fill(background);
        if (is_outline_flag == false) {
        }
    }

    //  Drawing tool settings
    QPainter painter;
    painter.begin(pxmp);
    if (!painter.isActive()){
        qDebug()<<"QPainter not running. Gerber file " << name_of_gerber_file << " can not handle!";
        finished();
        return -3;
    }
    painter.setOpacity(qreal(opacity_value));    // Feather transparency
    painter.translate((frame_thickness-dx)*dpi/k_mm_or_inch,((board_height+dy-frame_thickness)*dpi/k_mm_or_inch));    //  Y-axis reflection, so that the coordinate origin in the lower left corner and the required displacement
    //painter.translate((frame_thickness+dx)*dpi/k_mm_or_inch,((board_height-dy-frame_thickness)*dpi/k_mm_or_inch));
    QPen global_pen(preground, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);  //  Create a silent global pen
    painter.setPen(global_pen);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.scale((1/k_mm_or_inch),(-1/k_mm_or_inch));

    //  Main loop - handles all strings
    int i=0;
    int command=0;
    int int_unit;
    int int_mirr;                                   //  Mirror mode in list enum mirroring
    QString str;                                    //  Current line from file
    QString str_command = "";

    QMap<QString,am_template*> am_template_dictionary;
    current_d_code = 1;
    while (i<list_of_strings_of_gerber.size()) {

        str = list_of_strings_of_gerber.at(i);      //  Read a line from a file
        if (str.contains("G04")){
            //  ignore.
        }
        else {
            if (str.contains("%")){
                str_command = str.mid(1,2);
                command = string_to_extended_command(str_command);      //

                switch (command) {
                    case FS :{
                    //----------
                    //   FS
                    //----------                        
                        frmt_x_int = 3;                         //  кDigital oligarchy is always 3 to avoid inconsistency with the actual coordinate data format.
                        frmt_x_dec = str.mid(7,1).toInt();      //  Number of decimal places
                        frmt_y_int = 3;
                        frmt_y_dec = str.mid(10,1).toInt();
                        if(str.mid(3,1)  ==  'T')
                            zeropos = 1;
                        //  Check for exceeding the allowable number of digits...
                        if ((frmt_x_int>7)||(frmt_x_dec>6)||(frmt_y_int>7)||(frmt_y_dec>6)||((str.mid(12,1)!="%"))){
                            qDebug()<<"Invalid FS command format " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }

                    }break;
                    case MO :{
                    //----------
                    //   MO
                    //----------                        
                        int_unit = string_to_units(str.mid(3,2));
                        switch (int_unit){
                            case MM : unit = MM; break;
                            case IN : unit = IN; break;
                            default :
                            qDebug()<<"Invalid unit command MO. Gerber file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }
                    }break;
                    case AD :{
                    //----------
                    //   AD
                    //----------
                        QString d_code_number_of_aperture = "";
                        int int_d_code_of_aperture = 0;
                        QString name_of_aperture_template = "";
                        QString type_of_aperture_template = "";
                        QString modifiers = "";                     //  Modifiers available on the command line (separated by commas)
                        int aprt_temp_index = -1;                   //  Index of the first character on the command line
                        am_template* am_pointer = nullptr;          //  Pointer buffer in the current macro template. There is no default macro template.

                        //
                        //  When the aperture (name) is read out, the initial index of aperture is found
                        //
                        for (int j=4;j<str.size();j++) {
                            if (str.at(j).isDigit()){
                                d_code_number_of_aperture.append(str.at(j));
                            }
                            else if (d_code_number_of_aperture.isEmpty()){
                                qDebug()<< "Empty d-code in AD. Gerber file  " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -2;
                            }
                            else {
                                aprt_temp_index = j;
                                break;
                            }
                        }
                        int_d_code_of_aperture = d_code_number_of_aperture.toInt();
                        //
                        //  Character readout
                        //
                        if (str.contains(',')){
                            for (int j=str.indexOf(',')+1;(str.at(j)!='*')&&(str.at(j)!=',');j++) {
                                modifiers.append(str.at(j));
                            }
                        }

                        for (int j=aprt_temp_index;(str.at(j)!='*')&&(str.at(j)!=',');j++) {
                            name_of_aperture_template.append(str.at(j));
                        }
                        if ((name_of_aperture_template.size()==1)&&(name_of_aperture_template=='C'||name_of_aperture_template=='R'||name_of_aperture_template=='O'||name_of_aperture_template=='P')){
                            type_of_aperture_template = name_of_aperture_template;
                        }
                        else{
                            type_of_aperture_template = "MACRO";
                            //
                            //  Search in the dictionary of a given name macro template. If the index is found in buffpointer
                            //
//                            for (int i=0; i<am_template_dictionary.size(); i++) {
//                                if (name_of_aperture_template == am_template_dictionary.at(i)->get_name()){
//                                    am_pointer = am_template_dictionary.at(i);
//                                    break;
//                                }
//                            }
                            am_pointer = am_template_dictionary.find(name_of_aperture_template).value();
                            if (am_pointer == nullptr){
                                qDebug()<<"An empty element was found in the macro aperture dictionary. Gerber file " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -2;
                            }
                        }
                        //
                        //  Create a window and add it to the aperture dictionary
                        //
                        Aperture *new_aperture = new Aperture(int_d_code_of_aperture, name_of_aperture_template, type_of_aperture_template, modifiers, am_pointer);
                        new_aperture->create(dpi);
                        aperture_dictionary.insert(int_d_code_of_aperture, new_aperture);
                    }break;
                    //----------
                    //   AM
                    //----------
                    case AM :{
                        QString name_of_am_template = "";
                        QStringList data_blocks;
                        //
                        //  Macro mode name readout
                        //
                        for (int j=3;(str.at(j)!='*');j++) {
                            name_of_am_template.append(str.at(j));
                        }
                        if (name_of_am_template.isEmpty()){
                            qDebug()<<"Cannot read macro template name for command AM. Gerber file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }                    
                        //  Data block readout
                        //  From% AM to% AM
                        int percent_counter=0;
                        QString tmp = "";

                        while (percent_counter<2) {
                            str = list_of_strings_of_gerber.at(i);
                            tmp = tmp+str;
                            percent_counter=percent_counter+str.count('%');
                            if (percent_counter == 1)
                                i++;
                        }
                        QStringList tmplist = tmp.split('*');
                        for(int j=0;j<tmplist.size();++j)
                        {
                            str = tmplist.at(j);
                            str.remove('\n');
                            data_blocks.append(str);
                        }
                        //  Check for empty or unnecessary strings and remove them from the list (for example, a submenu named a macro template or an end symbol defined by a template )
                        for (int j=0; j<data_blocks.size();) {
                            if (!((data_blocks.at(j).at(0).isDigit())||(data_blocks.at(j).at(0)=='$')||(data_blocks.at(j).at(0)=='-'))){
                                data_blocks.removeAt(j);
                            }
                            else {
                                j++;
                            }
                        }
                        //
                        // Create a macro template and add it to the dictionary
                        //
                        am_template *new_am_template = new am_template(name_of_am_template,data_blocks);
                        am_template_dictionary.insert(name_of_am_template, new_am_template);
                    }break;
                    case AB :{
                    //AB implementation,TODO

                    }break;
                    case LP :{
                    //----------
                    //   LP
                    //----------
                        if (str.at(3)=='C'){
                            polarity = C;
                            painter.setBrush(background);
                            global_pen.setColor(background);
                        }
                        else if (str.at(3)=='D'){
                            polarity = D;
                            painter.setBrush(preground);
                            global_pen.setColor(preground);
                        }
                        else {
                            qDebug()<<"Invalid instruction format LP. Gerber-file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }
                    }break;
                    case LM :{
                    //----------
                    //   LM
                    //----------
                        int_mirr = string_to_mirroring(str.mid(3,2));
                        switch (int_mirr) {
                            case NO_MIRRORING : mirroring = NO_MIRRORING; break;
                            case X : mirroring = X; break;
                            case Y : mirroring = Y; break;
                            case XY : mirroring = XY; break;
                        }
                    }break;
                    case LN:{

                    }break;
                    case LR :{

                    //LR implementation

                    }break;
                    case LS :{

                    //LS implementation

                    }break;
                    case TF :{

                    //The image is not affected

                    }break;
                    case TO :{


                    }break;
                    case TD :{

                    //Implement TD

                    }break;
                }//end of switch
            }//end of if (str.contains("%"))

            //
            //D-Code Search
            //
            else if (str.contains("D")||str.contains("X")||str.contains("Y")) {
//                current_d_code = 1;
                if (str.contains("D")){
                str_command = str.mid(str.indexOf('D'),3);
                command = string_to_command(str_command);
                }
                else {
                    if (command==2) {
//                        qDebug()<< "coordinate data without operation code detected...";
//                        qDebug()<< str.contains("D") << "command:" <<command;
//                        qDebug()<< "number of string:0" << i;
                    }

//                    command = current_d_code;               //  deprecated mentor...
//                    command = D01;
                }
                check_for_G_in_D(str,&interpolation_mode);          //  deprecated mentor...

                switch (command) {
                    case D01 :{
                    //----------
                    //   D01
                    //----------
//                        current_d_code = 1;                                 //  deprecated mentor...
//                        check_for_G_in_D(str,&interpolation_mode);          //  deprecated mentor...
                        int command_x = current_x, command_y = current_y;   //  New coordinates. The default is the current coordinate
                        int command_i = 0, command_j = 0;                   //  Displacement (arc center)
                        //  Read the new coordinates from the command line
                        if (str.contains('X')){
                            QString x_val;
                            bool minus=0;
                            if ((str.at(str.indexOf('X')+1)) == '-')
                                minus = 1;
                            for (int j=str.indexOf('X') + minus + 1; str.at(j).isDigit();j++) {
                                x_val.append(str.at(j));
                            }
                            command_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                        }
                        if (str.contains('Y')){
                            bool minus=0;
                            QString y_val;
                            if ((str.at(str.indexOf('Y')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('Y') + minus + 1; str.at(j).isDigit();j++) {
                                y_val.append(str.at(j));
                            }
                            command_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                        }
                        if (str.contains('I')){
                            QString i_val;
                            bool minus=0;
                            if ((str.at(str.indexOf('I')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('I') + minus + 1; str.at(j).isDigit();j++) {
                                i_val.append(str.at(j));
                            }
                            command_i = trim_D_argument(i_val, frmt_x_int, frmt_x_dec, minus);
                        }
                        if (str.contains('J')){
                            bool minus=0;
                            QString j_val;
                            if ((str.at(str.indexOf('J')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('J') + minus + 1; str.at(j).isDigit();j++) {
                                j_val.append(str.at(j));
                            }
                            command_j = trim_D_argument(j_val, frmt_y_int, frmt_y_dec, minus);
                        }
                        //
                        // Line drawing
                        //
                        if (interpolation_mode == LINEAR){
                            painter.drawLine(current_x, current_y, command_x, command_y);
                        }
                        //
                        // CLOCKWISE_CIRCULAR mode drawing
                        //
                        else if (interpolation_mode == CLOCKWISE_CIRCULAR){
                            // Arc radius
                            int R = radius_from_big_I_J(command_i,command_j);
                            // Arc center coordinates:
                            int Cx = 0;
                            int Cy = 0;
                            // Starting angle and finite angle of arc:
                            int start_angle = 0;
                            int end_angle = 0;
                            int span_angle = 0;

                            if (quadrant_mode == SINGLE_QUADRANT){
                                // Initial angle, finite angle, arc center calculation:

                                if ((current_x < command_x)&&(current_y > command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y - command_j;
                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x > command_x)&&(current_y > command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y + command_j;
                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x > command_x)&&(current_y < command_y)){
                                    Cx = current_x + command_i;
                                    Cy = current_y + command_j;
                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                }
                                else {
                                    Cx = current_x + command_i;
                                    Cy = current_y - command_j;
                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                }
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                span_angle = end_angle - start_angle;
                                if (qAbs(span_angle)>90*16){
                                    if (span_angle<0) span_angle = -90*16;
                                    else {span_angle = 90*16;}
                                }
                                // Draw an arc:
                                painter.drawArc(arc_rect, -start_angle, -span_angle);
                            }
                            else if (quadrant_mode == MULTI_QUADRANT){
                                // Arc center coordinates:
                                int Cx = current_x + command_i;
                                int Cy = current_y + command_j;
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);

                                norm_angle(&start_angle);
                                norm_angle(&end_angle);
                                if (start_angle<=end_angle){
                                    span_angle = end_angle - (start_angle + 360*16);
                                }
                                else {
                                    span_angle = end_angle - start_angle;
                                }
                                painter.drawArc(arc_rect, -start_angle, abs(span_angle));
                            }
                            else {
                                qDebug()<<"Wrong! Quadrant mode is not specified. Gerber file " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -1;
                            }
                        }
                        //---------------------------------------------------------------
                        // COUNTERCLOCKWISE_CIRCULAR
                        //---------------------------------------------------------------
                        else if (interpolation_mode==COUNTERCLOCKWISE_CIRCULAR){
                            int R = radius_from_big_I_J(command_i,command_j);

                            int Cx = 0;
                            int Cy = 0;

                            int start_angle = 0;
                            int end_angle = 0;
                            int span_angle = 0;

                            if (quadrant_mode == SINGLE_QUADRANT){

                                if ((current_x > command_x)&&(current_y < command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y - command_j;
                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x < command_x)&&(current_y < command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y + command_j;
                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x < command_x)&&(current_y > command_y)){
                                    Cx = current_x + command_i;
                                    Cy = current_y + command_j;
                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                }
                                else {
                                    Cx = current_x + command_i;
                                    Cy = current_y - command_j;
                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                }
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                span_angle = end_angle - start_angle;
                                if (qAbs(span_angle)>90*16){
                                    if (span_angle<0) span_angle = -90*16;
                                    else {span_angle = 90*16;}
                                }
                                painter.drawArc(arc_rect, -start_angle, -span_angle);
                            }
                            else if (quadrant_mode == MULTI_QUADRANT){
                                int Cx = current_x + command_i;
                                int Cy = current_y + command_j;
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                norm_angle(&start_angle);
                                norm_angle(&end_angle);
                                if (start_angle >= end_angle){
                                    span_angle = (end_angle + 360*16) - start_angle;
                                }
                                else {
                                    span_angle = end_angle - start_angle;
                                }
                                painter.drawArc(arc_rect, -start_angle, -abs(span_angle));
                            }
                            else {
                                qDebug()<<"Wrong! Quadrant mode is not specified. Gerber file " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -1;
                            }
                        }
                        else {
                            qDebug()<<"Wrong! The interpolation mode is not set. Gerber file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -1;
                        }
                        //  Update current coordinate point
                        current_x = command_x;
                        current_y = command_y;
                    }break;
                    case D02 :{
                    //----------
                    //   D02
                    //----------
//                    current_d_code = 1;                         //  deprecated mentor...
                    check_for_G_in_D(str,&interpolation_mode);  //  deprecated mentor...
                    if (str.contains('X')){
                        QString x_val;
                        bool minus=0;
                        if ((str.at(str.indexOf('X')+1))=='-')
                            minus = 1;
                        for (int j=str.indexOf('X') + minus + 1; str.at(j).isDigit();j++) {
                            x_val.append(str.at(j));
                        }
                        current_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                    }
                    if (str.contains('Y')){
                        bool minus=0;
                        QString y_val;
                        if ((str.at(str.indexOf('Y')+1))=='-')
                            minus = 1;
                        for (int j=str.indexOf('Y') + minus + 1; str.at(j).isDigit();j++) {
                            y_val.append(str.at(j));
                        }
                        current_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                    }
                    }break;
                    case D03 :{
                    //----------
                    //   D03
                    //----------                    
                        if (str.contains('X')){
                            QString x_val;
                            bool minus=0;
                            if ((str.at(str.indexOf('X')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('X') + minus + 1; str.at(j).isDigit();j++) {
                                x_val.append(str.at(j));
                            }
                            current_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                        }
                        if (str.contains('Y')){
                            bool minus=0;
                            QString y_val;
                            if ((str.at(str.indexOf('Y')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('Y') + minus + 1; str.at(j).isDigit();j++) {
                                y_val.append(str.at(j));
                            }
                            current_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                        }                        
                        current_aperture->draw_me(current_x,current_y,&painter);
                    }break;
                    case Dnn :{
                    //----------
                    //   Dnnn
                    //----------
                        //  Reading numbers in D
                        QString number_of_Dnn;
                        for (int j=str.indexOf('D')+1;str.at(j).isDigit();j++) {
                            number_of_Dnn.append(str.at(j));
                        }
                        current_aperture = aperture_dictionary.find(number_of_Dnn.toInt()).value();
                        current_aperture->number++;
                        //  If this is a circle, set to the D01 draw pen parameter.
                        //  If a file is an outline, the width of the window is ignored to avoid a very thin outline on the image..
                        //**************************************************************
                            global_pen.setWidth(current_aperture->get_std_circ_dia_in_px(dpi));
                            painter.setPen(global_pen);
                        //**************************************************************
                    }break;
                }//end of switch
            }//end of else if (str.contains("D"))
            //---------------------------------------------------------------
            // Check G and M Command
            //---------------------------------------------------------------
            else if (str.contains("G")||str.contains("M")) {
                str_command = str.mid(0,3);
                command = string_to_command(str_command);
                switch (command) {
                    case G01 :{
                    //----------
                    //   G01
                    //----------
                        interpolation_mode = LINEAR;
                    }break;
                    case G02 :{
                    //----------
                    //   G02
                    //----------
                        interpolation_mode = CLOCKWISE_CIRCULAR;
                    }break;
                    case G03 :{
                    //----------
                    //   G03
                    //----------
                        interpolation_mode = COUNTERCLOCKWISE_CIRCULAR;
                    }break;
                    case G74 :{
                    //----------
                    //   G74
                    //----------
                        quadrant_mode = SINGLE_QUADRANT;
                    }break;
                    case G75 :{
                    //----------
                    //   G75
                    //----------
                        quadrant_mode = MULTI_QUADRANT;
                    }break;
                    case G36 :{

                        bool end_of_region = false;
                        bool end_of_contour = false;
                        bool creating_contour_now = false;
                        QList <QPainterPath*> contours;
                        QPointF startpoint;

                        //  Regional main processing cycle
                        painter.save();
                        painter.setPen(Qt::NoPen);
                        while (!end_of_region) {
                            creating_contour_now = false;
                            end_of_contour = false;

                            while (!end_of_contour) {
                                //  If a new path is not created at present, this is a D01 command, and then start to create a new path.
                                str = list_of_strings_of_gerber.at(i);
                                if ((str.contains("D01"))&&(creating_contour_now == false)){
                                    startpoint.setX(current_x);
                                    startpoint.setY(current_y);
                                    QPainterPath* new_contour = new QPainterPath(startpoint);
                                    new_contour->setFillRule(Qt::WindingFill);
                                    contours.append(new_contour);
                                    creating_contour_now = true;
                                }

                                if (str.contains("D")||str.contains("X")||str.contains("Y")){
                                    if (str.contains("D")){
                                    str_command = str.mid(str.indexOf('D'),3);
                                    command = string_to_command(str_command);
                                    }
                                    else {
                                        command = D01;               //  deprecated mentor...
                                    }
                                    switch (command) {
                                        //
                                        //   D01 for regions
                                        //
                                        case D01:{
//                                        current_d_code = 1;                               //  deprecated mentor...
                                        check_for_G_in_D(str,&interpolation_mode);          //  deprecated mentor...
                                        int command_x = current_x, command_y = current_y;
                                        int command_i = 0, command_j = 0;

                                        //  Read the new coordinates from the command line
                                        if (str.contains('X')){
                                            QString x_val;
                                            bool minus=0;
                                            if ((str.at(str.indexOf('X')+1)) == '-')
                                                minus = 1;
                                            for (int i=str.indexOf('X') + minus + 1; str.at(i).isDigit();i++) {
                                                x_val.append(str.at(i));
                                            }
                                            command_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                                        }
                                        if (str.contains('Y')){
                                            bool minus=0;
                                            QString y_val;
                                            if ((str.at(str.indexOf('Y')+1))=='-')
                                                minus = 1;
                                            for (int i=str.indexOf('Y') + minus + 1; str.at(i).isDigit();i++) {
                                                y_val.append(str.at(i));
                                            }
                                            command_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                                        }
                                        if (str.contains('I')){
                                            QString i_val;
                                            bool minus=0;
                                            if ((str.at(str.indexOf('I')+1))=='-')
                                                minus = 1;
                                            for (int i=str.indexOf('I') + minus + 1; str.at(i).isDigit();i++) {
                                                i_val.append(str.at(i));
                                            }
                                            command_i = trim_D_argument(i_val, frmt_x_int, frmt_x_dec, minus);
                                        }
                                        if (str.contains('J')){
                                            bool minus=0;
                                            QString j_val;
                                            if ((str.at(str.indexOf('J')+1))=='-')
                                                minus = 1;
                                            for (int i=str.indexOf('J') + minus + 1; str.at(i).isDigit();i++) {
                                                j_val.append(str.at(i));
                                            }
                                            command_j = trim_D_argument(j_val, frmt_y_int, frmt_y_dec, minus);
                                        }

                                        if (interpolation_mode == LINEAR){
                                            contours.last()->lineTo(command_x,command_y);
                                        }
                                        else if (interpolation_mode == CLOCKWISE_CIRCULAR){
                                            int R = radius_from_big_I_J(command_i,command_j);

                                            int Cx = 0;
                                            int Cy = 0;
                                            int start_angle = 0;
                                            int end_angle = 0;
                                            int span_angle = 0;

                                            if (quadrant_mode == SINGLE_QUADRANT){
                                                if ((current_x < command_x)&&(current_y > command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x > command_x)&&(current_y > command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x > command_x)&&(current_y < command_y)){
                                                    Cx = current_x + command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                                }
                                                else {
                                                    Cx = current_x + command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                                }
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                span_angle = end_angle - start_angle;
                                                if (qAbs(span_angle)>90*16){
                                                    if (span_angle<0) span_angle = -90*16;
                                                    else {span_angle = 90*16;}
                                                }
                                                contours.last()->arcTo(arc_rect,-start_angle/16, -span_angle/16);
                                            }
                                            else if (quadrant_mode == MULTI_QUADRANT){
                                                int Cx = current_x + command_i;
                                                int Cy = current_y + command_j;
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                norm_angle(&start_angle);
                                                norm_angle(&end_angle);
                                                if (start_angle<=end_angle){
                                                    span_angle = end_angle - (start_angle + 360*16);
                                                }
                                                else {
                                                    span_angle = end_angle - start_angle;
                                                }

                                                contours.last()->arcTo(arc_rect,-start_angle/16, abs(span_angle/16));
                                            }
                                            else {
                                                qDebug()<<"Error! Quadrant mode is not installed in root. Gerber file " << name_of_gerber_file << " Can't handle!";
                                                finished();
                                                return -1;
                                            }
                                        }
                                        else if (interpolation_mode==COUNTERCLOCKWISE_CIRCULAR){
                                            int R = radius_from_big_I_J(command_i,command_j);

                                            int Cx = 0;
                                            int Cy = 0;
                                            int start_angle = 0;
                                            int end_angle = 0;
                                            int span_angle = 0;

                                            if (quadrant_mode == SINGLE_QUADRANT){
                                                if ((current_x > command_x)&&(current_y < command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x < command_x)&&(current_y < command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x < command_x)&&(current_y > command_y)){
                                                    Cx = current_x + command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                                }
                                                else {
                                                    Cx = current_x + command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                                }
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                span_angle = end_angle - start_angle;
                                                if (qAbs(span_angle)>90*16){
                                                    if (span_angle<0) span_angle = -90*16;
                                                    else {span_angle = 90*16;}
//                                                        log << time.currentTime().toString() << " D01 command (region): warning! angle of Arc > 90 in SINGLE QUADRANT mode\n";
                                                }
                                                contours.last()->arcTo(arc_rect,-start_angle/16, -span_angle/16);
                                            }
                                            else if (quadrant_mode == MULTI_QUADRANT){
                                                int Cx = current_x + command_i;
                                                int Cy = current_y + command_j;
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                norm_angle(&start_angle);
                                                norm_angle(&end_angle);
                                                if (start_angle >= end_angle){
                                                    span_angle = (end_angle + 360*16) - start_angle;
                                                }
                                                else {
                                                    span_angle = end_angle - start_angle;
                                                }
                                                contours.last()->arcTo(arc_rect,-start_angle/16, -abs(span_angle/16));
                                            }
                                            else {
                                                qDebug()<<"Wrong! No mode specified in the region. Gerber file " << name_of_gerber_file << " can't handle!";
                                                finished();
                                                return -1;
                                            }
                                        }
                                        else {
                                            qDebug()<<"Wrong! There is no interpolation mode in the region. gerber file " << name_of_gerber_file << " can't handle!";
                                            finished();
                                            return -1;
                                        }
                                        current_x = command_x;
                                        current_y = command_y;
                                        }break;
                                        case D02:{
                                            //
                                            //   D02 for regions
                                            //
//                                            current_d_code = 2;     //  deprecated mentor
                                            //  Is it the end of D02? If so, then turn off the circuit
                                            check_for_G_in_D(str,&interpolation_mode); //   deprecated mentor...
                                            if (creating_contour_now == true){
                                                end_of_contour = true;
                                            }
                                            //  Here is the standard instruction processing
                                            if (str.contains('X')){
                                                    QString x_val;
                                                    bool minus=0;
                                                    if ((str.at(str.indexOf('X')+1))=='-')
                                                        minus = 1;
                                                    for (int i=str.indexOf('X') + minus + 1; str.at(i).isDigit();i++) {
                                                        x_val.append(str.at(i));
                                                    }
                                                    current_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                                                }
                                            if (str.contains('Y')){
                                                bool minus=0;
                                                QString y_val;
                                                if ((str.at(str.indexOf('Y')+1))=='-')
                                                    minus = 1;
                                                for (int i=str.indexOf('Y') + minus + 1; str.at(i).isDigit();i++) {
                                                    y_val.append(str.at(i));
                                                }
                                                current_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                                            }
                                        }break;
                                    }//end of switch
                                }//end of if (str.contains("D"))
                                else if (str.contains("G")) {
                                    str_command = str.mid(str.indexOf('G'),3);
                                    command = string_to_command(str_command);
                                    switch (command) {
                                        case G01 :{
                                        //----------
                                        //   G01
                                        //----------
                                            interpolation_mode = LINEAR;
                                        }break;
                                        case G02 :{
                                        //----------
                                        //   G02
                                        //----------
                                            interpolation_mode = CLOCKWISE_CIRCULAR;
                                        }break;
                                        case G03 :{
                                        //----------
                                        //   G03
                                        //----------
                                            interpolation_mode = COUNTERCLOCKWISE_CIRCULAR;
                                        }break;
                                        case G74 :{
                                        //----------
                                        //   G74
                                        //----------
                                            quadrant_mode = SINGLE_QUADRANT;
                                        }break;
                                        case G75 :{
                                        //----------
                                        //   G75
                                        //----------
                                            quadrant_mode = MULTI_QUADRANT;
                                        }break;
                                    }//end of switch
                                }//end of if (str.contains("G"))

                                //................................................

                                if (str.contains("G37")){
                                    end_of_contour = true;
                                    end_of_region = true;
                                    //  Add them to the whole contour map (qpatinterpath) in the loop from the Contour array, their colors..
                                    for (int i=0;i<contours.size();i++) {
                                        painter.fillPath(*contours.at(i),preground);
                                        painter.drawPath(*contours.at(i));
                                    }
                                    //  Dynamic memory release
                                    for (int i=0;i<contours.size();i++) {
                                        delete contours.at(i);
                                    }
                                    //  Restore global settings for foreground, polarity, etc.
                                    painter.restore();
                                    break;
                                }// end of if (str.contains("G37"))

                            i++;
                            }// end of while (!end_of_contour)
#ifdef PARSER_DEBUG
//                            name_of_output_file = "E:\\1\\"+QString::number(i)+"_G36.png";
//                            parser_debug();
#endif
                        }// end of while (!end_of_region)

                    }break;
                    case M02 :{
                    //----------
                    //   M02
                    //----------
                    }break;
//                    default: log << time.currentTime().toString() << " Invalid command format: " << str_command.toUtf8() << "\n";
                }//end of switch
            }//end of else if (str.contains("G")||str.contains("M"))
        }//end of else (if (str.contains("G04")))
        i++;
    }//The main loop that selects the line in the file ends

    //  Saves qimage to the selected drawing format on disk
    painter.end();


    //
    //  Remove dynamic memory from dictionary
    //
    //aperture_dictionary.clear();
    am_template_dictionary.clear();
    //pxmp->convertTo(QImage::Format_Mono,Qt::MonoOnly);
    if(clearflag)
        this->clear_data();
    finished();         //  Main application window signal
    return 1;

}
void  Processor::set_scale(float scale){
    scaling = scale;
}
int Processor::processs_repaint(){

    //  Create a qimage size to match. Solutions and charges, unit: mm or inch
    //QImage pxmp(qRound(board_width*dpi/k_mm_or_inch), qRound(board_height*dpi/k_mm_or_inch), QImage::Format_RGB32);
    //pxmp = QImage(qRound(board_width*dpi/k_mm_or_inch), qRound(board_height*dpi/k_mm_or_inch), QImage::Format_RGB32);
    aperture_dictionary.clear();
    pxmp->fill(background);

    //  Drawing tool settings
    QPainter painter;
    painter.begin(pxmp);
    if (!painter.isActive()){
        qDebug()<<"QPainter not running. Gerber file " << name_of_gerber_file << " can not handle!";
        finished();
        return -3;
    }
    painter.setOpacity(qreal(opacity_value));    // Feather transparency
    //painter.translate((frame_thickness-dx)*dpi/k_mm_or_inch,((board_height+dy-frame_thickness)*dpi/k_mm_or_inch));    //  Y-axis reflection, so that the coordinate origin in the lower left corner and the required displacement
    //painter.translate((frame_thickness+dx)*dpi/k_mm_or_inch,((board_height-dy-frame_thickness)*dpi/k_mm_or_inch));

    int width = pxmp->width();
    int height = pxmp->height();
    double offx  = -((scaling*width-width)/2-scaling*(frame_thickness+dx)*dpi/k_mm_or_inch);
    double offy = height + (scaling*height-height)/2-scaling*(frame_thickness+dy)*dpi/k_mm_or_inch;
    //

    //painter.translate(offx,offy);
    painter.translate((frame_thickness+dx)*dpi/k_mm_or_inch,((board_height-dy-frame_thickness)*dpi/k_mm_or_inch));

    QPen global_pen(preground, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);  //  Create a silent global pen
    painter.setPen(global_pen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale((1/k_mm_or_inch),(-1  /k_mm_or_inch));

    //  Main loop - handles all strings
    int i=0;
    int command=0;
    int int_unit;
    int int_mirr;                                   //  Mirror mode in list enum mirroring
    QString str;                                    //  Current line from file
    QString str_command = "";

    QMap<QString,am_template*> am_template_dictionary;
    current_d_code = 1;
    while (i<list_of_strings_of_gerber.size()) {

        str = list_of_strings_of_gerber.at(i);      //  Read a line from a file
        if (str.contains("G04")){
            //  ignore.
        }
        else {
            if (str.contains("%")){
                str_command = str.mid(1,2);
                command = string_to_extended_command(str_command);      //

                switch (command) {
                    case FS :{
                    //----------
                    //   FS
                    //----------
                        frmt_x_int = 3;                         //  кDigital oligarchy is always 3 to avoid inconsistency with the actual coordinate data format.
                        frmt_x_dec = str.mid(7,1).toInt();      //  Number of decimal places
                        frmt_y_int = 3;
                        frmt_y_dec = str.mid(10,1).toInt();
                        if(str.mid(3,1)  ==  'T')
                            zeropos = 1;
                        //  Check for exceeding the allowable number of digits...
                        if ((frmt_x_int>7)||(frmt_x_dec>6)||(frmt_y_int>7)||(frmt_y_dec>6)||((str.mid(12,1)!="%"))){
                            qDebug()<<"Invalid FS command format " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }

                    }break;
                    case MO :{
                    //----------
                    //   MO
                    //----------
                        int_unit = string_to_units(str.mid(3,2));
                        switch (int_unit){
                            case MM : unit = MM; break;
                            case IN : unit = IN; break;
                            default :
                            qDebug()<<"Invalid unit command MO. Gerber file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }
                    }break;
                    case AD :{
                    //----------
                    //   AD
                    //----------
                        QString d_code_number_of_aperture = "";
                        int int_d_code_of_aperture = 0;
                        QString name_of_aperture_template = "";
                        QString type_of_aperture_template = "";
                        QString modifiers = "";                     //  Modifiers available on the command line (separated by commas)
                        int aprt_temp_index = -1;                   //  Index of the first character on the command line
                        am_template* am_pointer = nullptr;          //  Pointer buffer in the current macro template. There is no default macro template.

                        //
                        //  When the aperture (name) is read out, the initial index of aperture is found
                        //
                        for (int j=4;j<str.size();j++) {
                            if (str.at(j).isDigit()){
                                d_code_number_of_aperture.append(str.at(j));
                            }
                            else if (d_code_number_of_aperture.isEmpty()){
                                qDebug()<< "Empty d-code in AD. Gerber file  " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -2;
                            }
                            else {
                                aprt_temp_index = j;
                                break;
                            }
                        }
                        int_d_code_of_aperture = d_code_number_of_aperture.toInt();
                        //
                        //  Character readout
                        //
                        if (str.contains(',')){
                            for (int j=str.indexOf(',')+1;(str.at(j)!='*')&&(str.at(j)!=',');j++) {
                                modifiers.append(str.at(j));
                            }
                        }

                        for (int j=aprt_temp_index;(str.at(j)!='*')&&(str.at(j)!=',');j++) {
                            name_of_aperture_template.append(str.at(j));
                        }
                        if ((name_of_aperture_template.size()==1)&&(name_of_aperture_template=='C'||name_of_aperture_template=='R'||name_of_aperture_template=='O'||name_of_aperture_template=='P')){
                            type_of_aperture_template = name_of_aperture_template;
                        }
                        else{
                            type_of_aperture_template = "MACRO";
                            //
                            //  Search in the dictionary of a given name macro template. If the index is found in buffpointer
                            //
//                            for (int i=0; i<am_template_dictionary.size(); i++) {
//                                if (name_of_aperture_template == am_template_dictionary.at(i)->get_name()){
//                                    am_pointer = am_template_dictionary.at(i);
//                                    break;
//                                }
//                            }
                            am_pointer = am_template_dictionary.find(name_of_aperture_template).value();
                            if (am_pointer == nullptr){
                                qDebug()<<"An empty element was found in the macro aperture dictionary. Gerber file " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -2;
                            }
                        }
                        //
                        //  Create a window and add it to the aperture dictionary
                        //
                        Aperture *new_aperture = new Aperture(int_d_code_of_aperture, name_of_aperture_template, type_of_aperture_template, modifiers, am_pointer);
                        new_aperture->create(dpi);
                        aperture_dictionary.insert(int_d_code_of_aperture, new_aperture);
                    }break;
                    //----------
                    //   AM
                    //----------
                    case AM :{
                        QString name_of_am_template = "";
                        QStringList data_blocks;
                        //
                        //  Macro mode name readout
                        //
                        for (int j=3;(str.at(j)!='*');j++) {
                            name_of_am_template.append(str.at(j));
                        }
                        if (name_of_am_template.isEmpty()){
                            qDebug()<<"Cannot read macro template name for command AM. Gerber file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }
                        //  Data block readout
                        //  From% AM to% AM
                        int percent_counter=0;
                        QString tmp = "";

                        while (percent_counter<2) {
                            str = list_of_strings_of_gerber.at(i);
                            tmp = tmp+str;
                            percent_counter=percent_counter+str.count('%');
                            if (percent_counter == 1)
                                i++;
                        }
                        QStringList tmplist = tmp.split('*');
                        for(int j=0;j<tmplist.size();++j)
                        {
                            str = tmplist.at(j);
                            str.remove('\n');
                            data_blocks.append(str);
                        }
                        //  Check for empty or unnecessary strings and remove them from the list (for example, a submenu named a macro template or an end symbol defined by a template )
                        for (int j=0; j<data_blocks.size();) {
                            if (!((data_blocks.at(j).at(0).isDigit())||(data_blocks.at(j).at(0)=='$')||(data_blocks.at(j).at(0)=='-'))){
                                data_blocks.removeAt(j);
                            }
                            else {
                                j++;
                            }
                        }
                        //
                        // Create a macro template and add it to the dictionary
                        //
                        am_template *new_am_template = new am_template(name_of_am_template,data_blocks);
                        am_template_dictionary.insert(name_of_am_template, new_am_template);
                    }break;
                    case AB :{
                    //AB implementation,TODO

                    }break;
                    case LP :{
                    //----------
                    //   LP
                    //----------
                        if (str.at(3)=='C'){
                            polarity = C;
                            painter.setBrush(background);
                            global_pen.setColor(background);
                        }
                        else if (str.at(3)=='D'){
                            polarity = D;
                            painter.setBrush(preground);
                            global_pen.setColor(preground);
                        }
                        else {
                            qDebug()<<"Invalid instruction format LP. Gerber-file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -2;
                        }
                    }break;
                    case LM :{
                    //----------
                    //   LM
                    //----------
                        int_mirr = string_to_mirroring(str.mid(3,2));
                        switch (int_mirr) {
                            case NO_MIRRORING : mirroring = NO_MIRRORING; break;
                            case X : mirroring = X; break;
                            case Y : mirroring = Y; break;
                            case XY : mirroring = XY; break;
                        }
                    }break;
                    case LN:{

                    }break;
                    case LR :{

                    //LR implementation

                    }break;
                    case LS :{

                    //LS implementation

                    }break;
                    case TF :{

                    //The image is not affected

                    }break;
                    case TO :{


                    }break;
                    case TD :{

                    //Implement TD

                    }break;
                }//end of switch
            }//end of if (str.contains("%"))

            //
            //D-Code Search
            //
            else if (str.contains("D")||str.contains("X")||str.contains("Y")) {
//                current_d_code = 1;
                if (str.contains("D")){
                str_command = str.mid(str.indexOf('D'),3);
                command = string_to_command(str_command);
                }
                else {
                    if (command==2) {
//                        qDebug()<< "coordinate data without operation code detected...";
//                        qDebug()<< str.contains("D") << "command:" <<command;
//                        qDebug()<< "number of string:0" << i;
                    }

//                    command = current_d_code;               //  deprecated mentor...
//                    command = D01;
                }
                check_for_G_in_D(str,&interpolation_mode);          //  deprecated mentor...

                switch (command) {
                    case D01 :{
                    //----------
                    //   D01
                    //----------
//                        current_d_code = 1;                                 //  deprecated mentor...
//                        check_for_G_in_D(str,&interpolation_mode);          //  deprecated mentor...
                        int command_x = current_x, command_y = current_y;   //  New coordinates. The default is the current coordinate
                        int command_i = 0, command_j = 0;                   //  Displacement (arc center)
                        //  Read the new coordinates from the command line
                        if (str.contains('X')){
                            QString x_val;
                            bool minus=0;
                            if ((str.at(str.indexOf('X')+1)) == '-')
                                minus = 1;
                            for (int j=str.indexOf('X') + minus + 1; str.at(j).isDigit();j++) {
                                x_val.append(str.at(j));
                            }
                            command_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                        }
                        if (str.contains('Y')){
                            bool minus=0;
                            QString y_val;
                            if ((str.at(str.indexOf('Y')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('Y') + minus + 1; str.at(j).isDigit();j++) {
                                y_val.append(str.at(j));
                            }
                            command_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                        }
                        if (str.contains('I')){
                            QString i_val;
                            bool minus=0;
                            if ((str.at(str.indexOf('I')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('I') + minus + 1; str.at(j).isDigit();j++) {
                                i_val.append(str.at(j));
                            }
                            command_i = trim_D_argument(i_val, frmt_x_int, frmt_x_dec, minus);
                        }
                        if (str.contains('J')){
                            bool minus=0;
                            QString j_val;
                            if ((str.at(str.indexOf('J')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('J') + minus + 1; str.at(j).isDigit();j++) {
                                j_val.append(str.at(j));
                            }
                            command_j = trim_D_argument(j_val, frmt_y_int, frmt_y_dec, minus);
                        }
                        //
                        // Line drawing
                        //
                        if (interpolation_mode == LINEAR){
                            painter.drawLine(current_x, current_y, command_x, command_y);
                        }
                        //
                        // CLOCKWISE_CIRCULAR mode drawing
                        //
                        else if (interpolation_mode == CLOCKWISE_CIRCULAR){
                            // Arc radius
                            int R = radius_from_big_I_J(command_i,command_j);
                            // Arc center coordinates:
                            int Cx = 0;
                            int Cy = 0;
                            // Starting angle and finite angle of arc:
                            int start_angle = 0;
                            int end_angle = 0;
                            int span_angle = 0;

                            if (quadrant_mode == SINGLE_QUADRANT){
                                // Initial angle, finite angle, arc center calculation:

                                if ((current_x < command_x)&&(current_y > command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y - command_j;
                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x > command_x)&&(current_y > command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y + command_j;
                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x > command_x)&&(current_y < command_y)){
                                    Cx = current_x + command_i;
                                    Cy = current_y + command_j;
                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                }
                                else {
                                    Cx = current_x + command_i;
                                    Cy = current_y - command_j;
                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                }
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                span_angle = end_angle - start_angle;
                                if (qAbs(span_angle)>90*16){
                                    if (span_angle<0) span_angle = -90*16;
                                    else {span_angle = 90*16;}
                                }
                                // Draw an arc:
                                painter.drawArc(arc_rect, -start_angle, -span_angle);
                            }
                            else if (quadrant_mode == MULTI_QUADRANT){
                                // Arc center coordinates:
                                int Cx = current_x + command_i;
                                int Cy = current_y + command_j;
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);

                                norm_angle(&start_angle);
                                norm_angle(&end_angle);
                                if (start_angle<=end_angle){
                                    span_angle = end_angle - (start_angle + 360*16);
                                }
                                else {
                                    span_angle = end_angle - start_angle;
                                }
                                painter.drawArc(arc_rect, -start_angle, abs(span_angle));
                            }
                            else {
                                qDebug()<<"Wrong! Quadrant mode is not specified. Gerber file " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -1;
                            }
                        }
                        //---------------------------------------------------------------
                        // COUNTERCLOCKWISE_CIRCULAR
                        //---------------------------------------------------------------
                        else if (interpolation_mode==COUNTERCLOCKWISE_CIRCULAR){
                            int R = radius_from_big_I_J(command_i,command_j);

                            int Cx = 0;
                            int Cy = 0;

                            int start_angle = 0;
                            int end_angle = 0;
                            int span_angle = 0;

                            if (quadrant_mode == SINGLE_QUADRANT){

                                if ((current_x > command_x)&&(current_y < command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y - command_j;
                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x < command_x)&&(current_y < command_y)){
                                    Cx = current_x - command_i;
                                    Cy = current_y + command_j;
                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                }
                                else if ((current_x < command_x)&&(current_y > command_y)){
                                    Cx = current_x + command_i;
                                    Cy = current_y + command_j;
                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                }
                                else {
                                    Cx = current_x + command_i;
                                    Cy = current_y - command_j;
                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                }
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                span_angle = end_angle - start_angle;
                                if (qAbs(span_angle)>90*16){
                                    if (span_angle<0) span_angle = -90*16;
                                    else {span_angle = 90*16;}
                                }
                                painter.drawArc(arc_rect, -start_angle, -span_angle);
                            }
                            else if (quadrant_mode == MULTI_QUADRANT){
                                int Cx = current_x + command_i;
                                int Cy = current_y + command_j;
                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                norm_angle(&start_angle);
                                norm_angle(&end_angle);
                                if (start_angle >= end_angle){
                                    span_angle = (end_angle + 360*16) - start_angle;
                                }
                                else {
                                    span_angle = end_angle - start_angle;
                                }
                                painter.drawArc(arc_rect, -start_angle, -abs(span_angle));
                            }
                            else {
                                qDebug()<<"Wrong! Quadrant mode is not specified. Gerber file " << name_of_gerber_file << " Can't handle!";
                                finished();
                                return -1;
                            }
                        }
                        else {
                            qDebug()<<"Wrong! The interpolation mode is not set. Gerber file " << name_of_gerber_file << " Can't handle!";
                            finished();
                            return -1;
                        }
                        //  Update current coordinate point
                        current_x = command_x;
                        current_y = command_y;
                    }break;
                    case D02 :{
                    //----------
                    //   D02
                    //----------
//                    current_d_code = 1;                         //  deprecated mentor...
                    check_for_G_in_D(str,&interpolation_mode);  //  deprecated mentor...
                    if (str.contains('X')){
                        QString x_val;
                        bool minus=0;
                        if ((str.at(str.indexOf('X')+1))=='-')
                            minus = 1;
                        for (int j=str.indexOf('X') + minus + 1; str.at(j).isDigit();j++) {
                            x_val.append(str.at(j));
                        }
                        current_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                    }
                    if (str.contains('Y')){
                        bool minus=0;
                        QString y_val;
                        if ((str.at(str.indexOf('Y')+1))=='-')
                            minus = 1;
                        for (int j=str.indexOf('Y') + minus + 1; str.at(j).isDigit();j++) {
                            y_val.append(str.at(j));
                        }
                        current_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                    }
                    }break;
                    case D03 :{
                    //----------
                    //   D03
                    //----------
                        if (str.contains('X')){
                            QString x_val;
                            bool minus=0;
                            if ((str.at(str.indexOf('X')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('X') + minus + 1; str.at(j).isDigit();j++) {
                                x_val.append(str.at(j));
                            }
                            current_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                        }
                        if (str.contains('Y')){
                            bool minus=0;
                            QString y_val;
                            if ((str.at(str.indexOf('Y')+1))=='-')
                                minus = 1;
                            for (int j=str.indexOf('Y') + minus + 1; str.at(j).isDigit();j++) {
                                y_val.append(str.at(j));
                            }
                            current_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                        }
                        current_aperture->draw_me(current_x,current_y,&painter);
                    }break;
                    case Dnn :{
                    //----------
                    //   Dnnn
                    //----------
                        //  Reading numbers in D
                        QString number_of_Dnn;
                        for (int j=str.indexOf('D')+1;str.at(j).isDigit();j++) {
                            number_of_Dnn.append(str.at(j));
                        }
                        current_aperture = aperture_dictionary.find(number_of_Dnn.toInt()).value();

                        //  If this is a circle, set to the D01 draw pen parameter.
                        //  If a file is an outline, the width of the window is ignored to avoid a very thin outline on the image..
                        //**************************************************************
                            global_pen.setWidth(current_aperture->get_std_circ_dia_in_px(dpi));
                            painter.setPen(global_pen);
                        //**************************************************************
                    }break;
                }//end of switch
            }//end of else if (str.contains("D"))
            //---------------------------------------------------------------
            // Check G and M Command
            //---------------------------------------------------------------
            else if (str.contains("G")||str.contains("M")) {
                str_command = str.mid(0,3);
                command = string_to_command(str_command);
                switch (command) {
                    case G01 :{
                    //----------
                    //   G01
                    //----------
                        interpolation_mode = LINEAR;
                    }break;
                    case G02 :{
                    //----------
                    //   G02
                    //----------
                        interpolation_mode = CLOCKWISE_CIRCULAR;
                    }break;
                    case G03 :{
                    //----------
                    //   G03
                    //----------
                        interpolation_mode = COUNTERCLOCKWISE_CIRCULAR;
                    }break;
                    case G74 :{
                    //----------
                    //   G74
                    //----------
                        quadrant_mode = SINGLE_QUADRANT;
                    }break;
                    case G75 :{
                    //----------
                    //   G75
                    //----------
                        quadrant_mode = MULTI_QUADRANT;
                    }break;
                    case G36 :{

                        bool end_of_region = false;
                        bool end_of_contour = false;
                        bool creating_contour_now = false;
                        QList <QPainterPath*> contours;
                        QPointF startpoint;

                        //  Regional main processing cycle
                        painter.save();
                        painter.setPen(Qt::NoPen);
                        while (!end_of_region) {
                            creating_contour_now = false;
                            end_of_contour = false;

                            while (!end_of_contour) {
                                //  If a new path is not created at present, this is a D01 command, and then start to create a new path.
                                str = list_of_strings_of_gerber.at(i);
                                if ((str.contains("D01"))&&(creating_contour_now == false)){
                                    startpoint.setX(current_x);
                                    startpoint.setY(current_y);
                                    QPainterPath* new_contour = new QPainterPath(startpoint);
                                    new_contour->setFillRule(Qt::WindingFill);
                                    contours.append(new_contour);
                                    creating_contour_now = true;
                                }

                                if (str.contains("D")||str.contains("X")||str.contains("Y")){
                                    if (str.contains("D")){
                                    str_command = str.mid(str.indexOf('D'),3);
                                    command = string_to_command(str_command);
                                    }
                                    else {
                                        command = D01;               //  deprecated mentor...
                                    }
                                    switch (command) {
                                        //
                                        //   D01 for regions
                                        //
                                        case D01:{
//                                        current_d_code = 1;                               //  deprecated mentor...
                                        check_for_G_in_D(str,&interpolation_mode);          //  deprecated mentor...
                                        int command_x = current_x, command_y = current_y;
                                        int command_i = 0, command_j = 0;

                                        //  Read the new coordinates from the command line
                                        if (str.contains('X')){
                                            QString x_val;
                                            bool minus=0;
                                            if ((str.at(str.indexOf('X')+1)) == '-')
                                                minus = 1;
                                            for (int i=str.indexOf('X') + minus + 1; str.at(i).isDigit();i++) {
                                                x_val.append(str.at(i));
                                            }
                                            command_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                                        }
                                        if (str.contains('Y')){
                                            bool minus=0;
                                            QString y_val;
                                            if ((str.at(str.indexOf('Y')+1))=='-')
                                                minus = 1;
                                            for (int i=str.indexOf('Y') + minus + 1; str.at(i).isDigit();i++) {
                                                y_val.append(str.at(i));
                                            }
                                            command_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                                        }
                                        if (str.contains('I')){
                                            QString i_val;
                                            bool minus=0;
                                            if ((str.at(str.indexOf('I')+1))=='-')
                                                minus = 1;
                                            for (int i=str.indexOf('I') + minus + 1; str.at(i).isDigit();i++) {
                                                i_val.append(str.at(i));
                                            }
                                            command_i = trim_D_argument(i_val, frmt_x_int, frmt_x_dec, minus);
                                        }
                                        if (str.contains('J')){
                                            bool minus=0;
                                            QString j_val;
                                            if ((str.at(str.indexOf('J')+1))=='-')
                                                minus = 1;
                                            for (int i=str.indexOf('J') + minus + 1; str.at(i).isDigit();i++) {
                                                j_val.append(str.at(i));
                                            }
                                            command_j = trim_D_argument(j_val, frmt_y_int, frmt_y_dec, minus);
                                        }

                                        if (interpolation_mode == LINEAR){
                                            contours.last()->lineTo(command_x,command_y);
                                        }
                                        else if (interpolation_mode == CLOCKWISE_CIRCULAR){
                                            int R = radius_from_big_I_J(command_i,command_j);

                                            int Cx = 0;
                                            int Cy = 0;
                                            int start_angle = 0;
                                            int end_angle = 0;
                                            int span_angle = 0;

                                            if (quadrant_mode == SINGLE_QUADRANT){
                                                if ((current_x < command_x)&&(current_y > command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x > command_x)&&(current_y > command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x > command_x)&&(current_y < command_y)){
                                                    Cx = current_x + command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                                }
                                                else {
                                                    Cx = current_x + command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                                }
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                span_angle = end_angle - start_angle;
                                                if (qAbs(span_angle)>90*16){
                                                    if (span_angle<0) span_angle = -90*16;
                                                    else {span_angle = 90*16;}
                                                }
                                                contours.last()->arcTo(arc_rect,-start_angle/16, -span_angle/16);
                                            }
                                            else if (quadrant_mode == MULTI_QUADRANT){
                                                int Cx = current_x + command_i;
                                                int Cy = current_y + command_j;
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                norm_angle(&start_angle);
                                                norm_angle(&end_angle);
                                                if (start_angle<=end_angle){
                                                    span_angle = end_angle - (start_angle + 360*16);
                                                }
                                                else {
                                                    span_angle = end_angle - start_angle;
                                                }

                                                contours.last()->arcTo(arc_rect,-start_angle/16, abs(span_angle/16));
                                            }
                                            else {
                                                qDebug()<<"Error! Quadrant mode is not installed in root. Gerber file " << name_of_gerber_file << " Can't handle!";
                                                finished();
                                                return -1;
                                            }
                                        }
                                        else if (interpolation_mode==COUNTERCLOCKWISE_CIRCULAR){
                                            int R = radius_from_big_I_J(command_i,command_j);

                                            int Cx = 0;
                                            int Cy = 0;
                                            int start_angle = 0;
                                            int end_angle = 0;
                                            int span_angle = 0;

                                            if (quadrant_mode == SINGLE_QUADRANT){
                                                if ((current_x > command_x)&&(current_y < command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x < command_x)&&(current_y < command_y)){
                                                    Cx = current_x - command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = qRound(-atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = qRound(-atan2(Cy-command_y, command_x-Cx)*16*180/pi);
                                                }
                                                else if ((current_x < command_x)&&(current_y > command_y)){
                                                    Cx = current_x + command_i;
                                                    Cy = current_y + command_j;
                                                    start_angle = 180*16 + qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 + qRound(atan2(Cy-command_y, Cx-command_x)*16*180/pi);
                                                }
                                                else {
                                                    Cx = current_x + command_i;
                                                    Cy = current_y - command_j;
                                                    start_angle = 180*16 - qRound(atan2(command_j,command_i)*16*180/pi);
                                                    end_angle = 180*16 - qRound(atan2(command_y-Cy, Cx-command_x)*16*180/pi);
                                                }
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                span_angle = end_angle - start_angle;
                                                if (qAbs(span_angle)>90*16){
                                                    if (span_angle<0) span_angle = -90*16;
                                                    else {span_angle = 90*16;}
//                                                        log << time.currentTime().toString() << " D01 command (region): warning! angle of Arc > 90 in SINGLE QUADRANT mode\n";
                                                }
                                                contours.last()->arcTo(arc_rect,-start_angle/16, -span_angle/16);
                                            }
                                            else if (quadrant_mode == MULTI_QUADRANT){
                                                int Cx = current_x + command_i;
                                                int Cy = current_y + command_j;
                                                QRect arc_rect(Cx-R, Cy-R, R*2, R*2);
                                                start_angle = qRound(atan2(-command_j,-command_i)*16*180/pi);
                                                end_angle = qRound(atan2(command_y-Cy, command_x-Cx)*16*180/pi);
                                                norm_angle(&start_angle);
                                                norm_angle(&end_angle);
                                                if (start_angle >= end_angle){
                                                    span_angle = (end_angle + 360*16) - start_angle;
                                                }
                                                else {
                                                    span_angle = end_angle - start_angle;
                                                }
                                                contours.last()->arcTo(arc_rect,-start_angle/16, -abs(span_angle/16));
                                            }
                                            else {
                                                qDebug()<<"Wrong! No mode specified in the region. Gerber file " << name_of_gerber_file << " can't handle!";
                                                finished();
                                                return -1;
                                            }
                                        }
                                        else {
                                            qDebug()<<"Wrong! There is no interpolation mode in the region. gerber file " << name_of_gerber_file << " can't handle!";
                                            finished();
                                            return -1;
                                        }
                                        current_x = command_x;
                                        current_y = command_y;
                                        }break;
                                        case D02:{
                                            //
                                            //   D02 for regions
                                            //
//                                            current_d_code = 2;     //  deprecated mentor
                                            //  Is it the end of D02? If so, then turn off the circuit
                                            check_for_G_in_D(str,&interpolation_mode); //   deprecated mentor...
                                            if (creating_contour_now == true){
                                                end_of_contour = true;
                                            }
                                            //  Here is the standard instruction processing
                                            if (str.contains('X')){
                                                    QString x_val;
                                                    bool minus=0;
                                                    if ((str.at(str.indexOf('X')+1))=='-')
                                                        minus = 1;
                                                    for (int i=str.indexOf('X') + minus + 1; str.at(i).isDigit();i++) {
                                                        x_val.append(str.at(i));
                                                    }
                                                    current_x = trim_D_argument(x_val, frmt_x_int, frmt_x_dec, minus);
                                                }
                                            if (str.contains('Y')){
                                                bool minus=0;
                                                QString y_val;
                                                if ((str.at(str.indexOf('Y')+1))=='-')
                                                    minus = 1;
                                                for (int i=str.indexOf('Y') + minus + 1; str.at(i).isDigit();i++) {
                                                    y_val.append(str.at(i));
                                                }
                                                current_y = trim_D_argument(y_val, frmt_y_int, frmt_y_dec, minus);
                                            }
                                        }break;
                                    }//end of switch
                                }//end of if (str.contains("D"))
                                else if (str.contains("G")) {
                                    str_command = str.mid(str.indexOf('G'),3);
                                    command = string_to_command(str_command);
                                    switch (command) {
                                        case G01 :{
                                        //----------
                                        //   G01
                                        //----------
                                            interpolation_mode = LINEAR;
                                        }break;
                                        case G02 :{
                                        //----------
                                        //   G02
                                        //----------
                                            interpolation_mode = CLOCKWISE_CIRCULAR;
                                        }break;
                                        case G03 :{
                                        //----------
                                        //   G03
                                        //----------
                                            interpolation_mode = COUNTERCLOCKWISE_CIRCULAR;
                                        }break;
                                        case G74 :{
                                        //----------
                                        //   G74
                                        //----------
                                            quadrant_mode = SINGLE_QUADRANT;
                                        }break;
                                        case G75 :{
                                        //----------
                                        //   G75
                                        //----------
                                            quadrant_mode = MULTI_QUADRANT;
                                        }break;
                                    }//end of switch
                                }//end of if (str.contains("G"))

                                //................................................

                                if (str.contains("G37")){
                                    end_of_contour = true;
                                    end_of_region = true;
                                    //  Add them to the whole contour map (qpatinterpath) in the loop from the Contour array, their colors..
                                    for (int i=0;i<contours.size();i++) {
                                        painter.fillPath(*contours.at(i),preground);
                                        painter.drawPath(*contours.at(i));
                                    }
                                    //  Dynamic memory release
                                    for (int i=0;i<contours.size();i++) {
                                        delete contours.at(i);
                                    }
                                    //  Restore global settings for foreground, polarity, etc.
                                    painter.restore();
                                    break;
                                }// end of if (str.contains("G37"))

                            i++;
                            }// end of while (!end_of_contour)
#ifdef PARSER_DEBUG
//                            name_of_output_file = "E:\\1\\"+QString::number(i)+"_G36.png";
//                            parser_debug();
#endif
                        }// end of while (!end_of_region)

                    }break;
                    case M02 :{
                    //----------
                    //   M02
                    //----------
                    }break;
//                    default: log << time.currentTime().toString() << " Invalid command format: " << str_command.toUtf8() << "\n";
                }//end of switch
            }//end of else if (str.contains("G")||str.contains("M"))
        }//end of else (if (str.contains("G04")))
        i++;
    }//The main loop that selects the line in the file ends

    //  Saves qimage to the selected drawing format on disk
    painter.end();


    //
    //  Remove dynamic memory from dictionary
    //
    //aperture_dictionary.clear();
    am_template_dictionary.clear();
    if(clearflag)
        this->clear_data();
    finished();         //  Main application window signal
    return 1;

}
void Processor::parser_debug(){
        QFile file(name_of_output_file);
        file.open(QIODevice::WriteOnly);
        if (image_format == "bmp"){
            if (!pxmp->save(&file,"BMP")) {
                qDebug()<<"Wrong! Gerber file " << name_of_gerber_file << " Unable to save image!";
            }
        }
        else if (image_format == "png"){
            if (!pxmp->save(&file,"PNG")){
                qDebug()<<"Wrong! Gerber file " << name_of_gerber_file << " Unable to save image!";
            }
        }
}
int Processor::string_to_command(const QString str){

    if (str=="D01")       return D01;
    else if (str=="D02")  return D02;
    else if (str=="D03")  return D03;
    else if (str=="G01")  return G01;
    else if (str=="G02")  return G02;
    else if (str=="G03")  return G03;
    else if (str=="G74")  return G74;
    else if (str=="G75")  return G75;
    else if (str=="G36")  return G36;
    else if (str=="G37")  return G37;
    else if (str=="G04")  return G04;
    else if (str=="M02")  return M02;
    else if (str.contains('D'))  return Dnn;
    return -1;

}

int Processor::string_to_extended_command(const QString str){

    if (str=="FS")       return FS;
    else if (str=="MO")  return MO;
    else if (str=="AD")  return AD;
    else if (str=="AM")  return AM;
    else if (str=="AB")  return AB;
    else if (str=="LP")  return LP;
    else if (str=="LM")  return LM;
    else if (str=="LR")  return LR;
    else if (str=="LS")  return LS;
    else if (str=="TF")  return TF;
    else if (str=="TA")  return TA;
    else if (str=="TO")  return TO;
    else if (str=="TD")  return TD;
    return -1;

}

int Processor::string_to_units(const QString str){

    if (str=="MM")       return MM;
    else if (str=="IN")  return IN;
    return -1;

}

int Processor::string_to_mirroring(const QString str){

    if (str=="XY")                  return XY;
    else if(str.mid(1,1)=='*'||str.mid(1,1)=='%')
    {
        if (str.left(1)=='X')       return X;
        else if (str.left(1)=='Y')  return Y;
        else if (str.left(1)=='N')  return NO_MIRRORING;
    }
    return -1;

}

int Processor::trim_D_argument(QString str, const int int_format, const int dec_format, const bool minus){

    if (str.size()<(int_format+dec_format)) {
        int difference = int_format+dec_format-str.size();
        if(zeropos == 0){
            for (int i=0;i<difference;i++) {
                str = str.insert(0,'0');
            }
        }
        else{
            for (int i=0;i<difference;i++) {
                str = str+'0';
            }
        }
    }
    str = str.insert(int_format,'.');
    if (minus){
        str.insert(0,'-');
    }
    return qRound((str.toDouble())*dpi);

}
int Processor::trim_D_argument_nodpi(QString str, const int int_format, const int dec_format, const bool minus){

    if (str.size()<(int_format+dec_format)) {
        int difference = int_format+dec_format-str.size();
        for (int i=0;i<difference;i++) {
            str = str.insert(0,'0');
        }
    }
    str = str.insert(int_format,'.');
    if (minus){
        str.insert(0,'-');
    }
    return qRound((str.toDouble()));

}
void Processor::norm_angle(int* angle){

    if (*angle<0){
         *angle = (360*16) + *angle;
    }

}

void Processor::check_for_G_in_D(const QString str, enum interpolation_mode* mode){

    if (str.contains("G01")||str.contains("G1")){
        *mode = LINEAR;
    }
    else if (str.contains("G02")||str.contains("G2")){
        *mode = CLOCKWISE_CIRCULAR;
    }
    else if (str.contains("G03")||str.contains("G3")){
        *mode = COUNTERCLOCKWISE_CIRCULAR;
    }

}

void Processor::get_outline_size(double *width, double *height, double *dx, double *dy){

    int min_x=2100000000, max_x=0, min_y=2100000000, max_y=0;
    int x_val, y_val;
    int frmt_int = 3, frmt_dec=1;
    QString str, str_val;
    bool minus=0;

    for (int i=0;i<list_of_strings_of_gerber.size();i++) {
        str = list_of_strings_of_gerber.at(i);
//        minus=false;
        //
        //  X
        //
        if (str.contains("%FS")){
            frmt_dec = str.mid(7,1).toInt();
            if(str.mid(3,1) == 'T'){
                zeropos = 1;
            }
        }
        if ((str.contains('X'))&&(!(str.contains('%')))&&(!(str.contains(',')))){
            minus=false;
            str_val="";
            if ((str.at(str.indexOf('X')+1)) == '-'){
                minus = true;
            }
            for (int i = str.indexOf('X') + minus + 1; str.at(i).isDigit();i++) {
                str_val.append(str.at(i));

            }
            x_val = trim_D_argument(str_val, frmt_int, frmt_dec, minus);
            if (x_val < min_x){
                min_x = x_val;
            }
            if (x_val > max_x) {
                max_x = x_val;
            }
        }
        //
        //  Y
        //
        if ((str.contains('Y'))&&(!(str.contains('%')))&&(!(str.contains(',')))){
            minus=false;
            str_val="";
            if ((str.at(str.indexOf('Y')+1)) == '-'){
                minus = true;
            }
            for (int i = str.indexOf('Y') + minus + 1; str.at(i).isDigit();i++) {
                str_val.append(str.at(i));
            }
            y_val = trim_D_argument(str_val, frmt_int, frmt_dec, minus);
            if (y_val < min_y){
                min_y = y_val;
            }
            if (y_val > max_y) {
                max_y = y_val;
            }
        }
    }
    *dx = double(min_x)/dpi;
    *dy = double(min_y)/dpi;

    *width = double(max_x-min_x)/dpi;
    *height = double(max_y-min_y)/dpi;
}

