#ifndef MQTTEXCEPTION_H
#define MQTTEXCEPTION_H

#include <QException>

class MqttException : public QException
{
public:
    explicit MqttException(QString exception);
    QString errorString() const;
    const char* what() const noexcept;

private:
    QString m_exception;
};

#endif // MQTTEXCEPTION_H
