QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StackCPU
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    stackcpu.cpp

HEADERS  += mainwindow.h \
    stackcpu.h

FORMS    += mainwindow.ui
CONFIG += c++11
