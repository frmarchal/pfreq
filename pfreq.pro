#-------------------------------------------------
#
# Project created by QtCreator 2012-07-23T18:02:46
#
#-------------------------------------------------

QT       += core gui

TARGET = pfreq
TEMPLATE = app


SOURCES += main.cpp\
        mainscreen.cpp \
    Savgol.cpp \
    GaussSmth.cpp \
    Utils.cpp \
    config.cpp \
    qexception.cpp \
    GraphImage.cpp \
    convlv.cpp

HEADERS  += mainscreen.h \
    Savgol.h \
    GaussSmth.h \
    Utils.h \
    config.h \
    qexception.h \
    GraphImage.h \
    convlv.h

FORMS    += mainscreen.ui


