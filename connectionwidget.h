#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QtMqtt/QMqttClient>
#include "tabledelegate.h"

namespace Ui {
class ConnectionWidget;
}

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget *parent = nullptr);
    void initConnect();
    void initMqttEvents();
    void initStatus();
    void initTabOrder();
    void setOnlineStatus(bool flag);
    void addHistory(QString event, QString topic, QString msg, QString qos, QString retain);
    void setLastMessage(QString topic, QString msg, QString qos, QString retain);
    ~ConnectionWidget();

private:
    Ui::ConnectionWidget *ui;
    QMenu        *m_menu;
    QAction      *m_clear;
    TopicModel   *m_model;
    QMqttClient  *m_client;
};

#endif // CONNECTIONWIDGET_H
