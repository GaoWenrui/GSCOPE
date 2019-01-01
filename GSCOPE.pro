#-------------------------------------------------
#
# Project created by QtCreator 2018-07-23T22:51:15
#
#-------------------------------------------------

QT       += core gui opengl printsupport\

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GSCOPE
TEMPLATE = app


SOURCES += main.cpp\
        gscopemainwindow.cpp \
    calibrationdlg.cpp \
    operationcontroldlg.cpp \
    datalogctrldlg.cpp \
    registerinfodlg.cpp \
    serial/impl/list_ports/list_ports_linux.cc \
    serial/impl/list_ports/list_ports_osx.cc \
    serial/impl/list_ports/list_ports_win.cc \
    serial/impl/unix.cc \
    serial/impl/win.cc \
    serial/serial.cc \
    serialportdlg.cpp \
    thread/serialprocessobj.cpp \
    plot/qcustomplot.cpp \
    plot/axistag.cpp \
    glwidget/glwidget.cpp \
    thread/serialloopthread.cpp \
    thread/serialwriteobj.cpp \
    thread/serialdataformat.cpp

HEADERS  += gscopemainwindow.h \
    calibrationdlg.h \
    operationcontroldlg.h \
    datalogctrldlg.h \
    registerinfodlg.h \
    serialportdlg.h \
    serial/impl/unix.h \
    serial/impl/win.h \
    serial/serial.h \
    serial/v8stdint.h \
    thread/threadsafequeue.hpp \
    thread/serialdataformat.h \
    thread/serialprocessobj.h \
    thread/serialloopthread.h \
    plot/qcustomplot.h \
    plot/axistag.h \
    glwidget/glwidget.h \
    thread/serialwriteobj.h

FORMS    += gscopemainwindow.ui

RESOURCES += \
    gscoperes.qrc

win32 {

LIBS += -lhid -lsetupapi

}
