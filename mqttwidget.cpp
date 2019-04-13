#include "mqttwidget.h"
#include "ui_mqttwidget.h"
#include "tabledelegate.h"
#include "messageviewer.h"
//#include "qmqtt.h"
//#include "qtmqtt.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QSslSocket>
#include <QSslError>
#include <QSslKey>
#include <QSslConfiguration>
#include <QAbstractSocket>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>

class MqttRequest
{
public:
    QString deviceSK;
    QString deviceID;
    QString deviceDesc;

    void fromParams(MqttParams &params) noexcept(false)
    {
        qDebug() << "device sk:" << params.deviceSK;
        QFile file(params.deviceSK);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            throw MqttException(file.errorString());
        deviceSK = QString(file.readAll());
        deviceID = params.deviceID;
        deviceDesc = "";
        file.close();
    }

    QByteArray toJson()
    {
        QJsonObject obj;
        obj.insert("device_sk", deviceSK);
        obj.insert("device_id", deviceID);
        obj.insert("device_desc", deviceDesc);

        QJsonDocument doc(obj);

        return doc.toJson();
    }
};

class MqttResponse
{
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

    void fromJson(QByteArray &data) noexcept(false) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError)
            throw MqttException(err.errorString());

        if (!doc.isObject())
            throw MqttException("json not is object.");

        QJsonObject obj = doc.object();
        code         = obj["code"].toInt();
        message      = obj["message"].toString();
        companyID    = obj["company_id"].toInt();
        clientID     = obj["client_id"].toString();
        broker       = obj["broker"].toString();
        port         = obj["port"].toInt();
        deviceID     = obj["device_id"].toString();
        secretKey    = obj["secret_key"].toString();
        keepAlive    = obj["keep_alive"].toInt();
        cleanSession = obj["clean_session"].toBool();
        timestamp    = obj["timestamp"].toVariant().toLongLong();
    }
};

int post(const QByteArray &msg, QString method, QString url, QByteArray &response) noexcept(false)
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
    bool timeout = false;

    QObject::connect(&manager, &QNetworkAccessManager::finished, [&loop](){
        loop.quit();
    });
    QObject::connect(&timer, &QTimer::timeout, [&loop, &timeout](){
        timeout = true;
        loop.quit();
    });

    reply->deleteLater();
    timer.start(2000);
    loop.exec();
    qDebug() << "wait http response loop exit...";
    if (timeout) {
        throw MqttException("http post request timeout");
    }

    response = reply->readAll();
    qDebug() << "response data:" << response;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "response code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "server response error:" << reply->errorString();
        throw MqttException(response.length() == 0 ? reply->errorString() : response);
    }

    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

MqttWidget::MqttWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MqttWidget)
{
    ui->setupUi(this);

    // init mqtt
    m_params = new MqttParams;
    m_request = new MqttRequest;
    m_response = new MqttResponse;
    m_client = new QMqttClient;
    m_ssl = new QSslSocket;
    //m_client = new QMqtt(this);

    // connection button init.
    m_menu = new QMenu(ui->historyTable);
    m_clear = new QAction(QIcon(":/image/clear.gif"), tr("Clear"), m_menu);
    m_menu->addAction(m_clear);

    // set history table style.
    ui->historyTable->verticalHeader()->setHidden(true);
    ui->historyTable->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui->historyTable->horizontalHeader()->setStretchLastSection(true);
    ui->historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->historyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->historyTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->historyTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    ui->historyTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    ui->historyTable->setColumnWidth(0, 100);
    ui->historyTable->setColumnWidth(3, 80);
    ui->historyTable->setColumnWidth(4, 80);
    ui->historyTable->setColumnWidth(5, 200);


    // set topics table view model.
    m_model = new TopicModel(0, 3);

    // set topics table view model and style.
    ui->topics->setModel(m_model);
    ui->topics->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->topics->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->topics->setColumnWidth(0, 25);
    ui->topics->verticalHeader()->setDefaultSectionSize(25);
    //ui->topics->setAlternatingRowColors(true);
    //ui->topics->setStyleSheet("QTableView{background-color: rgb(250, 250, 115); alternate-background-color: rgb(141, 163, 215);}");
    ui->topics->setWindowTitle(tr("topics"));
    ui->topics->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // set topics table header.
    QStringList headerList;
    headerList << "" << tr("topic") << tr("qos");
    m_model->setHorizontalHeaderLabels(headerList);
    ui->topics->verticalHeader()->setVisible(false);
    ui->topics->horizontalHeader()->setStretchLastSection(true);

    // set topics table delegate combo box.
    QosDelegate *qosDelegate = new QosDelegate(ui->topics);
    ui->topics->setItemDelegateForColumn(2, qosDelegate);

    // init  mqtt events.
    initMqttEvents();

    // init signal connect.
    initConnect();

    // init status.
    initStatus();
}

MqttWidget::~MqttWidget()
{
    if (m_clear != nullptr)
        delete m_clear;
    if (m_menu != nullptr)
        delete m_menu;
    if (m_model != nullptr)
        delete m_model;
    if (m_client != nullptr) {
        m_client->disconnectFromHost();
        delete m_client;
    }

    if (m_params != nullptr)
        delete m_params;
    if (m_request != nullptr)
        delete m_request;
    if (m_response != nullptr)
        delete m_response;

    delete ui;
}

void MqttWidget::initConnect()
{
    connect(ui->skBrowse, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.sk;;*.*"), nullptr);
        if (fileName.length() > 0)
            ui->sk->setText(fileName);
    });
    connect(ui->connect, &QPushButton::clicked, [this](){
        if (m_client->state() == QMqttClient::Connected) {
            return;
        }

        m_params->server = ui->uri->text();
        m_params->deviceID = ui->id->text();
        m_params->deviceSK = ui->sk->text();

        try {
            m_request->fromParams(*m_params);
            *m_response = doRegister(*m_request);
        } catch (MqttException &e) {
            qWarning() << "register error:" << e.errorString();
            addHistory(tr("Error"), "", e.errorString(), "", "");
            return;
        }

        QString hostname = m_response->broker;

        m_client->setHostname(hostname.replace("tcp://", "").replace("ssl://", ""));
        m_client->setPort(static_cast<quint16>(m_response->port));
        m_client->setCleanSession(m_response->cleanSession);
        m_client->setClientId(m_response->clientID);
        m_client->setKeepAlive(static_cast<quint16>(m_response->keepAlive));
        m_client->setUsername(m_response->deviceID);
        m_client->setPassword(m_response->secretKey.toLocal8Bit());

        if (m_response->broker.contains("ssl")) {

            QSslConfiguration sslConfig = QSslConfiguration::defaultDtlsConfiguration();
            auto certs = QSslCertificate::fromPath("server.crt");
            sslConfig.setCaCertificates(certs);
            sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
            sslConfig.setProtocol(QSsl::AnyProtocol);

            m_ssl->setSslConfiguration(sslConfig);
            m_client->setTransport(m_ssl, QMqttClient::SecureSocket);

            m_ssl->connectToHostEncrypted(m_client->hostname(), m_client->port());
            if (!m_ssl->waitForEncrypted(5000)) {
                qDebug() << "wait tls connect false.";
                addHistory(tr("Error"), "", "tls connection timeout", "", "");
            }

            qDebug() << "setting ssl configuration.";
        }

        m_client->connectToHost();
        qDebug() << "connect to host:" << m_client->hostname() << "port:" << m_client->port();
    });
    connect(ui->disconnect, &QPushButton::clicked, [this](){
        if (m_client->state() != QMqttClient::Disconnected) {
            m_client->disconnectFromHost();
        }
    });
    connect(ui->subAdd, &QPushButton::clicked, [this](){
        qDebug() << "clicked sub add button.";
        int rowIndex = m_model->rowCount();
        m_model->insertRow(rowIndex);

        QModelIndex index0 = m_model->index(rowIndex, 0, QModelIndex());
        QModelIndex index1 = m_model->index(rowIndex, 1, QModelIndex());
        QModelIndex index2 = m_model->index(rowIndex, 2, QModelIndex());

        m_model->setData(index0, Qt::Checked, Qt::CheckStateRole);
        m_model->setData(index1, "test", Qt::EditRole);
        m_model->setData(index2, "0", Qt::EditRole);

        if (!ui->subDelete->isEnabled()) {
            ui->subDelete->setEnabled(true);
        }
        if (!ui->subClean->isEnabled()) {
            ui->subClean->setEnabled(true);
        }
    });
    connect(ui->subDelete, &QPushButton::clicked, [this](){
        qDebug() << "clicked sub del button.";
        QModelIndex index = ui->topics->currentIndex();
        if (index.isValid()) {
            m_model->removeRow(index.row());
            if (m_model->rowCount() == 0) {
                ui->subDelete->setEnabled(false);
                ui->subClean->setEnabled(false);
            }
        }
    });
    connect(ui->subClean, &QPushButton::clicked, [this](){
        qDebug() << "clicked sub clean.";
        while (m_model->rowCount() != 0) {
            m_model->removeRow(0);
        }
        ui->subDelete->setEnabled(false);
        ui->subClean->setEnabled(false);
    });
    connect(ui->subscribe, &QPushButton::clicked, [this](){
        if (m_client->state() == QMqttClient::Connected) {
            for (int i=0; i<m_model->rowCount(); i++) {
                if (m_model->item(i)->checkState() == Qt::Checked) {
                    QString topic = m_model->item(i, 1)->data(Qt::EditRole).toString();
                    quint8 qos = static_cast<quint8>(m_model->item(i, 2)->data(Qt::EditRole).toString().toUShort());
                    m_client->subscribe(topic, qos);
                    addHistory(tr("Subscribe"), topic, "", QString("%1").arg(qos), "");
                    qDebug() << "subscribe topic:" << topic << "qos:" << qos;
                }
            }
        }
    });
    connect(ui->unsubscribe, &QPushButton::clicked, [this](){
        if (m_client->state() == QMqttClient::Connected) {
            for (int i=0; i<m_model->rowCount(); i++) {
                if (m_model->item(i)->checkState() == Qt::Checked) {
                    QString topic = m_model->item(i, 1)->data(Qt::EditRole).toString();
                    m_client->unsubscribe(topic);
                    addHistory(tr("Unsubscribe"), topic, "", "", "");
                    qDebug() << "unsubscribe topic:" << topic;
                }
            }
        }
    });
    connect(ui->hex, &QCheckBox::stateChanged, [this](){
        if (ui->hex->isChecked()) {
            ui->message->setPlainText(ui->message->document()->toPlainText().toLocal8Bit().toHex().toUpper());
            ui->message->setReadOnly(true);
            ui->message->setStyleSheet("QPlainTextEdit{background:transparent;}");
        } else {
            ui->message->setPlainText(QByteArray::fromHex(ui->message->document()->toPlainText().toLocal8Bit()));
            ui->message->setReadOnly(false);
            ui->message->setStyleSheet("");
        }
    });
    connect(ui->file, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("*.txt;;*.*"), nullptr);
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            qDebug() << "open file error" << file.errorString();
            return;
        }
        QByteArray data = file.readAll();
        int i = 0;
        for (i=0; i<data.length(); i++) {
            if (data[i] == '\0')
                break;
        }
        if (i != data.length()) {
            ui->hex->setChecked(true);
            ui->message->document()->setPlainText(data.toHex().toUpper());
        } else {
            ui->hex->setChecked(false);
            ui->message->document()->setPlainText(data);
        }
    });
    connect(ui->publish, &QPushButton::clicked, [this](){
        if (m_client->state() == QMqttClient::Connected) {
            QString topic = ui->topic->text();
            QByteArray message = ui->message->toPlainText().toLocal8Bit();
            quint8 qos = static_cast<quint8>(ui->qos->currentIndex());
            bool retained = ui->retained->isChecked();
            m_client->publish(QMqttTopicName(topic), message, qos, retained);
            addHistory(tr("Publish"), topic, message, QString("%1").arg(qos), retained ? "Yes" : "No");
            qDebug() << "publish topic:" << topic << "qos:" << qos << "retained:" << retained << "message:" << message;
        }
    });


    connect(ui->historyTable, &QTableView::doubleClicked, [this](const QModelIndex &index){
        MessageViewer msg(this);
        msg.setEvent(ui->historyTable->item(index.row(), 0)->text());
        msg.setTopic(ui->historyTable->item(index.row(), 1)->text());
        msg.setMessage(ui->historyTable->item(index.row(), 2)->text());
        msg.setQos(ui->historyTable->item(index.row(), 3)->text());
        msg.setRetained(ui->historyTable->item(index.row(), 4)->text());
        msg.setTime(ui->historyTable->item(index.row(), 5)->text());
        msg.exec();
    });
    connect(ui->historyTable, &QWidget::customContextMenuRequested, [this](){
        m_menu->exec(QCursor::pos());
    });

    connect(m_clear, &QAction::triggered, [this](){
        qDebug() << "clicked history clear button.";
        while (ui->historyTable->rowCount() != 0) {
            ui->historyTable->removeRow(0);
        }
    });
}

void MqttWidget::initMqttEvents()
{
    connect(m_client, &QMqttClient::connected, [this](){
        qDebug() << "client connected.";
        addHistory(tr("Connection"), "", "", "", "");
    });

    connect(m_client, &QMqttClient::disconnected, [this](){
        qDebug() << "client disconnected.";
        addHistory(tr("Disconnection"), "", "", "", "");
    });

    connect(m_client, &QMqttClient::stateChanged, [this](QMqttClient::ClientState state){
        qDebug() << "client state changed:" << state;
        switch (state) {
        case QMqttClient::Disconnected:
            setOnlineStatus(false);
            ui->state->setText(tr("Disconnected"));
            ui->state->setStyleSheet("QLineEdit{background:transparent;}");
            break;
        case QMqttClient::Connecting:
            ui->connect->setEnabled(false);
            ui->disconnect->setEnabled(true);
            ui->state->setText(tr("Connecting"));
            ui->state->setStyleSheet("QLineEdit{background:transparent;}");
            break;
        case QMqttClient::Connected:
            setOnlineStatus(true);
            ui->state->setText(tr("Connected"));
            ui->state->setStyleSheet("QLineEdit{color:green;background:transparent;}");
            break;
        }
    });

    connect(m_client, &QMqttClient::errorChanged, [this](QMqttClient::ClientError error){
        qWarning() << "mqtt error event:" << error;
        addHistory(tr("Error"), "", QString("error:%1").arg(error), "", "");
    });

//    connect(m_client, &QMqttClient::subscribed, [this](const QString topic, const quint8 qos){
//        qDebug() << "subscribed topic:" << topic << "qos:" << qos;
//        addHistory("Subscribed", topic, "", QString("%1").arg(qos), "");
//    });

//    connect(m_client, &QMqttClient::unsubscribed, [this](const QString topic){
//        qDebug() << "unsubscribed topic:" << topic;
//        addHistory("Unsubscribed", topic, "", "", "");
//    });

    connect(m_client, &QMqttClient::messageReceived, [this](const QByteArray &message, const QMqttTopicName &topic){
        qDebug() << "received message, topic:" << topic.name() << "payload:" << message;
        Q_UNUSED(this);
        //addHistory(tr("Received"), topic, payload, QString("%1").arg(qos), retain ? "Yes" : "No");
        addHistory(tr("Received"), topic.name(), message, "", "");
    });

//    connect(m_client, &QMqttClient::published, [this](const QString topic, const quint8 qos, const QByteArray payload, const bool retain){
//        qDebug() << "published message, topic:" << topic << "payload:" << payload;
//        Q_UNUSED(this);
//        //addHistory(tr("Sent"), "", "", "", "");
//        addHistory(tr("Published"), topic, payload, QString("%1").arg(qos), retain ? "Yes" : "No");
//    });
}

void MqttWidget::initStatus()
{
    ui->uri->setText("http://localhost:8200");
    QString uuid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
    QStringList uuids = uuid.split("-");
    ui->id->setText("mqtt-" + uuids[uuids.length() - 1]);
    ui->state->setText(tr("Disconnected"));

    ui->hex->setChecked(false);

    ui->uri->setEnabled(true);
    ui->id->setEnabled(true);
    ui->state->setEnabled(false);
    ui->connect->setEnabled(true);
    ui->disconnect->setEnabled(false);
    ui->subAdd->setEnabled(false);
    ui->subDelete->setEnabled(false);
    ui->subClean->setEnabled(false);
    ui->subscribe->setEnabled(false);
    ui->unsubscribe->setEnabled(false);
    ui->topic->setEnabled(false);
    ui->qos->setEnabled(false);
    ui->retained->setEnabled(false);
    ui->hex->setEnabled(false);
    ui->message->setEnabled(false);
    ui->file->setEnabled(false);
    ui->publish->setEnabled(false);
    ui->historyTable->setEnabled(true);

    setOnlineStatus(false);
}

void MqttWidget::initTabOrder()
{
    setTabOrder(ui->uri, ui->sk);
    setTabOrder(ui->sk, ui->skBrowse);
    setTabOrder(ui->skBrowse, ui->id);
    setTabOrder(ui->id, ui->state);
    setTabOrder(ui->state, ui->connect);
    setTabOrder(ui->connect, ui->disconnect);
    setTabOrder(ui->disconnect, ui->subAdd);
    setTabOrder(ui->subAdd, ui->subDelete);
    setTabOrder(ui->subDelete, ui->subClean);
    setTabOrder(ui->subClean, ui->topics);
    setTabOrder(ui->topics, ui->subscribe);
    setTabOrder(ui->subscribe, ui->unsubscribe);
    setTabOrder(ui->unsubscribe, ui->topic);
    setTabOrder(ui->topic, ui->qos);
    setTabOrder(ui->qos, ui->retained);
    setTabOrder(ui->retained, ui->hex);
    setTabOrder(ui->hex, ui->message);
    setTabOrder(ui->message, ui->file);
    setTabOrder(ui->file, ui->publish);
    setTabOrder(ui->publish, ui->historyTable);
}

void MqttWidget::setOnlineStatus(bool flag)
{
    ui->connect->setDisabled(flag);
    ui->uri->setDisabled(flag);
    ui->id->setDisabled(flag);

    ui->disconnect->setEnabled(flag);
    ui->subAdd->setEnabled(flag);
    ui->topics->setEnabled(flag);
    if (!flag || (flag && m_model->rowCount() > 0)) {
        ui->subDelete->setEnabled(flag);
        ui->subClean->setEnabled(flag);
    }
    ui->subscribe->setEnabled(flag);
    ui->unsubscribe->setEnabled(flag);

    ui->topic->setEnabled(flag);
    ui->qos->setEnabled(flag);
    ui->retained->setEnabled(flag);
    ui->hex->setEnabled(flag);
    ui->message->setEnabled(flag);
    ui->file->setEnabled(flag);
    ui->publish->setEnabled(flag);
}

void MqttWidget::addHistory(QString event, QString topic, QString msg, QString qos, QString retain)
{
    int rowIndex = ui->historyTable->rowCount();
    ui->historyTable->insertRow(rowIndex);
    ui->historyTable->setRowHeight(rowIndex, 25);

    ui->historyTable->setItem(rowIndex, 0, new QTableWidgetItem(event));
    ui->historyTable->setItem(rowIndex, 1, new QTableWidgetItem(topic));
    ui->historyTable->setItem(rowIndex, 2, new QTableWidgetItem(msg));
    ui->historyTable->setItem(rowIndex, 3, new QTableWidgetItem(qos));
    ui->historyTable->setItem(rowIndex, 4, new QTableWidgetItem(retain));
    ui->historyTable->setItem(rowIndex, 5, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")));

    for (int i=0; i<6; i++)
        ui->historyTable->item(rowIndex, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

MqttResponse MqttWidget::doRegister(MqttRequest request) noexcept(false)
{
    qDebug() << "request:" << request.toJson().data();
    QString url = m_params->server;
    if (!url.contains("http")) {
        url = "http://" + url;
    }
    QStringList urls = url.split("//");
    if (urls.length() < 2)
        throw MqttException(QString("invalid url:") + url);
    if (!urls.at(1).contains("/"))
        url += "/device/register2.url";

    qDebug() << "url:" << url;

    QByteArray buf;
    try {
        int ret = post(request.toJson(), "POST", url, buf);
        if (ret != 200)
            throw MqttException(QString(buf));
    } catch (MqttException &e) {
        throw e;
    }

    qDebug() << "response:" << buf;
    MqttResponse response;
    response.fromJson(buf);
    qDebug() << response.broker;
    qDebug() << response.companyID;

    return response;
}
