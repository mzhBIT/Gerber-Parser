#ifndef DIALOGDCODE_H
#define DIALOGDCODE_H
#include <QTableView>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QPushButton>
#include <QMap>
#include "aperture.h"
class DialogDcode : public QMainWindow
{
    Q_OBJECT
public:
    explicit DialogDcode(QMainWindow *parent = nullptr);
    QTableView* tableView;
    QStandardItemModel* model;
    QPushButton* okbtn;
    QPushButton* canclebtn;

    void table_update(QMap<int,Aperture*> *aperture_dictionary,double k);
signals:

};

#endif // DIALOGDCODE_H
