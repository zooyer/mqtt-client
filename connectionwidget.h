#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QMenu>
#include <string>
//#include <QtMqtt/QMqttClient>
#include <qmqtt>


//#include "mqtt/async_client.h"
// //#pragma comment(lib,"kernel32.lib")
//#pragma comment(lib, "advapi32.lib")
//#pragma comment(lib,"user32.lib")
//#pragma comment(lib,"gdi32.lib")
// //#pragma comment(lib,"winspool.lib")

//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"libeay32.lib")
//#pragma comment(lib,"ssleay32.lib")
//#pragma comment(lib,"paho-mqtt3as.lib")
//#pragma comment(lib,"paho-mqttpp3-static.lib")






using namespace std;

namespace Ui {
class ConnectionWidget;
}

class ConnectionWidget : public QWidget //, public mqtt::callback
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget *parent = 0);
    ~ConnectionWidget();

//    virtual void connected(const string& cause);
//    virtual void connection_lost(const string& cause);
//    virtual void message_arrived(mqtt::const_message_ptr msg);
//    virtual void delivery_complete(mqtt::deliveryTokenPtr tok);

private:
    Ui::ConnectionWidget *ui;
    //QMqttClient *m_client;
    //mqtt::async_client *m_client;
    QMQTT::Client *m_client;

private:
    QMenu *m_menu;
    QAction *m_action;
    QSslConfiguration *m_ssl;

    void setConnectEnable(bool flag);
    void setLoginEnable(bool flag);
    void setPersistenceEnable(bool flag);
    void setSSLEnable(bool flag);
    void setHAEnable(bool flag);
    void setLWTEnable(bool flag);
    void historyAdd(QString event, QString topic, QString msg, QString qos, QString retain);
};

#endif // CONNECTIONWIDGET_H
