#-------------------------------------------------
#
# Project created by QtCreator 2022-01-24T19:02:34
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++

TARGET = PolygonEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

RC_ICONS += A.ico
RC_LANG = 0x0004
VERSION = 0.1.0
QMAKE_TARGET_DESCRIPTION = "A program for editing polygons"
QMAKE_TARGET_COPYRIGHT = "jkjkil4"

SOURCES += \
        AddBackgroundDialog/addbackgrounddialog.cpp \
        SideListWidget/sidelistwidget.cpp \
        Viewport/viewport.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        AddBackgroundDialog/addbackgrounddialog.h \
        SideListWidget/sidelistwidget.h \
        Viewport/viewport.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc
