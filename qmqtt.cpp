#include "qmqtt.h"
#include "qmqtt/qmqtt_message.h"

QMqtt::QMqtt(QObject *parent) : AbstractMqtt(parent)
{
    m_client = new QMQTT::Client;
    m_sslConfig = new QSslConfiguration;
    m_sslEnable = false;

    connect(m_client, &QMQTT::Client::connected, this, &QMqtt::connected);
    connect(m_client, &QMQTT::Client::disconnected, this, &QMqtt::disconnected);
    connect(m_client, &QMQTT::Client::received, this, [this](const QMQTT::Message& message){
        emit this->received(message.topic(), message.qos(), message.payload(), message.retain());
    });
    connect(m_client, &QMQTT::Client::published, this, [this](const QMQTT::Message& message, quint16 msgid = 0){
        Q_UNUSED(msgid);
        emit this->published(message.topic(), message.qos(), message.payload(), message.retain());
    });
    connect(m_client, &QMQTT::Client::subscribed, this, &QMqtt::subscribed);
    connect(m_client, &QMQTT::Client::unsubscribed, this, &QMqtt::unsubscribed);
    connect(m_client, &QMQTT::Client::error, this, [this](const QMQTT::ClientError error){
        emit this->errorChanged(QString("%1").arg(error));
    });
}

QMqtt::~QMqtt()
{
    delete m_client;
    delete m_sslConfig;
}

void QMqtt::setBroker(const QString &broker)
{
    QString hostname;
    QString url = broker;
    QString protocol;
    quint16 port;
    if (!broker.contains("//")) {
        url = "tcp://" + broker;
    }
    QStringList uris = url.split(":");
    if (uris.length() != 3) {
        emit errorChanged("set broker format error");
        return;
    }
    protocol = uris[0];
    hostname = uris[1].replace("//", "");
    port = uris[2].toUShort();
    if (protocol == "tcp") {
        m_sslEnable = false;
    } else if (protocol == "ssl") {
        m_sslEnable = true;
    } else {
        emit errorChanged(QString("unknown protocol:") + protocol);
        return;
    }
    m_client->setHostName(hostname);
    m_client->setPort(port);
}

void QMqtt::setSslConfiguration(const QSslConfiguration &ssl)
{
    *m_sslConfig = ssl;
}

void QMqtt::setCleanSession(const bool clean)
{
    m_client->setCleanSession(clean);
}

void QMqtt::setClientId(const QString &id)
{
    m_client->setClientId(id);
}

void QMqtt::setKeepAlive(const quint16 second)
{
    m_client->setKeepAlive(second);
}

void QMqtt::setUsername(const QString &username)
{
    m_client->setUsername(username);
}

void QMqtt::setPassword(const QString &password)
{
    m_client->setPassword(password.toLocal8Bit());
}

void QMqtt::setWillTopic(const QString &topic)
{
    m_client->setWillTopic(topic);
}

void QMqtt::setWillQos(const quint8 qos)
{
    m_client->setWillQos(qos);
}

void QMqtt::setWillRetain(const bool retain)
{
    m_client->setWillRetain(retain);
}

void QMqtt::setWillMessage(const QByteArray &msg)
{
    m_client->setWillMessage(msg);
}

void QMqtt::connectToHost()
{
    QMQTT::Client *newClient = nullptr;
    if (m_sslEnable) {
        newClient = new QMQTT::Client("localhost", 1883, *m_sslConfig);
    } else {
        newClient = new QMQTT::Client;
    }
    if (newClient != nullptr) {
        newClient->setHostName(m_client->hostName());
        newClient->setPort(m_client->port());
        newClient->setCleanSession(m_client->cleanSession());
        newClient->setClientId(m_client->clientId());
        newClient->setKeepAlive(m_client->keepAlive());
        newClient->setUsername(m_client->username());
        newClient->setPassword(m_client->password());
        newClient->setWillTopic(m_client->willTopic());
        newClient->setWillQos(m_client->willQos());
        newClient->setWillRetain(m_client->willRetain());
        newClient->setWillMessage(m_client->willMessage());

        delete m_client;
        m_client = newClient;
    }

    m_client->connectToHost();
}

void QMqtt::disconnectFromHost()
{
    m_client->disconnectFromHost();
}

void QMqtt::publish(const QString &topic, const quint8 qos, const QByteArray &payload, const bool retain)
{
    m_client->publish(QMQTT::Message(0, topic, payload, qos, retain));
}

void QMqtt::subscribe(const QString &topic, const quint8 qos)
{
    m_client->subscribe(topic, qos);
}

void QMqtt::unsubscribe(const QString &topic)
{
    m_client->unsubscribe(topic);
}

AbstractMqtt::MqttStatus QMqtt::status()
{
    switch (m_client->connectionState()) {
    case QMQTT::STATE_INIT:
    case QMQTT::STATE_DISCONNECTED:
        return Disconnected;
    case QMQTT::STATE_CONNECTING:
        return Connecting;
    case QMQTT::STATE_CONNECTED:
        return Connected;
    }
    return Disconnected;
}
