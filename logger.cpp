#include "logger.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <cstdlib>
#include <iostream>
using namespace std;

QSharedPointer<Logger> Logger::getInstance()
{
    if (m_pInstance.isNull()) {
        QMutexLocker _(&m_mutex);
        if (m_pInstance.isNull()) {
            m_pInstance = QSharedPointer<Logger>(new Logger);
        }
    }

    return m_pInstance;
}

QtMessageHandler Logger::handler()
{
    return static_cast<QtMessageHandler>([](QtMsgType type, const QMessageLogContext &context, const QString &msg){
        m_mutex.lock();

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

        QFile logFile(QString("%1.log").arg(m_timePoint));
        if (!logFile.open(QIODevice::ReadWrite | QIODevice::Append)) {
            cout << logFile.errorString().toStdString() << endl;
            m_mutex.unlock();
            return;
        }

        logFile.write(log.toLocal8Bit());
        logFile.write("\r\n");
        logFile.close();

        m_mutex.unlock();
    });
}

QMutex Logger::m_mutex;
QSharedPointer<Logger> Logger::m_pInstance;
QString Logger::m_timePoint;

Logger::Logger(QObject *parent) : QObject(parent)
{
    m_timePoint = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
}

Logger::Logger(const Logger &)
{

}

Logger &Logger::operator=(const Logger &)
{
    return *this;
}
