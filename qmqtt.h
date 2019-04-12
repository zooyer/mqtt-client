#ifndef QMQTT_H
#define QMQTT_H

#include <QSslConfiguration>
#include "abstractmqtt.h"
#include "qmqtt/qmqtt_client.h"

class QMqtt : public AbstractMqtt
{
    Q_OBJECT
public:
    explicit QMqtt(QObject *parent = nullptr);
    ~QMqtt();

    void setBroker(const QString &broker);
    void setSslConfiguration(const QSslConfiguration &ssl);
    void setCleanSession(const bool clean);
    void setClientId(const QString &id);
    void setKeepAlive(const quint16 second);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setWillTopic(const QString &topic);
    void setWillQos(const quint8 qos);
    void setWillRetain(const bool retain);
    void setWillMessage(const QByteArray &msg);

    void connectToHost();
    void disconnectFromHost();
    void publish(const QString &topic, const quint8 qos, const QByteArray &payload, const bool retain);
    void subscribe(const QString &topic, const quint8 qos);
    void unsubscribe(const QString &topic);
    MqttStatus status();

public slots:

private:
     QMQTT::Client *m_client;
     QSslConfiguration *m_sslConfig;
     bool m_sslEnable;
};

#endif // QMQTT_H
