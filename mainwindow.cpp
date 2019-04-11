#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "connectionbutton.h"
#include "connectionwidget.h"
#include "mqttwidget.h"

#include <QDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_id = 0;
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    connect(ui->actionabout, &QAction::triggered, [this](){
        qDebug() << "clicked about button, will enter about widget.";
        About about(this);
        about.exec();
    });
    connect(ui->actionexit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionnew_connect, &QAction::triggered, [this](){
        newConnection(true);
    });
    connect(ui->authAction, &QAction::triggered, [this](){
        newConnection(false);
    });
    connect(ui->authButton, &QPushButton::clicked, [this](){
        newConnection(false);
    });


    connect(ui->newConnButton, &QPushButton::clicked, this, [this](){
        newConnection(true);
    });
    connect(ui->connWidget, &QTabWidget::tabCloseRequested, [this](int index){
        qDebug() << "widget" << index << "will closed!";
        ui->connWidget->removeTab(index);
    });
}

MainWindow::~MainWindow()
{
    if (m_group != nullptr)
        delete m_group;
    delete ui;
}

void MainWindow::newConnection(bool isNative)
{
    ConnectionButton *conn = new ConnectionButton(ui->serverButtons);
    conn->setCheckable(true);
    conn->setText(QString("Connection %1").arg(++m_id));
    m_group->addButton(conn);
    ui->serversLayout->insertWidget(ui->serversLayout->count() - 1, conn);

    QWidget *w;

    if (isNative) {
        w = new ConnectionWidget(conn);
    } else {
        w = new MqttWidget(conn);
    }

    conn->setWidget(w);

    connect(conn, &ConnectionButton::addConnection, this, &MainWindow::newConnection);

    connect(conn, &ConnectionButton::namedChanged, [this, conn](){
        qDebug() << "connection button " << conn << " name changed:" << conn->text();
        int index = ui->connWidget->indexOf(conn->getWidget());
        if (index >= 0) {
            ui->connWidget->setTabText(index, conn->text());
        }
    });

    connect(conn, &ConnectionButton::clicked, [this, conn](){
        qDebug() << "clicked connection button:" << conn;
        int index = ui->connWidget->indexOf(conn->getWidget());
        if (index < 0) {
            qDebug() << "not found widget in tabwidget, will add to tabwidget.";
            ui->connWidget->addTab(conn->getWidget(), QIcon(":/image/connection.gif"), conn->text());
            ui->connWidget->setCurrentWidget(conn->getWidget());
        } else {
            qDebug() << "found widget in tabwidget, index:" << index;
            ui->connWidget->setCurrentIndex(ui->connWidget->indexOf(conn->getWidget()));
        }

        conn->setChecked(true);
    });

    conn->clicked();
}
