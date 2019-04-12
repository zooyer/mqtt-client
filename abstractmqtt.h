#ifndef ABSTRACTMQTT_H
#define ABSTRACTMQTT_H

#include <QSslConfiguration>
#include <QObject>

class AbstractMqtt : public QObject
{
public:
    enum MqttStatus
    {
        Disconnected = 0,
        Connecting,
        Connected
    };

    Q_OBJECT
public:
    explicit AbstractMqtt(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AbstractMqtt(){}

    virtual void setBroker(const QString &broker) = 0;
    virtual void setSslConfiguration(const QSslConfiguration &ssl) = 0;
    virtual void setCleanSession(const bool clean) = 0;
    virtual void setClientId(const QString &id) = 0;
    virtual void setKeepAlive(const quint16 second) = 0;
    virtual void setUsername(const QString &username) = 0;
    virtual void setPassword(const QString &password) = 0;
    virtual void setWillTopic(const QString &topic) = 0;
    virtual void setWillQos(const quint8 qos) = 0;
    virtual void setWillRetain(const bool retain) = 0;
    virtual void setWillMessage(const QByteArray &msg) = 0;

    virtual void connectToHost() = 0;
    virtual void disconnectFromHost() = 0;
    virtual void publish(const QString &topic, const quint8 qos, const QByteArray &payload, const bool retain) = 0;
    virtual void subscribe(const QString &topic, const quint8 qos) = 0;
    virtual void unsubscribe(const QString &topic) = 0;
    virtual MqttStatus status() = 0;


signals:
    void connected();
    void disconnected();
    void errorChanged(const QString &error);
    void statusChanged(const MqttStatus &status);
    void subscribed(const QString &topic, const quint8 qos);
    void unsubscribed(const QString &topic);
    void received(const QString &topic, const quint8 qos, const QByteArray &payload, const bool retain);
    void published(const QString &topic, const quint8 qos, const QByteArray &payload, const bool retain);

public slots:
};

#endif // ABSTRACTMQTT_H
