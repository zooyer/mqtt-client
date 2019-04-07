#-------------------------------------------------
#
# Project created by QtCreator 2019-04-03T22:10:19
#
#-------------------------------------------------

QT       += core gui network qmqtt

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
        mainwidget.cpp \
    connectionbutton.cpp \
    connectionwidget.cpp \
    mqttsdkwidget.cpp \
    mqttsdk.cpp \
    lineedit.cpp \
    messageviewer.cpp

HEADERS += \
        mainwidget.h \
    connectionbutton.h \
    connectionwidget.h \
    mqttsdkwidget.h \
    mqttsdk.h \
    lineedit.h \
    messageviewer.h

FORMS += \
        mainwidget.ui \
    connectionwidget.ui \
    mqttsdkwidget.ui \
    messageviewer.ui

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lQt5Mqtt -lpaho-mqttpp3-static
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lQt5Mqttd -lpaho-mqttpp3-static
#else:unix: LIBS += -L$$PWD/lib/ -lQt5Mqtt

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lpaho-mqttpp3-static -lpaho-mqtt3as -lwsock32 -llibeay32 -lssleay32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lpaho-mqttpp3-static -lpaho-mqtt3as -lwsock32 -llibeay32 -lssleay32
else:unix: LIBS += -L$$PWD/lib/ -lpaho-mqttpp3-static

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
