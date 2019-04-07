#include "mqttsdkwidget.h"
#include "ui_mqttsdkwidget.h"
#include <QUuid>
#include <QDateTime>
#include <QFileDialog>
#include <QDebug>
#include "messageviewer.h"

MqttSDKWidget::MqttSDKWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MqttSDKWidget)
{
    ui->setupUi(this);

    m_menu = new QMenu(ui->history);
    m_action = new QAction("Clear", m_menu);
    m_menu->addAction(m_action);
    m_client = new MqttSDK(this);


    setConnectEnable(false);

    ui->status->setStyleSheet("QLineEdit{background:transparent;}");
    //ui->tabWidget->setStyleSheet("QTabWidget:pane {border:1px solid rgba(0,0,0,100);background:  transparent; }");


    if (ui->url->text().length() == 0)
        ui->url->setText("http://localhost:8200");
    if (ui->id->text().length() == 0) {
        QString uuid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
        QStringList l = uuid.split("-");
        ui->id->setText("mqtt-" + l[l.length() - 1]);
    }
    ui->status->setText("Disconnected");

    ui->history->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->history->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->topics->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);


    connect(m_action, &QAction::triggered, [this](){
        int rowCount = ui->history->rowCount();
        if (rowCount > 0) {
            for (int i=0; i<rowCount; i++)
                ui->history->removeRow(rowCount - i - 1);
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

    connect(ui->skBrowse, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, "Open DeviceSK File", "", tr("*.sk;;*.*"), 0);
        if (fileName.length() > 0)
            ui->sk->setText(fileName);
    });

    connect(ui->connect, &QPushButton::clicked, [this](){
        MqttSDKParams params;
        params.deviceID = ui->id->text();
        params.deviceSK = ui->sk->text();
        params.server = ui->url->text();
        m_client->setParams(params);

        // to connecting.
        setConnectEnable(false);
        ui->connect->setEnabled(false);
        ui->status->setText("Connecting");

        if (!m_client->init()) {
            ui->connect->setEnabled(true);
            ui->status->setText("Disconnected");
        }
    });

    connect(ui->disconnect, &QPushButton::clicked, [this](){
        if (m_client->isConnected() && m_client->uninit()) {
            qDebug() << "disconnected";
        }
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
                    m_client->receiveBroadcast(topic, (quint8)qos.toUShort(), nullptr);
                    historyAdd("Subscribed", topic, "", qos, "");
                }
            }
        }
    });
    connect(ui->unsubscribe, &QPushButton::clicked, [this](){
        for (int i=0; i<ui->topics->rowCount(); i++) {
            if (ui->topics->item(i, 0)->checkState() == Qt::Checked) {
                QString topic = ui->topics->item(i, 1)->text();
                if (topic.length() > 0) {
                    m_client->unReceiveBroadcast(topic);
                    historyAdd("Unsubscribed", topic, "", "", "");
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

        m_client->broadcast(topic, qos, retained, msg.toLatin1());
        historyAdd("Publish", topic, msg, QString("%1").arg(qos), retained ? "Yes" : "No");
    });


    connect(m_client, &MqttSDK::errored, [this](const QMQTT::ClientError error){
        if (!m_client->isConnected()) {

            ui->connect->setEnabled(true);
            ui->status->setText("Disconnected");
            setConnectEnable(false);
        }
        qDebug() << "QMQTT error:" << error;
    });

    connect(m_client, &MqttSDK::connected, [this](){
        historyAdd("Connected", "", "", "", "");
        setConnectEnable(true);
        ui->status->setText("Connected");
        ui->status->setStyleSheet("QLineEdit{color:green;background:transparent;}");
    });
    connect(m_client, &MqttSDK::disconnected, [this](){
        historyAdd("Disconnected", "", "", "", "");
        setConnectEnable(false);
        ui->status->setText("Disconnected");
        ui->status->setStyleSheet("QLineEdit{background:transparent;}");
    });

    connect(m_client, &MqttSDK::received, [this](const QMQTT::Message& message){
        historyAdd("Received", message.topic(), message.payload(), QString("%1").arg(message.qos()), message.retain() ? "Yes" : "No");
    });
}

MqttSDKWidget::~MqttSDKWidget()
{
    delete ui;
}

void MqttSDKWidget::setConnectEnable(bool flag)
{
    ui->url->setDisabled(flag);
    ui->sk->setDisabled(flag);
    ui->skBrowse->setDisabled(flag);
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

void MqttSDKWidget::historyAdd(QString event, QString topic, QString msg, QString qos, QString retain)
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
