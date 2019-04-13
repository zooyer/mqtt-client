#ifndef LOGGER_H
#define LOGGER_H

#include <QMutex>
#include <QObject>
#include <QSharedPointer>

class Logger : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<Logger> getInstance();
    QtMessageHandler handler();

private:
    explicit Logger(QObject *parent = nullptr);
    Logger(const Logger&);
    Logger& operator=(const Logger&);

signals:

public slots:

private:
    static QMutex m_mutex;
    static QSharedPointer<Logger> m_pInstance;
    static QString m_timePoint;
};

#endif // LOGGER_H
