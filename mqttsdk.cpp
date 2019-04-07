#include "mqttsdk.h"
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSsl>

struct MqttRequest
{
    QString deviceSK;
    QString deviceID;
    QString deviceDesc;

    MqttRequest()
    {

    }

    void fromParams(MqttSDKParams &params) throw(MqttSDKException)
    {
        qDebug() << "device sk:" << params.deviceSK;
        QFile file(params.deviceSK);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            throw MqttSDKException(file.errorString());
        deviceSK = QString(file.readAll());
        deviceID = params.deviceID;
        deviceDesc = "";
        file.close();
    }

    QByteArray toJson() {
        QJsonObject obj;
        obj.insert("device_sk", deviceSK);
        obj.insert("device_id", deviceID);
        obj.insert("device_desc", deviceDesc);

        QJsonDocument doc(obj);

        return doc.toJson();
    }
};

class MqttResponse {
public:
    // status code and status msg.
    int     code;
    QString message;

    int     companyID;
    QString clientID;
    QString broker;
    int     port;
    QString deviceID;
    QString secretKey;
    int     keepAlive;
    bool    cleanSession;
    qint64  timestamp;

    MqttResponse()
    {

    }

    void fromJson(QByteArray &data) throw(MqttSDKException) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError)
            throw MqttSDKException(err.errorString());

        if (!doc.isObject())
            throw MqttSDKException("json not is object.");

        QJsonObject obj = doc.object();
        code = obj["code"].toInt();
        message = obj["message"].toString();
        companyID = obj["company_id"].toInt();
        clientID = obj["client_id"].toString();
        broker = obj["broker"].toString();
        port = obj["port"].toInt();
        deviceID = obj["device_id"].toString();
        secretKey = obj["secret_key"].toString();
        keepAlive = obj["keep_alive"].toInt();
        cleanSession = obj["clean_session"].toBool();
        timestamp = obj["timestamp"].toVariant().toLongLong();
    }
};

int post(const QByteArray &msg, QString method, QString url, QByteArray &response) throw(MqttSDKException)
{
    QUrl Url = QUrl(url);
    QNetworkRequest request(Url);
    QNetworkAccessManager manager;
    QNetworkReply *reply = nullptr;

    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    if (method.toUpper() == "POST")
        reply = manager.post(request, msg);
    else
        reply = manager.get(request);

    qDebug() << "port msg:" << msg.data();
    qDebug() << "port url:" << url;

    QEventLoop loop;
    QTimer timer;

    QObject::connect(&manager, &QNetworkAccessManager::finished, [&loop](){
        loop.quit();
    });
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);


    reply->deleteLater();
    timer.start(2000);
    loop.exec();
    qDebug() << "loop exit...";

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "server response error:" << reply->errorString();
        throw MqttSDKException(reply->errorString());
    }

    response = reply->readAll();
    qDebug() << response;

    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

MqttSDK::MqttSDK(QObject *parent) : QObject(parent)
{
    this->m_client = new QMQTT::Client;
    this->m_ssl = new QSslConfiguration;
    this->m_request = new MqttRequest;
    this->m_response = new MqttResponse;
}

MqttSDK::MqttSDK(MqttSDKParams params, QObject *parent) : MqttSDK(parent)
{
    this->m_params = params;
}

void MqttSDK::setParams(MqttSDKParams params)
{
    m_params = params;
}

void MqttSDK::initMqttClient()
{
    qDebug() << "in init mqtt client...";
    if (m_client != nullptr)
        delete m_client;

    if (m_response->broker.contains("ssl://")) {

        QList<QSslCertificate> certs = QSslCertificate::fromPath("server.crt");
        qDebug() << "certs count:" << certs.length();
        if (certs.length() > 0)
            qDebug() << "certs[0] : " << certs[0].toPem();
        m_ssl->setCaCertificates(certs);
//            QSslCertificate cert(data, QSsl::Pem);
//            QList<QSslCertificate> list;
//            list.append(cert);
//            m_ssl->setLocalCertificate(cert);
        //m_ssl->setCaCertificates(list);
        m_ssl->setPeerVerifyMode(QSslSocket::VerifyNone);
        //m_ssl->setLocalCertificateChain(list);
        //m_ssl->setCaCertificates(cert);

        m_ssl->setProtocol(QSsl::AnyProtocol);

        m_client = new QMQTT::Client("localhost", 1883, *m_ssl);

    } else {
        m_client = new QMQTT::Client;
    }

    m_client->setCleanSession(m_response->cleanSession);
    m_client->setClientId(m_response->clientID);
    m_client->setKeepAlive((quint16)m_response->keepAlive);
    m_client->setHostName(m_response->broker.replace("tcp://", "").replace("ssl://", ""));
    m_client->setPort((quint16)m_response->port);
    m_client->setUsername(m_response->deviceID);
    m_client->setPassword(m_response->secretKey.toLocal8Bit());


    qDebug() << "set mqtt client options finish.";

    connect(m_client, &QMQTT::Client::error, [this](const QMQTT::ClientError error){
        qDebug() << "QMQTT Error:" << error;
        emit errored(error);
    });

    connect(m_client, &QMQTT::Client::connected, [this](){
        qDebug() << "on connected!";
        emit connected();
        auto list = m_subs.keys();
        if (m_subs.size() > 0) {
            foreach (auto key, list) {
                m_client->subscribe(key, m_subs[key].qos);
            }
        }
    });

    connect(m_client, &QMQTT::Client::disconnected, [this](){
        qDebug() << "on disconnected!";
        emit disconnected();
    });

    connect(m_client, &QMQTT::Client::received, [this](const QMQTT::Message& message) {
        qDebug() << "on received!";
        emit received(message);
        if (m_subs.contains(message.topic())) {
            m_subs[message.topic()].callback(message.topic(), message);
            return;
        }

        qDebug() << "TOPIC:" << message.topic();
        qDebug() << "MSG:" << QString(message.payload());
    });

    m_client->connectToHost();
}

QString MqttSDK::encodeTopic(QString channel)
{
    QByteArray encode = channel.toLocal8Bit().toBase64();
    encode = encode.replace("+", "-");
    encode = encode.replace("/", "_");

    return encode;
}

QString MqttSDK::decodeTopic(QString channel)
{
    QByteArray decode = channel.toLocal8Bit();
    decode = decode.replace("_", "/");
    decode = decode.replace("-", "+");
    decode = QByteArray::fromBase64(decode);

    return decode;
}

bool MqttSDK::init()
{
    MqttRequest req;
    MqttResponse res;

    try {
        req.fromParams(m_params);
        res = doRegister(req);
        if (res.message != "" && res.code != 0) {
            qDebug() << QString("register code:%1, message:%2").arg(res.code).arg(res.message);
            return false;
        }
        *m_request = req;
        *m_response = res;

        initMqttClient();

        return true;
    } catch (QException &e) {
        qDebug() << "init eror:" << e.what();
    }

    return false;
}

bool MqttSDK::uninit()
{
    if (m_client->isConnectedToHost())
        m_client->disconnectFromHost();

    return true;
}

bool MqttSDK::isConnected()
{
    return m_client->isConnectedToHost();
}

bool MqttSDK::sendMessage(QString id, quint8 qos, QString channel, const QByteArray &message)
{
    return false;
}

bool MqttSDK::receiveMessage(QString id, quint8 qos, QString channel, CbReceive callback)
{
    return false;
}

bool MqttSDK::unReceiveMessage(QString id, QString channel)
{

    return false;
}

bool MqttSDK::broadcast(QString topic, quint8 qos, bool betain, const QByteArray &message)
{
    QMQTT::Message msg;
    msg.setTopic(topic);
    msg.setQos(qos);
    msg.setPayload(message);
    msg.setRetain(betain);

    return m_client->publish(msg) == 0;
}

bool MqttSDK::receiveBroadcast(QString topic, quint8 qos, CbBroadcast callback)
{
    m_client->subscribe(topic, qos);
    if (callback != nullptr) {
        sub s;
        s.topic = topic;
        s.qos = qos;
        s.callback = callback;
        m_subs[topic] = s;
    }

    return true;
}

bool MqttSDK::unReceiveBroadcast(QString topic)
{
    if (!m_subs.contains(topic))
        return false;

    m_subs.remove(topic);

    return true;
}

MqttResponse MqttSDK::doRegister(MqttRequest request) throw(MqttSDKException)
{
    qDebug() << "request:" << request.toJson().data();
    QString url = m_params.server;
    if (!url.contains("http")) {
        url = "http://" + url;
    }
    QStringList urls = url.split("//");
    if (urls.length() < 2)
        throw MqttSDKException(QString("invalid url:") + url);
    if (!urls.at(1).contains("/"))
        url += "/device/register2.url";

    qDebug() << "url:" << url;

    QByteArray buf;
    try {
        int ret = post(request.toJson(), "POST", url, buf);
        if (ret != 200)
            throw MqttSDKException(QString(buf));
    } catch (MqttSDKException &e) {
        throw e;
    }

    qDebug() << "response：" << buf;
    MqttResponse response;
    response.fromJson(buf);
    qDebug() << response.broker;
    qDebug() << response.companyID;

    return response;
}

void test()
{
    MqttSDKParams params;
    params.deviceID = "id-0001";
    params.deviceSK = "C:\\Users\\Administrator\\Desktop\\demo.txt";
    params.server = "127.0.0.1";
    MqttRequest req;
    try {
        req.fromParams(params);
        qDebug() << req.toJson().data();
    } catch (QException &e) {
        qDebug() << "new mqtt request error:" << e.what();
    }

    QString js = "{\"company_id\":12,\"client_id\":\"uuid-0102\",\"broker\":\"127.0.0.1\",\"port\":9999,\"device_id\":\"devid\",\"secret_key\":\"key...\",\"keep_alive\":30,\"clean_session\":true,\"timestamp\":15023053342}";

    MqttResponse res;
    try {
        QByteArray bytes = js.toLatin1();
        res.fromJson(bytes);
        qDebug() << res.code << res.message;
        qDebug() << res.companyID << res.clientID << res.broker << res.port << res.deviceID << res.secretKey  << res.keepAlive << res.cleanSession << res.timestamp;
    } catch (QException &e) {
        qDebug() << "response error:" << e.what();
    }


}
