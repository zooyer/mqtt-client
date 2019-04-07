#include "mainwidget.h"
#include <QApplication>
#include "mqttsdk.h"
#include <QDebug>

#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>
#include <cstdlib>
#include <iostream>
using namespace std;

//#include <qmqtt.h>

void test();

QMutex m; //日志代码互斥锁
QString timePoint;

//日志生成
void LogMsgOutput(QtMsgType type,
    const QMessageLogContext& context,
    const QString& msg)
{
    // 持有锁
    m.lock();

    cout << msg.toStdString() << endl;

    // Critical Resource of Code
    QByteArray localMsg = msg.toLocal8Bit();
    QString log;

    switch (type) {
    case QtDebugMsg:
        // fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(),
        // context.file, context.line, context.function);
        log.append(QString("Debug :%1 %2  Line:%3  Content:%4")
                       .arg(context.file)
                       .arg(context.function)
                       .arg(context.line)
                       .arg(msg));
        break;
    case QtInfoMsg:
        // fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(),
        // context.file, context.line, context.function);
        log.append(QString("Info: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        break;
    case QtWarningMsg:
        // fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(),
        // context.file, context.line, context.function);
        log.append(QString("Warning: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        break;
    case QtCriticalMsg:
        // fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(),
        // context.file, context.line, context.function);
        log.append(QString("Critical: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        break;
    case QtFatalMsg:
        // fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(),
        // context.file, context.line, context.function);
        log.append(QString("Fatal: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        abort();
    }

    QFile file;
    QString path = QString("mqtt-client-%1.log").arg(timePoint);
    file.setFileName(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Append)) {
        QString erinfo = file.errorString();
        cout << erinfo.toStdString() << endl;
        return;
    }
    QTextStream out(&file);
    out << "\n\r" << log;
    file.close();

    // 释放锁
    m.unlock();
}

int main(int argc, char *argv[])
{
    // release模式下，调试信息输出至日志文件
#ifndef _DEBUG
    timePoint = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    qInstallMessageHandler(LogMsgOutput);
#endif

#ifdef QT_NO_SSL
     qDebug() << "ssl: not support";
#else
    qDebug() << "ssl: support";
#endif

    QApplication a(argc, argv);
    MainWidget w;
    w.show();
    a.setFont(QFont("Tahoma", 8));

    //QMQTT::Client *client = new QMQTT::Client();

    test();


    return a.exec();
}
