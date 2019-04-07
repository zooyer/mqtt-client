#include "connectionwidget.h"
#include "ui_connectionwidget.h"
#include <QUuid>
#include <QDateTime>
#include <QFileDialog>
#include <QDebug>
#include "messageviewer.h"

ConnectionWidget::ConnectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionWidget)
{
    ui->setupUi(this);

    m_menu = new QMenu(ui->history);
    m_action = new QAction("Clear", m_menu);
    m_menu->addAction(m_action);

    //m_client = new QMqttClient(this);
    //m_client = nullptr;
    m_client = new QMQTT::Client;
    m_ssl = new QSslConfiguration;

    setConnectEnable(false);
    setSSLEnable(false);
    setHAEnable(false);
    setLWTEnable(false);
    setLoginEnable(false);
    setPersistenceEnable(false);

    ui->status->setStyleSheet("QLineEdit{background:transparent;}");
    ui->tabWidget->setStyleSheet("QTabWidget:pane {border:1px solid rgba(0,0,0,100);background:  transparent; }");

    //ui->tabWidget->setStyleSheet("QTabWidget:pane {background:  transparent; }");
//    ui->messageWidget->setStyleSheet("QTabWidget:pane {border:0px; background:  transparent; }");

    //ui->tableWidget->setStyleSheet("QTabWidget::pane{border:none;}");

//    ui->topics->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//    ui->topics->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
//    ui->topics->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
//    ui->topics->setItem(0,0,new QTableWidgetItem("Jan"));


    if (ui->url->text().length() == 0)
        ui->url->setText("tcp://localhost:1883");
    if (ui->id->text().length() == 0) {
        QString uuid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
        QStringList l = uuid.split("-");
        ui->id->setText("mqtt-" + l[l.length() - 1]);
    }
    ui->status->setText("Disconnected");

    ui->directory->setText(QCoreApplication::applicationDirPath());
    ui->tabWidget->setCurrentIndex(0);
    ui->messageWidget->setCurrentIndex(0);
    ui->history->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->history->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->lastMessage->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->topics->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);


    connect(m_action, &QAction::triggered, [this](){
        int rowCount = ui->history->rowCount();
        if (rowCount > 0) {
            for (int i=0; i<rowCount; i++)
                ui->history->removeRow(rowCount - i - 1);
        }
    });

    connect(ui->directoryBrowse, &QPushButton::clicked, [this](){
        QString dirName = QFileDialog::getExistingDirectory(this, "Please select a directory for persistence store:", "");
        if (dirName.length() > 0)
            ui->directory->setText(dirName);
    });
    connect(ui->trustBrowse, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, "Please select a directory for trust store:", "", tr("*.p12;;*.pfx;;*.*"), 0);
        if (fileName.length() > 0)
            ui->trustLocation->setText(fileName);
    });
    connect(ui->keyBrowse, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, "Please select a directory for key store:", "", tr("*.p12;;*.pfx;;*.*"), 0);
        if (fileName.length() > 0)
            ui->keyLocation->setText(fileName);
    });

    connect(ui->ssl, &QCheckBox::clicked, [this](){
        if (ui->ssl->isChecked()) {
            setSSLEnable(true);
        } else {
            setSSLEnable(false);
        }
    });
    connect(ui->ha, &QCheckBox::clicked, [this](){
        if (ui->ha->isChecked()) {
            setHAEnable(true);
        } else {
            setHAEnable(false);
        }
    });
    connect(ui->lwt, &QCheckBox::clicked, [this](){
        if (ui->lwt->isChecked()) {
            setLWTEnable(true);
        } else {
            setLWTEnable(false);
        }
    });
    connect(ui->login, &QCheckBox::clicked, [this](){
        if (ui->login->isChecked()) {
            setLoginEnable(true);
        } else {
            setLoginEnable(false);
        }
    });
    connect(ui->persistence, &QCheckBox::clicked, [this](){
        if (ui->persistence->isChecked()) {
            setPersistenceEnable(true);
        } else {
            setPersistenceEnable(false);
        }
    });


    connect(ui->history, &QTableWidget::doubleClicked, [this](const QModelIndex &index){
        QString event = ui->history->item(index.row(), 0)->text();
        QString topic = ui->history->item(index.row(), 1)->text();
        QString message = ui->history->item(index.row(), 2)->text();
        QString qos = ui->history->item(index.row(), 3)->text();
        QString retanined = ui->history->item(index.row(), 4)->text();
        QString time = ui->history->item(index.row(), 5)->text();
        qDebug() << "double clicked index:" << index;
        if (qos == "0")
            qos += " - At Most Once";
        else if (qos == "1")
            qos += " - At Least Once";
        else if (qos == "2")
            qos += " - Exactly Once";
        MessageViewer v(event, topic, message, qos, retanined, time);
        if (v.exec() == QDialog::Accepted)
        {
            QString fileName = QFileDialog::getSaveFileName(this,
                    tr("Save As"),
                    "",
                    tr("Config Files (*.txt);; All Files(*.*)"));

                if (!fileName.isNull()) {
                    QFile file(fileName);
                    if (!file.open(QFile::WriteOnly)) {
                        qDebug() << "open file error" << file.errorString();
                        return;
                    }

                    file.write(message.toLatin1());
                    file.close();
                }
        }
    });
    connect(ui->history, &QTableWidget::customContextMenuRequested, [this](){
        m_menu->exec(QCursor::pos());
    });

    connect(ui->connect, &QPushButton::clicked, [this](){
        // paho mqtt lib
//        if (m_client == nullptr)
//            m_client = new mqtt::async_client(ui->url->text().toStdString(), ui->id->text().toStdString());
        /*
        mqtt::connect_options options;
        options.set_clean_session(ui->cleanSession->isChecked());
        options.set_keep_alive_interval(ui->keepAlive->value());
        options.set_connect_timeout(ui->Timeout->value());
        options.set_automatic_reconnect(true);

        if (ui->ssl->isChecked()) {
            mqtt::ssl_options ssl;
            ssl.set_trust_store(ui->trustLocation->text().toStdString());
            ssl.set_key_store(ui->keyLocation->text().toStdString());
            options.set_ssl(ssl);
        }
        if (ui->ha->isChecked()) {
            qDebug() << "not support ha.";
        }
        if (ui->lwt->isChecked()) {
            mqtt::will_options will;
            will.set_topic(ui->lwtTopic->text().toStdString());
            will.set_qos(ui->lwtQos->currentIndex());
            will.set_retained(ui->lwtRetained->isChecked());
            will.set_payload(ui->lwtMessage->document()->toPlainText().toStdString());
            options.set_will(will);
        }
        if (ui->login->isChecked()) {
            options.set_user_name(ui->username->text().toStdString());
            options.set_password(ui->password->text().toStdString());
        }
        if (ui->persistence->isChecked()) {
            qDebug() << "not support persistence.";
        }

        try {
            mqtt::token_ptr conntok = m_client->connect(options);
            //conntok->wait();
        } catch (const exception& exc) {
            qDebug() << "connect error:" << exc.what();
        }
        */

        // qt standard lib.
//        QStringList list = ui->url->text().split(":");
//        if (list.length() < 3)
//            return;

//        QString ip = list[1].replace("//", "");
//        quint16 port = list[2].toUShort();

//        m_client->setHostname(ip);
//        m_client->setPort(port);
//        m_client->setClientId(ui->id->text());
//        m_client->setCleanSession(ui->cleanSession->isChecked());
//        m_client->setKeepAlive(ui->keepAlive->value());

//        if (ui->login->isChecked()) {
//            m_client->setUsername(ui->username->text());
//            m_client->setPassword(ui->password->text());
//        }

//        m_client->connectToHost();
//        qDebug() << "connect to host:" << ip << ":" << port;

        // init ssl
        if (m_client != nullptr)
            delete m_client;
        if (ui->ssl->isChecked()) {
            QSslCertificate certificate;
            QSslKey key;
            QList<QSslCertificate> importedCerts;

            QFile trustFile(ui->trustLocation->text());
            QFile keyFile(ui->keyLocation->text());

            if (keyFile.open(QFile::ReadOnly)) {
                QSslCertificate::importPkcs12(&keyFile, &key, &certificate, &importedCerts, ui->keyPassword->text().toLocal8Bit());
                keyFile.close();
                m_ssl->setLocalCertificateChain(importedCerts);
//                m_ssl->setCaCertificates();
//                m_ssl->setProtocol();
                importedCerts.clear();
            }


            if (trustFile.open(QFile::ReadOnly)) {
                QSslCertificate::importPkcs12(&trustFile, &key, &certificate, &importedCerts, ui->trustPassword->text().toLocal8Bit());
                trustFile.close();
                m_ssl->setCaCertificates(importedCerts);
                m_ssl->setLocalCertificate(certificate);
                m_ssl->setPrivateKey(key);
            }

            m_ssl->setPeerVerifyMode(QSslSocket::VerifyNone);
            m_ssl->setProtocol(QSsl::AnyProtocol);
            m_client = new QMQTT::Client("localhost", 1883, *m_ssl);
        } else {
            m_client = new QMQTT::Client;
        }


        QStringList list = ui->url->text().split(":");
        if (list.length() < 3)
            return;

        QString ip = list[1].replace("//", "");
        quint16 port = list[2].toUShort();

        //m_client->setAutoReconnect(true);
        m_client->setCleanSession(ui->cleanSession->isChecked());
        m_client->setClientId(ui->id->text());
        m_client->setKeepAlive(quint16(ui->keepAlive->value()));
        //m_client->setHost(ui->url->text());
        m_client->setHostName(ip);
        m_client->setPort(port);

        if (ui->ha->isChecked()) {
            qDebug() << "not support ha!";
        }
        if (ui->lwt->isChecked()) {
            m_client->setWillTopic(ui->lwtTopic->text());
            m_client->setWillQos(quint8(ui->lwtQos->currentIndex()));
            m_client->setWillRetain(ui->lwtRetained->isChecked());
            m_client->setWillMessage(ui->lwtMessage->document()->toPlainText().toLatin1());
        }
        if (ui->login->isChecked()) {
            m_client->setUsername(ui->username->text());
            m_client->setPassword(ui->password->text().toLatin1());
        }
        if (ui->persistence->isChecked()) {
            qDebug() << "not support persistence.";
        }

        // to connecting.
        setConnectEnable(false);
        ui->connect->setEnabled(false);
        ui->status->setText("Connecting");

        m_client->connectToHost();
        qDebug() << "connect to host:" << ip << ":" << port;

    });
    connect(ui->disconnect, &QPushButton::clicked, [this](){
        // paho lib.
//        if (m_client->is_connected()) {
//            m_client->disconnect();
//            delete m_client;
//            m_client = nullptr;
//        }
        // standard lib.
//        if (m_client->state() == QMqttClient::Connected)
//            m_client->disconnectFromHost();

        // is zero???
        qDebug() << m_client->connectionState();
//        if (m_client->connectionState() == QMQTT::STATE_CONNECTED)
//            m_client->disconnectFromHost();

        if (m_client->isConnectedToHost())
            m_client->disconnectFromHost();
    });


    connect(ui->subAdd, &QPushButton::clicked, [this](){
        int rowIndex = ui->topics->rowCount();
        ui->topics->insertRow(rowIndex);
        ui->topics->setRowHeight(rowIndex, 25);
        QTableWidgetItem *itemTopic = new QTableWidgetItem("test");
        QTableWidgetItem *itemQos = new QTableWidgetItem("0");
        QTableWidgetItem *check = new QTableWidgetItem;
        check->setCheckState(Qt::Checked);

        ui->topics->setItem(rowIndex, 0, check);
        ui->topics->setItem(rowIndex, 1, itemTopic);
        ui->topics->setItem(rowIndex, 2, itemQos);

        for (int i=0; i<3; i++)
            ui->topics->item(rowIndex, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    });
    connect(ui->subDelete, &QPushButton::clicked, [this](){
        int currentRow = ui->topics->currentRow();
        if (currentRow >= 0)
            ui->topics->removeRow(currentRow);
    });
    connect(ui->subClean, &QPushButton::clicked, [this](){
        int rowCount = ui->topics->rowCount();
        if (rowCount > 0) {
            for (int i=0; i<rowCount; i++)
                ui->topics->removeRow(rowCount - i - 1);
        }
    });
    connect(ui->subscribe, &QPushButton::clicked, [this](){
        for (int i=0; i<ui->topics->rowCount(); i++) {
            if (ui->topics->item(i, 0)->checkState() == Qt::Checked) {
                QString topic = ui->topics->item(i, 1)->text();
                QString qos = ui->topics->item(i, 2)->text();
                if (topic.length() > 0 && (qos == "0" || qos == "1" || qos == "2")) {
                    // qt standard.
                    //m_client->subscribe(QMqttTopicFilter(topic), qos.toUShort());

                    // paho lib.
                    //m_client->subscribe(topic.toStdString(), qos.toInt());

                    m_client->subscribe(topic, (quint8)qos.toUShort());
                    //historyAdd("Subscribed", topic, "", qos, "");
                }
            }
        }
    });
    connect(ui->unsubscribe, &QPushButton::clicked, [this](){
        for (int i=0; i<ui->topics->rowCount(); i++) {
            if (ui->topics->item(i, 0)->checkState() == Qt::Checked) {
                QString topic = ui->topics->item(i, 1)->text();
                if (topic.length() > 0) {
                    // qt standard lib
                    //m_client->unsubscribe(QMqttTopicFilter(topic));
                    // eclipse paho lib
                    //m_client->unsubscribe(topic.toStdString());
                    m_client->unsubscribe(topic);
                    //historyAdd("Unsubscribed", topic, "", "", "");
                }
            }
        }
    });


    connect(ui->hex, &QCheckBox::clicked, [this](){
        if (ui->hex->isChecked()) {
            ui->message->setPlainText(ui->message->document()->toPlainText().toLatin1().toHex().toUpper());
            ui->message->setReadOnly(true);
            ui->message->setStyleSheet("QPlainTextEdit{background:transparent;}");
        } else {
            ui->message->setPlainText(QByteArray::fromHex(ui->message->document()->toPlainText().toLatin1()));
            ui->message->setReadOnly(false);
            ui->message->setStyleSheet("");
        }
    });
    connect(ui->file, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", tr("*.txt;;*.*"), 0);
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
        if (ui->topic->text().length() == 0)
            return;

        QString topic = ui->topic->text();
        QString msg = ui->message->document()->toPlainText();
        quint8 qos = (quint8)ui->qos->currentIndex();
        bool retained = ui->retained->isChecked();

        // qt standard lib.
        //m_client->publish(QMqttTopicName(topic), msg.toLatin1(), qos, retained);
        // eclipse paho lib.
        //m_client->publish(topic.toStdString(), msg.data(), msg.length(), qos, retained)->wait();

//        explicit Message(const quint16 id, const QString &topic, const QByteArray &payload,
//                         const quint8 qos = 0, const bool retain = false, const bool dup = false);

        QMQTT::Message message(0, topic, msg.toLatin1(), qos, retained);
        m_client->publish(message);
        //historyAdd("Publish", topic, msg, QString("%1").arg(qos), retained ? "Yes" : "No");
    });


    // qt standard lib.
    /*

    connect(m_client, &QMqttClient::connected, [this](){
        historyAdd("Connected", "", "", "", "");
    });
    connect(m_client, &QMqttClient::disconnected, [this](){
        historyAdd("Disconnected", "", "", "", "");
    });
    connect(m_client, &QMqttClient::stateChanged, [this](){
        qDebug() << "state is changed.";
        switch (m_client->state()) {
        case QMqttClient::Connected:
            setConnectEnable(true);
            ui->status->setText("Connected");
            ui->status->setStyleSheet("QLineEdit{color:green;background:transparent;}");
            break;
        case QMqttClient::Connecting:
            setConnectEnable(false);
            ui->connect->setEnabled(false);
            ui->status->setText("Connecting");
            break;
        case QMqttClient::Disconnected:
            setConnectEnable(false);
            ui->status->setText("Disconnected");
            ui->status->setStyleSheet("QLineEdit{background:transparent;}");
            break;
        default:
            break;
        }
        //ui->status->setText(m_client->state().);
    });
    connect(m_client, &QMqttClient::messageReceived, [this](const QByteArray &message, const QMqttTopicName &topic){
        historyAdd("Received", topic.name(), message, "", "");
        if (ui->lastMessage->rowCount() <= 0)
            ui->lastMessage->insertRow(0);

        ui->lastMessage->setRowHeight(0, 25);

        QTableWidgetItem *itemTopic = new QTableWidgetItem(topic.name());
        QTableWidgetItem *itemMessage = new QTableWidgetItem(QString(message));
        QTableWidgetItem *itemQos = new QTableWidgetItem("");
        QTableWidgetItem *itemRetain = new QTableWidgetItem("");
        QTableWidgetItem *itemTime = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"));

        ui->lastMessage->setItem(0, 0, itemTopic);
        ui->lastMessage->setItem(0, 1, itemMessage);
        ui->lastMessage->setItem(0, 2, itemQos);
        ui->lastMessage->setItem(0, 3, itemRetain);
        ui->lastMessage->setItem(0, 4, itemTime);

        for (int i=0; i<5; i++)
            ui->lastMessage->item(0, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    });

    */




    connect(m_client, &QMQTT::Client::error, [this](const QMQTT::ClientError error){
        qDebug() << "QMQTT error:" << error;
    });

    connect(m_client, &QMQTT::Client::published, [this](const QMQTT::Message& message, quint16 msgid){
        qDebug() << "msgid" << msgid;
        historyAdd("Publish", message.topic(), QString(message.payload()), QString("%1").arg(message.qos()), message.retain() ? "Yes" : "No");
    });

    connect(m_client, &QMQTT::Client::subscribed, [this](const QString& topic, const quint8 qos){
        historyAdd("Subscribed", topic, "", QString("%1").arg(qos), "");
    });
    connect(m_client, &QMQTT::Client::unsubscribed, [this](const QString& topic){
        historyAdd("Unsubscribed", topic, "", "", "");
    });

    connect(m_client, &QMQTT::Client::connected, [this](){
        historyAdd("Connected", "", "", "", "");
        setConnectEnable(true);
        ui->status->setText("Connected");
        ui->status->setStyleSheet("QLineEdit{color:green;background:transparent;}");
    });
    connect(m_client, &QMQTT::Client::disconnected, [this](){
        historyAdd("Disconnected", "", "", "", "");
        setConnectEnable(false);
        ui->status->setText("Disconnected");
        ui->status->setStyleSheet("QLineEdit{background:transparent;}");
    });

    connect(m_client, &QMQTT::Client::received, [this](const QMQTT::Message& message){
        historyAdd("Received", message.topic(), message.payload(), QString("%1").arg(message.qos()), message.retain() ? "Yes" : "No");
        if (ui->lastMessage->rowCount() <= 0)
            ui->lastMessage->insertRow(0);

        ui->lastMessage->setRowHeight(0, 25);

        QTableWidgetItem *itemTopic = new QTableWidgetItem(message.topic());
        QTableWidgetItem *itemMessage = new QTableWidgetItem(QString(message.payload()));
        QTableWidgetItem *itemQos = new QTableWidgetItem(QString("%1").arg(message.qos()));
        QTableWidgetItem *itemRetain = new QTableWidgetItem(message.retain() ? "Yes" : "No");
        QTableWidgetItem *itemTime = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"));

        ui->lastMessage->setItem(0, 0, itemTopic);
        ui->lastMessage->setItem(0, 1, itemMessage);
        ui->lastMessage->setItem(0, 2, itemQos);
        ui->lastMessage->setItem(0, 3, itemRetain);
        ui->lastMessage->setItem(0, 4, itemTime);

        for (int i=0; i<5; i++)
            ui->lastMessage->item(0, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    });

}

ConnectionWidget::~ConnectionWidget()
{
    delete ui;
}

/*   eclipse paho mqtt lib.
 *
void ConnectionWidget::connected(const string &cause)
{
    qDebug() << "connected!!!";
    historyAdd("Connected", "", "", "", "");
    setConnectEnable(true);
    ui->status->setText("Connected");
    ui->status->setStyleSheet("QLineEdit{color:green;background:transparent;}");
}

void ConnectionWidget::connection_lost(const string &cause)
{
    qDebug() << "lost!!!";
    historyAdd("Disconnected", "", "", "", "");
    setConnectEnable(false);
    ui->status->setText("Disconnected");
    ui->status->setStyleSheet("QLineEdit{background:transparent;}");
}

void ConnectionWidget::message_arrived(mqtt::const_message_ptr msg)
{
    return;
    QString topic = QString::fromStdString(msg->get_topic());
    QString message = QString::fromStdString(msg->to_string());
    historyAdd("Received", topic, message, QString("%1").arg(msg->get_qos()), msg->is_retained() ? "Yes" : "No");
    if (ui->lastMessage->rowCount() <= 0)
        ui->lastMessage->insertRow(0);

    ui->lastMessage->setRowHeight(0, 25);

    QTableWidgetItem *itemTopic = new QTableWidgetItem(topic);
    QTableWidgetItem *itemMessage = new QTableWidgetItem(message);
    QTableWidgetItem *itemQos = new QTableWidgetItem("");
    QTableWidgetItem *itemRetain = new QTableWidgetItem("");
    QTableWidgetItem *itemTime = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"));

    ui->lastMessage->setItem(0, 0, itemTopic);
    ui->lastMessage->setItem(0, 1, itemMessage);
    ui->lastMessage->setItem(0, 2, itemQos);
    ui->lastMessage->setItem(0, 3, itemRetain);
    ui->lastMessage->setItem(0, 4, itemTime);

    for (int i=0; i<5; i++)
        ui->lastMessage->item(0, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

void ConnectionWidget::delivery_complete(mqtt::deliveryTokenPtr tok)
{

}

*/

void ConnectionWidget::setConnectEnable(bool flag)
{
    ui->url->setDisabled(flag);
    ui->id->setDisabled(flag);
    ui->connect->setDisabled(flag);

    ui->disconnect->setEnabled(flag);
    ui->topics->setEnabled(flag);
    ui->subAdd->setEnabled(flag);
    ui->subDelete->setEnabled(flag);
    ui->subClean->setEnabled(flag);
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

void ConnectionWidget::setLoginEnable(bool flag)
{
    ui->username->setEnabled(flag);
    ui->password->setEnabled(flag);
}

void ConnectionWidget::setPersistenceEnable(bool flag)
{
    ui->directoryBrowse->setEnabled(flag);
}

void ConnectionWidget::setSSLEnable(bool flag)
{
    ui->keyLocation->setEnabled(flag);
    ui->keyPassword->setEnabled(flag);
    ui->keyBrowse->setEnabled(flag);
    ui->trustLocation->setEnabled(flag);
    ui->trustPassword->setEnabled(flag);
    ui->trustBrowse->setEnabled(flag);
}

void ConnectionWidget::setHAEnable(bool flag)
{
    ui->haAdd->setEnabled(flag);
    ui->haDelete->setEnabled(flag);
    ui->haClean->setEnabled(flag);
    ui->servers->setEnabled(flag);
}

void ConnectionWidget::setLWTEnable(bool flag)
{
    ui->lwtTopic->setEnabled(flag);
    ui->lwtQos->setEnabled(flag);
    ui->lwtRetained->setEnabled(flag);
    ui->lwtHex->setEnabled(flag);
    ui->lwtMessage->setEnabled(flag);
}

void ConnectionWidget::historyAdd(QString event, QString topic, QString msg, QString qos, QString retain)
{
    int rowIndex = ui->history->rowCount();
    ui->history->insertRow(rowIndex);
    ui->history->setRowHeight(rowIndex, 25);

    QTableWidgetItem *itemEvent = new QTableWidgetItem(event);
    QTableWidgetItem *itemTopic = new QTableWidgetItem(topic);
    QTableWidgetItem *itemMessage = new QTableWidgetItem(msg);
    QTableWidgetItem *itemQos = new QTableWidgetItem(qos);
    QTableWidgetItem *itemRetain = new QTableWidgetItem(retain);
    QTableWidgetItem *itemTime = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"));


    ui->history->setItem(rowIndex, 0, itemEvent);
    ui->history->setItem(rowIndex, 1, itemTopic);
    ui->history->setItem(rowIndex, 2, itemMessage);
    ui->history->setItem(rowIndex, 3, itemQos);
    ui->history->setItem(rowIndex, 4, itemRetain);
    ui->history->setItem(rowIndex, 5, itemTime);

    for (int i=0; i<6; i++)
        ui->history->item(rowIndex, i)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}

