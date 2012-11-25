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
    convlv.cpp \
    selectcolumn.cpp \
    background.cpp \
    seloutfile.cpp \
    xrange.cpp \
    about.cpp \
    selectcolors.cpp

HEADERS  += mainscreen.h \
    Savgol.h \
    GaussSmth.h \
    Utils.h \
    config.h \
    qexception.h \
    GraphImage.h \
    convlv.h \
    selectcolumn.h \
    background.h \
    seloutfile.h \
    xrange.h \
    about.h \
    selectcolors.h

FORMS    += mainscreen.ui \
    selectcolumn.ui \
    background.ui \
    seloutfile.ui \
    xrange.ui \
    about.ui \
    selectcolors.ui

RC_FILE = mafico.rc
