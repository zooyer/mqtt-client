#ifndef MQTTSDKWIDGET_H
#define MQTTSDKWIDGET_H

#include <QMenu>
#include <QWidget>
#include "mqttsdk.h"

namespace Ui {
class MqttSDKWidget;
}

class MqttSDKWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MqttSDKWidget(QWidget *parent = 0);
    ~MqttSDKWidget();

    void setConnectEnable(bool flag);

private:
    Ui::MqttSDKWidget *ui;
    MqttSDK *m_client;

    void historyAdd(QString event, QString topic, QString msg, QString qos, QString retain);

private:
    QMenu *m_menu;
    QAction *m_action;
};

#endif // MQTTSDKWIDGET_H
