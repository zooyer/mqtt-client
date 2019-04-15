#include "connectionwidget.h"
#include "ui_connectionwidget.h"
#include "messageviewer.h"
#include <QDebug>
#include <QDateTime>
#include <QItemDelegate>
#include <QFileDialog>
#include <QStringList>
#include <QUuid>
#include <QSslKey>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>

ConnectionWidget::ConnectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionWidget)
{
    ui->setupUi(this);

    m_client = new QMqttClient;
    m_ssl = new QSslSocket;

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

    // set last message table style.
    ui->lastTable->verticalHeader()->setHidden(true);
    ui->lastTable->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui->lastTable->horizontalHeader()->setStretchLastSection(true);
    ui->lastTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->lastTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->lastTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->lastTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    ui->lastTable->setColumnWidth(2, 80);
    ui->lastTable->setColumnWidth(3, 80);
    ui->lastTable->setColumnWidth(4, 200);


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
    headerList << "" << tr("Topic") << tr("QoS");
    m_model->setHorizontalHeaderLabels(headerList);
    ui->topics->verticalHeader()->setVisible(false);
    ui->topics->horizontalHeader()->setStretchLastSection(true);

    // set topics table delegate combo box.
    QosDelegate *qosDelegate = new QosDelegate(ui->topics);
    ui->topics->setItemDelegateForColumn(2, qosDelegate);


    // set HA servers table style.
    ui->servers->verticalHeader()->setHidden(true);
    ui->servers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->servers->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // init status
    initStatus();

    // init tab order
    initTabOrder();

    // init mqtt events.
    initMqttEvents();

    // init signal connect.
    initConnect();

    // test
    //addHistory("add", "tttt", "sss", "aa", "aaaa");
    //addHistory("add", "tttt", "sss", "aa", "aaaa");
    //addHistory("add", "tttt", "sss", "aa", "aaaa");
//    setLastMessage("tttt", "sss", "aa", "aaaa");
//    setLastMessage("tttt", "sss123", "aa", "aaaa");
}

void ConnectionWidget::initConnect()
{
    connect(ui->connect, &QPushButton::clicked, [this](){
        if (m_client->state() == QMqttClient::Connected) {
            return;
        }
        QStringList uri = ui->uri->text().split(":");
        if (uri.length() < 2 || uri.length() > 3) {
            qDebug() << "uri error.";
            return;
        } else if (uri.length() == 2) {
            m_client->setHostname(uri[0]);
            m_client->setPort(uri[1].toUShort());
        } else {
            m_client->setHostname(uri[1].replace("//", ""));
            m_client->setPort(uri[2].toUShort());
        }
        m_client->setClientId(ui->id->text());
        m_client->setCleanSession(ui->cleanSession->isChecked());
        m_client->setKeepAlive(static_cast<quint16>(ui->keepAlive->value()));

        // ssl
        if (ui->ssl->isChecked()) {
            QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
            QSslCertificate certificate;
            QSslKey key;
            QList<QSslCertificate> importedCerts;

            QFile trustFile(ui->trust->text());
            QFile keyFile(ui->key->text());

            if (keyFile.open(QFile::ReadOnly)) {
                QSslCertificate::importPkcs12(&keyFile, &key, &certificate, &importedCerts, ui->keyPassword->text().toLocal8Bit());
                keyFile.close();
                sslConfig.setLocalCertificateChain(importedCerts);
//                m_ssl->setCaCertificates();
//                m_ssl->setProtocol();
                importedCerts.clear();
            }


            if (trustFile.open(QFile::ReadOnly)) {
                QSslCertificate::importPkcs12(&trustFile, &key, &certificate, &importedCerts, ui->trustPassword->text().toLocal8Bit());
                trustFile.close();
                sslConfig.setCaCertificates(importedCerts);
                sslConfig.setLocalCertificate(certificate);
                sslConfig.setPrivateKey(key);
            }

            sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
            sslConfig.setProtocol(QSsl::AnyProtocol);
            m_ssl->setSslConfiguration(sslConfig);
            m_client->setTransport(m_ssl, QMqttClient::SecureSocket);
            m_ssl->connectToHostEncrypted(m_client->hostname(), m_client->port());
            if (!m_ssl->waitForEncrypted()) {
                qDebug() << "wait tls connection false";
                addHistory(tr("Error"), "", "wait tls connection failed", "", "");
            }
            m_client->connectToHostEncrypted();
        }
        // HA
        if (ui->ha->isChecked()) {
            qDebug() << "not support HA.";
            addHistory(tr("Error"), "", "not support HA", "", "");
        }
        // login
        if (ui->login->isChecked()) {
            m_client->setUsername(ui->username->text());
            m_client->setPassword(ui->password->text());
        }
        // lwt
        if (ui->lwt->isChecked()) {
            m_client->setWillTopic(ui->lwtTopic->text());
            m_client->setWillQoS(static_cast<quint8>(ui->lwtQos->currentIndex()));
            m_client->setWillRetain(ui->lwtRetained->isChecked());
            m_client->setWillMessage(ui->lwtMessage->toPlainText().toLocal8Bit());
        }
        if (m_client->state() == QMqttClient::Disconnected) {
            m_client->connectToHost();
        }

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
                    auto subscription  = m_client->subscribe(QMqttTopicFilter(topic), qos);
                    if (!subscription) {
                        qWarning() << "Subscribe Error:" << "Could not subscribe. Is there a valid connection?";
                        addHistory("Error", topic, "Could not subscribe. Is there a valid connection?", QString("%1").arg(qos), "");
                    } else {
                        qDebug() << "subscribe topic:" << topic << "qos:" << qos;
                        addHistory(tr("Subscribe"), topic, "", QString("%1").arg(qos), "");
                        connect(subscription, &QMqttSubscription::messageReceived, [this](QMqttMessage msg){
                            qDebug() << "received message, topic:" << msg.topic().name() << "payload:" << msg.payload();
                            addHistory(tr("Received"), msg.topic().name(), msg.payload(), QString("%1").arg(msg.qos()), msg.retain() ? "Yes" : "No");
                            setLastMessage(msg.topic().name(), msg.payload(), QString("%1").arg(msg.qos()), msg.retain() ? "Yes" : "No");
                        });
                    }
                }
            }
        }
    });
    connect(ui->unsubscribe, &QPushButton::clicked, [this](){
        if (m_client->state() == QMqttClient::Connected) {
            for (int i=0; i<m_model->rowCount(); i++) {
                if (m_model->item(i)->checkState() == Qt::Checked) {
                    QString topic = m_model->item(i, 1)->data(Qt::EditRole).toString();
                    qDebug() << "unsubscribe topic:" << topic;
                    m_client->unsubscribe(QMqttTopicFilter(topic));
                    addHistory("Unsubscribe", topic, "", "", "");
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
            qDebug() << "publish topic:" << topic << "qos:" << qos << "retained:" << retained << "message:" << message;
            addHistory(tr("Publish"), topic, message, QString("%1").arg(qos), retained ? "Yes" : "No");
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
    connect(ui->lastTable, &QTableWidget::doubleClicked, [this](const QModelIndex &index){
        MessageViewer msg(this);
        msg.setEvent(tr("Last Message"));
        msg.setTopic(ui->lastTable->item(index.row(), 0)->text());
        msg.setMessage(ui->lastTable->item(index.row(), 1)->text());
        msg.setQos(ui->lastTable->item(index.row(), 2)->text());
        msg.setRetained(ui->lastTable->item(index.row(), 3)->text());
        msg.setTime(ui->lastTable->item(index.row(), 4)->text());
        msg.exec();
    });


    connect(m_clear, &QAction::triggered, [this](){
        qDebug() << "clicked history clear button.";
        while (ui->historyTable->rowCount() != 0) {
            ui->historyTable->removeRow(0);
        }
    });


    connect(ui->ssl, &QCheckBox::stateChanged, [this](){
        bool enabledFlag = false;
        if (ui->ssl->isChecked()) {
            enabledFlag = true;
        }
        ui->key->setEnabled(enabledFlag);
        ui->keyPassword->setEnabled(enabledFlag);
        ui->keyBrowse->setEnabled(enabledFlag);
        ui->trust->setEnabled(enabledFlag);
        ui->trustPassword->setEnabled(enabledFlag);
        ui->trustBrowse->setEnabled(enabledFlag);
    });
    connect(ui->ha, &QCheckBox::stateChanged, [this](){
        bool enabledFlag = false;
        if (ui->ha->isChecked()) {
            enabledFlag = true;
        }
        ui->haAdd->setEnabled(enabledFlag);
        if (!ui->ha->isChecked() || (ui->ha->isChecked() && ui->servers->rowCount() > 0)) {
            ui->haDelete->setEnabled(enabledFlag);
            ui->haClean->setEnabled(enabledFlag);
        }
        ui->servers->setEnabled(enabledFlag);
    });
    connect(ui->lwt, &QCheckBox::stateChanged, [this](){
        bool enabledFlag = false;
        if (ui->lwt->isChecked()) {
            enabledFlag = true;
        }
        ui->lwtTopic->setEnabled(enabledFlag);
        ui->lwtQos->setEnabled(enabledFlag);
        ui->lwtRetained->setEnabled(enabledFlag);
        ui->lwtHex->setEnabled(enabledFlag);
        ui->lwtMessage->setEnabled(enabledFlag);
    });
    connect(ui->login, &QCheckBox::stateChanged, [this](){
        bool enabledFlag = false;
        if (ui->login->isChecked()) {
            enabledFlag = true;
        }
        ui->username->setEnabled(enabledFlag);
        ui->password->setEnabled(enabledFlag);
    });
    connect(ui->persistence, &QCheckBox::stateChanged, [this](){
        bool enabledFlag = false;
        if (ui->persistence->isChecked()) {
            enabledFlag = true;
        }
        ui->directory->setEnabled(enabledFlag);
        ui->dirBrowse->setEnabled(enabledFlag);
    });
    connect(ui->dirBrowse, &QPushButton::clicked, [this](){
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Please select a directory for persistence store"));
        if (dirName.length() > 0)
            ui->directory->setText(dirName);
    });
    connect(ui->keyBrowse, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Please select a directory for key store"), "", tr("*.p12;;*.pfx;;*.*"));
        if (fileName.length() > 0)
            ui->key->setText(fileName);
    });
    connect(ui->trustBrowse, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Please select a directory for trust store"), "", tr("*.p12;;*.pfx;;*.*"));
        if (fileName.length() > 0)
            ui->trust->setText(fileName);
    });
    connect(ui->haAdd, &QPushButton::clicked, [this](){
        qDebug() << "clicked HA add button.";
        int rowIndex = ui->servers->rowCount();
        ui->servers->insertRow(rowIndex);

        ui->servers->setItem(rowIndex, 0, new QTableWidgetItem("tcp://localhost:1883"));

        if (!ui->haDelete->isEnabled()) {
            ui->haDelete->setEnabled(true);
        }
        if (!ui->haClean->isEnabled()) {
            ui->haClean->setEnabled(true);
        }
    });
    connect(ui->haDelete, &QPushButton::clicked, [this](){
        qDebug() << "clicked HA del button.";
        QModelIndex index = ui->servers->currentIndex();
        if (index.isValid()) {
            ui->servers->removeRow(index.row());
            if (ui->servers->rowCount() == 0) {
                ui->haDelete->setEnabled(false);
                ui->haClean->setEnabled(false);
            }
        }
    });
    connect(ui->haClean, &QPushButton::clicked, [this](){
        qDebug() << "clicked HA clean.";
        while (ui->servers->rowCount() != 0) {
            ui->servers->removeRow(0);
        }
        ui->haDelete->setEnabled(false);
        ui->haClean->setEnabled(false);
    });
    connect(ui->lwtHex, &QPushButton::clicked, [this](){
        if (ui->lwtHex->isChecked()) {
            ui->lwtMessage->setPlainText(ui->lwtMessage->document()->toPlainText().toLocal8Bit().toHex().toUpper());
            ui->lwtMessage->setReadOnly(true);
            ui->lwtMessage->setStyleSheet("QPlainTextEdit{background:transparent;}");
        } else {
            ui->lwtMessage->setPlainText(QByteArray::fromHex(ui->message->document()->toPlainText().toLocal8Bit()));
            ui->lwtMessage->setReadOnly(false);
            ui->lwtMessage->setStyleSheet("");
        }
    });
}

void ConnectionWidget::initMqttEvents()
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
        addHistory(tr("Error"), "", QString("%1").arg(error), "", "");
    });

/*
#ifndef QT_NO_SSL
    connect(m_client, &QMqttClient::messageStatusChanged, [this](qint32 id, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &properties){
        qDebug() << "mqtt message changed:" << (quint8)s;
        Q_UNUSED(id);
        Q_UNUSED(properties);
        switch (s) {
        case QMqtt::MessageStatus::Unknown:

            break;
        case QMqtt::MessageStatus::Published:

            break;
        case QMqtt::MessageStatus::Acknowledged:

            break;
        case QMqtt::MessageStatus::Received:

            break;
        case QMqtt::MessageStatus::Released:

            break;
        case QMqtt::MessageStatus::Completed:

            break;
        default:
            break;
        }
    });

#endif
*/

    connect(m_client, &QMqttClient::messageReceived, [this](const QByteArray &message, const QMqttTopicName &topic){
        qDebug() << "default received message, topic:" << topic.name() << "payload:" << message;
        //addHistory(tr("Received"), topic.name(), message, "", "");
        //setLastMessage(topic.name(), message, "", "");
        Q_UNUSED(this);
    });

    connect(m_client, &QMqttClient::messageSent, [this](qint32 id){
        qDebug() << "send message id:" << id;
        //addHistory(tr("Sent"), "", "", "", "");
        Q_UNUSED(this);
    });
}

void ConnectionWidget::initStatus()
{
    ui->uri->setText("tcp://localhost:1883");
    QString uuid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
    QStringList uuids = uuid.split("-");
    ui->id->setText("mqtt-" + uuids[uuids.length() - 1]);
    ui->state->setText(tr("Disconnected"));

    ui->hex->setChecked(false);
    ui->cleanSession->setChecked(true);
    ui->ssl->setChecked(false);
    ui->ha->setChecked(false);
    ui->lwt->setChecked(false);
    ui->keepAlive->setValue(60);
    ui->timeout->setValue(30);
    ui->login->setChecked(false);
    ui->persistence->setChecked(false);
    ui->directory->setText(QCoreApplication::applicationDirPath());

    // set history table and last message table current index is zero.
    ui->tabWidget->setCurrentIndex(0);
    ui->messageTableWidget->setCurrentIndex(0);

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
    ui->lastTable->setEnabled(true);
    ui->cleanSession->setEnabled(true);
    ui->ssl->setEnabled(true);
    ui->ha->setEnabled(true);
    ui->lwt->setEnabled(true);
    ui->keepAlive->setEnabled(true);
    ui->timeout->setEnabled(true);
    ui->login->setEnabled(true);
    ui->username->setEnabled(false);
    ui->password->setEnabled(false);
    ui->persistence->setEnabled(true);
    ui->directory->setEnabled(false);
    ui->dirBrowse->setEnabled(false);
    ui->key->setEnabled(false);
    ui->keyBrowse->setEnabled(false);
    ui->keyPassword->setEnabled(false);
    ui->trust->setEnabled(false);
    ui->trustBrowse->setEnabled(false);
    ui->trustPassword->setEnabled(false);
    ui->haAdd->setEnabled(false);
    ui->haDelete->setEnabled(false);
    ui->haClean->setEnabled(false);
    ui->servers->setEnabled(false);
    ui->lwtTopic->setEnabled(false);
    ui->lwtQos->setEnabled(false);
    ui->lwtRetained->setEnabled(false);
    ui->lwtHex->setEnabled(false);
    ui->lwtMessage->setEnabled(false);

    setOnlineStatus(false);
}

void ConnectionWidget::initTabOrder()
{
    setTabOrder(ui->uri, ui->id);
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
    setTabOrder(ui->publish, ui->tabWidget);
}

void ConnectionWidget::setOnlineStatus(bool flag)
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

void ConnectionWidget::addHistory(QString event, QString topic, QString msg, QString qos, QString retain)
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

void ConnectionWidget::setLastMessage(QString topic, QString msg, QString qos, QString retain)
{
    qDebug() << "row count:" << ui->lastTable->rowCount();
    if (ui->lastTable->rowCount() < 1) {
        ui->lastTable->insertRow(0);
        ui->lastTable->setRowHeight(0, 25);
//        for (int i=0; i<5; i++)
//            ui->lastTable->item(0, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }

    ui->lastTable->setItem(0, 0, new QTableWidgetItem(topic));
    ui->lastTable->setItem(0, 1, new QTableWidgetItem(msg));
    ui->lastTable->setItem(0, 2, new QTableWidgetItem(qos));
    ui->lastTable->setItem(0, 3, new QTableWidgetItem(retain));
    ui->lastTable->setItem(0, 4, new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd")));
}

ConnectionWidget::~ConnectionWidget()
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

    delete ui;
}

