#include "qtmqtt.h"

QtMqtt::QtMqtt(QObject *parent) : AbstractMqtt(parent)
{
    m_client = new QMqttClient(this);
    m_sslSocket = new QSslSocket(this);
    m_sslConfig = new QSslConfiguration;
    m_sslEnable = false;

    connect(m_client, &QMqttClient::connected, this, &QtMqtt::connected);
    connect(m_client, &QMqttClient::disconnected, this, &QtMqtt::disconnected);
    connect(m_client, &QMqttClient::messageReceived, [this](const QByteArray &message, const QMqttTopicName &topic){
        emit received(topic.name(), 0, message, false);
    });
    connect(m_client, &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state){
        MqttStatus status = Disconnected;
        switch (state) {
        case QMqttClient::Disconnected:
            status = Disconnected;
            break;
        case QMqttClient::Connecting:
            status = Connecting;
            break;
        case QMqttClient::Connected:
            status = Connected;
            break;
        }
        emit this->statusChanged(status);
    });
    connect(m_client, &QMqttClient::errorChanged, this, [this](QMqttClient::ClientError error){
        emit this->errorChanged(QString("%1").arg(error));
    });
}

QtMqtt::~QtMqtt()
{
    delete m_client;
    delete m_sslConfig;
}

void QtMqtt::setBroker(const QString &broker)
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
    m_client->setHostname(hostname);
    m_client->setPort(port);
}

void QtMqtt::setSslConfiguration(const QSslConfiguration &ssl)
{
    *m_sslConfig = ssl;
    m_sslSocket->setSslConfiguration(*m_sslConfig);
}

void QtMqtt::setCleanSession(const bool clean)
{
    m_client->setCleanSession(clean);
}

void QtMqtt::setClientId(const QString &id)
{
    m_client->setClientId(id);
}

void QtMqtt::setKeepAlive(const quint16 second)
{
    m_client->setKeepAlive(second);
}

void QtMqtt::setUsername(const QString &username)
{
    m_client->setUsername(username);
}

void QtMqtt::setPassword(const QString &password)
{
    m_client->setPassword(password);
}

void QtMqtt::setWillTopic(const QString &topic)
{
    m_client->setWillTopic(topic);
}

void QtMqtt::setWillQos(const quint8 qos)
{
    m_client->setWillQoS(qos);
}

void QtMqtt::setWillRetain(const bool retain)
{
    m_client->setWillRetain(retain);
}

void QtMqtt::setWillMessage(const QByteArray &msg)
{
    m_client->setWillMessage(msg);
}

void QtMqtt::connectToHost()
{
    if (m_sslEnable) {
        m_client->setTransport(m_sslSocket, QMqttClient::AbstractSocket);
        m_client->connectToHostEncrypted();
    } else {
        m_client->connectToHost();
    }
}

void QtMqtt::disconnectFromHost()
{
    m_client->disconnectFromHost();
}

void QtMqtt::publish(const QString &topic, const quint8 qos, const QByteArray &payload, const bool retain)
{
    m_client->publish(topic, payload, qos, retain);
    emit published(topic, qos, payload, retain);
}

void QtMqtt::subscribe(const QString &topic, const quint8 qos)
{
    auto subscription = m_client->subscribe(topic, qos);
    if (subscription) {
        emit subscribed(topic, qos);
    }
}

void QtMqtt::unsubscribe(const QString &topic)
{
    m_client->unsubscribe(topic);
    emit unsubscribed(topic);
}

AbstractMqtt::MqttStatus QtMqtt::status()
{
    switch (m_client->state()) {
    case QMqttClient::Disconnected:
        return Disconnected;
    case QMqttClient::Connecting:
        return Connecting;
    case QMqttClient::Connected:
        return Connected;
    }

    return Disconnected;
}
