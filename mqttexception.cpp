#include "mqttexception.h"

MqttException::MqttException(QString exception)
{
    m_exception = exception;
}

QString MqttException::errorString() const
{
    return m_exception;
}

const char *MqttException::what() const noexcept
{
    return m_exception.toLocal8Bit().data();
}
