#-------------------------------------------------
#
# Project created by QtCreator 2024-12-04T14:33:39
#
#-------------------------------------------------

QT       += core gui network

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CaseyMgmtTunnel
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    caseymgmtsrv.cpp

HEADERS  += mainwindow.h \
    caseymgmtprot.h \
    caseymgmtsrv.h

FORMS    += mainwindow.ui
