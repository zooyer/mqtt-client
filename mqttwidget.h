#ifndef MQTTWIDGET_H
#define MQTTWIDGET_H

#include <QWidget>
#include <QMenu>
//#include <QtMqtt/QMqttClient>
#include <QFileDialog>
#include "tabledelegate.h"
#include "mqttexception.h"
#include "abstractmqtt.h"

namespace Ui {
class MqttWidget;
}

struct MqttParams
{
    QString server;
    QString deviceSK;
    QString deviceID;
};

class MqttRequest;
class MqttResponse;
class MqttWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MqttWidget(QWidget *parent = nullptr);
    ~MqttWidget();

    void initConnect();
    void initMqttEvents();
    void initStatus();
    void initTabOrder();
    void setOnlineStatus(bool flag);
    void addHistory(QString event, QString topic, QString msg, QString qos, QString retain);

private:
    MqttResponse doRegister(MqttRequest request) noexcept(false);

private:
    Ui::MqttWidget *ui;
    QMenu        *m_menu;
    QAction      *m_clear;
    TopicModel   *m_model;
    //QMqttClient  *m_client;
    AbstractMqtt *m_client;
    MqttParams   *m_params;
    MqttRequest  *m_request;
    MqttResponse *m_response;
};

#endif // MQTTWIDGET_H
