#ifndef MQTTSDK_H
#define MQTTSDK_H

#include <QFile>
#include <QObject>
#include <QDebug>
#include <QException>
//#include <QtMqtt/QMqttClient>
#include <qmqtt>
#include <QMap>

class MqttRequest;
class MqttResponse;


typedef void(*CbReceive)(QString id, QString channel, const QByteArray &message);
typedef void(*CbBroadcast)(QString topic, const QMQTT::Message &message);

struct sub
{
    QString topic;
    quint8 qos;
    CbBroadcast callback;
};

class MqttSDKException : public QException
{
private:
    QString m_exception;
public:
    MqttSDKException(QString exception)
    {
        m_exception = exception;
    }
    const char* what() const throw()
    {
        return m_exception.toLatin1().data();
    }
};

struct MqttSDKParams
{
    QString server;
    QString deviceSK;
    QString deviceID;
};


class MqttSDK : public QObject
{
    Q_OBJECT
public:
    explicit MqttSDK(QObject *parent = nullptr);
    explicit MqttSDK(MqttSDKParams params, QObject *parent = nullptr);
    void setParams(MqttSDKParams params);
    bool init();
    bool uninit();
    bool isConnected();
    bool sendMessage(QString id, quint8 qos, QString channel, const QByteArray &message);
    bool receiveMessage(QString id, quint8 qos, QString channel, CbReceive callback);
    bool unReceiveMessage(QString id, QString channel);
    bool broadcast(QString topic, quint8 qos, bool betain, const QByteArray &message);
    bool receiveBroadcast(QString topic, quint8 qos, CbBroadcast callback);
    bool unReceiveBroadcast(QString topic);

private:
    MqttResponse doRegister(MqttRequest request) throw (MqttSDKException);
    void initMqttClient();
    QString encodeTopic(QString channel);
    QString decodeTopic(QString channel);

signals:
    void connected();
    void disconnected();
    void errored(const QMQTT::ClientError &error);
    void received(const QMQTT::Message& message);

public slots:

private:
    //QMqttClient *m_client;
    QMQTT::Client *m_client;
    QSslConfiguration *m_ssl;
    MqttSDKParams m_params;
    MqttRequest   *m_request;
    MqttResponse  *m_response;

    QMap<QString,sub> m_subs;
};

#endif // MQTTSDK_H
