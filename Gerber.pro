#-------------------------------------------------
#
# Project created by QtCreator 2021-02-03T17:51:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gerber
TEMPLATE = app


SOURCES += main.cpp\
        am_template.cpp \
        aperture.cpp \
        dialogdcode.cpp \
        imglabel.cpp \
        mainwindow.cpp \
        processor.cpp

HEADERS  += mainwindow.h \
    am_template.h \
    aperture.h \
    dialogdcode.h \
    imglabel.h \
    processor.h \
    ui_mainwindow.h

FORMS    += mainwindow.ui


RESOURCES += \
    ../Pic/imgsrc.qrc
