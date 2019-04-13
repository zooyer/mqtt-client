#-------------------------------------------------
#
# Project created by QtCreator 2019-04-08T13:36:59
#
#-------------------------------------------------

QT       += core gui network

VERSION   = 1.0.1

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MqttClient
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


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    about.cpp \
    connectionbutton.cpp \
    connectionwidget.cpp \
    lineedit.cpp \
    mqttwidget.cpp \
    tabledelegate.cpp \
    messageviewer.cpp \
    mqttexception.cpp \
    license.cpp \
    logger.cpp

HEADERS += \
        mainwindow.h \
    about.h \
    connectionbutton.h \
    connectionwidget.h \
    lineedit.h \
    mqttwidget.h \
    tabledelegate.h \
    messageviewer.h \
    mqttexception.h \
    license.h \
    logger.h

FORMS += \
        mainwindow.ui \
    about.ui \
    connectionwidget.ui \
    mqttwidget.ui \
    messageviewer.ui \
    license.ui

RESOURCES += \
    res.qrc

TRANSLATIONS += zh_cn.ts\
                en_us.ts

win32:CONFIG(release, debug|release): LIBS += -lQt5Mqtt
else:win32:CONFIG(debug, debug|release): LIBS += -lQt5Mqttd
else:unix: LIBS += -lQt5Mqtt

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lqmqtt
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lqmqttd
#else:unix: LIBS += -L$$PWD/lib/ -lqmqtt

#INCLUDEPATH += $$PWD/include
#DEPENDPATH += $$PWD/include
