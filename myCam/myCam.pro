#-------------------------------------------------
#
# Project created by QtCreator 2019-11-22T09:58:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myCam
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mycamera.cpp \
    mylcd.cpp

HEADERS  += mainwindow.h \
    mycamera.h \
    mylcd.h

FORMS    += mainwindow.ui
