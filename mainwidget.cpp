#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "connectionwidget.h"
#include "connectionbutton.h"
#include "mqttsdkwidget.h"

#include <QVBoxLayout>
//#include <QtMqtt/QMqttClient>
#include <QDebug>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    m_connectCount = 0;

    connect(ui->add, &QPushButton::clicked, [=](){
        ConnectionWidget *widget = new ConnectionWidget();
        addConnectButton(widget);
        return;
        ConnectionButton *btn = new ConnectionButton(widget, ui->scrollAreaWidgetContents);
        widget->setParent(btn);
        btn->setText(QString("Connection %1").arg(++m_connectCount));
        ui->verticalLayout->insertWidget(ui->verticalLayout->count() - 1, btn);
        btn->m_widget->setParent(ui->connWidget);
        ui->verticalLayout_3->addWidget(btn->m_widget);
        btn->m_widget->hide();

        connect(btn, &ConnectionButton::clicked, this, [=](){
            QObjectList list = ui->connWidget->children();
            qDebug() << "children num:" << list.length();
            for (int i=1; i<list.length(); i++)
                //((QWidget*)list[i])->setParent(nullptr);
                ((QWidget*)list[i])->hide();
            btn->m_widget->setParent(ui->connWidget);
            btn->m_widget->show();
        });

    });

    connect(ui->addSDK, &QPushButton::clicked, [=](){
        MqttSDKWidget *widget = new MqttSDKWidget();
        addConnectButton(widget);
    });


    return;


/*
    QMqttClient *m_client = new QMqttClient(this);
    m_client->setHostname("127.0.0.1");
    m_client->setPort(1883);
    m_client->setClientId("789");
    m_client->setCleanSession(true);

    m_client->setProtocolVersion(QMqttClient::MQTT_3_1_1);


    connect(m_client, &QMqttClient::disconnected, this, [this](){
        qDebug() << "disconnected!";
    });

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic){
        qDebug() << "TOPIC:" << topic.name();
        qDebug() << "MSG:" << QString(message);
    });


    qDebug() << m_client->state();
    if (m_client->state() == QMqttClient::Disconnected) {
        m_client->connectToHost();
    } else {
        m_client->disconnectFromHost();
    }

    qDebug() << m_client->state();

    m_client->subscribe(QMqttTopicFilter("#"));

    m_client->publish(QMqttTopicName("aaa"), QByteArray("hello"), 2, false);


    connect(ui->add, &QPushButton::clicked, this, [=](){
        if (m_client->publish(QMqttTopicName("aaa"), QByteArray("hello"), 2, false) == -1)
            qDebug() << m_client->error();
    });


    ConnectionWidget *c = new ConnectionWidget(ui->connWidget);
    //layout->addWidget(c);
    //ui->connWidget->setLayout(layout);

    c->show();

    */
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::addConnectButton(QWidget *widget)
{
    ConnectionButton *btn = new ConnectionButton(widget, ui->scrollAreaWidgetContents);
    widget->setParent(btn);
    btn->setText(QString("Connection %1").arg(++m_connectCount));
    ui->verticalLayout->insertWidget(ui->verticalLayout->count() - 1, btn);
    btn->m_widget->setParent(ui->connWidget);
    ui->verticalLayout_3->addWidget(btn->m_widget);
    btn->m_widget->hide();

    connect(btn, &ConnectionButton::clicked, this, [=](){
        QObjectList list = ui->connWidget->children();
        qDebug() << "children num:" << list.length();
        for (int i=1; i<list.length(); i++)
            //((QWidget*)list[i])->setParent(nullptr);
            ((QWidget*)list[i])->hide();
        btn->m_widget->setParent(ui->connWidget);
        btn->m_widget->show();
    });
}
