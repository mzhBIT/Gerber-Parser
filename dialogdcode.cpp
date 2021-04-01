#include "dialogdcode.h"
#include <QTableView>
#include <QHeaderView>
#include <QDebug>
DialogDcode::DialogDcode(QMainWindow *parent) : QMainWindow(parent)
{
    this->setGeometry(100,100,500,500);
    this->setWindowTitle("线宽D码表");
    this->setWindowFlag(Qt::Drawer);
    tableView = new QTableView(this);
    tableView->setGeometry(0,0,500,400);
    okbtn = new QPushButton(this);
    okbtn->setGeometry(50,450,60,30);
    okbtn->setText("确认");
    canclebtn = new QPushButton(this);
    canclebtn->setGeometry(390,450,60,30);
    canclebtn->setText("取消");
    connect(canclebtn,&QPushButton::clicked,this,[this](){this->close();});
}
void DialogDcode::table_update(QMap<int,Aperture*> *aperture_dictionary,double k){
    if(aperture_dictionary !=0){
        QStandardItemModel* model = new QStandardItemModel(this);
        model->setHorizontalHeaderLabels({"D码", "类型","数目", "原始线宽(mm)", "当前线宽(mm)"});
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableView->verticalHeader()->hide();
        QMap<int, Aperture*>::const_iterator i = aperture_dictionary->constBegin();
        int cnt = 0;
        while (i != aperture_dictionary->constEnd()) {
            model->setItem(cnt,0,new QStandardItem(QString::number(i.key())));
            model->setItem(cnt,1,new QStandardItem((i.value())->get_type()));
            model->setItem(cnt,2,new QStandardItem(QString::number((i.value())->number)));
            model->setItem(cnt,3,new QStandardItem(QString::number((i.value())->line_width*25.4/k)));
            model->setItem(cnt,4,new QStandardItem(QString::number((i.value())->line_width*25.4/k)));
            //tableView->setItemDelegateForRow(cnt, readOnlyDelegate);
            for(int k=0;k<5;++k){
                QStandardItem* item = model->item(cnt,k);
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            }
            ++i;
            ++cnt;
        }
        tableView->setModel(model);
        this->show();
    }
}
