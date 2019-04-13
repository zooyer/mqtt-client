#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "connectionbutton.h"
#include "connectionwidget.h"
#include "mqttwidget.h"

#include <QComboBox>
#include <QDialog>
#include <QDebug>
#include <QTranslator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_id = 0;
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
    m_menu = new QMenu(this);
    m_new = new QAction(QIcon(":/image/add.gif"), tr("New Connection"), m_menu);
    m_auth = new QAction(QIcon(":/image/auth.gif"), tr("New Auth Connection"), m_menu);

    m_menu->addAction(m_new);
    m_menu->addAction(m_auth);

    ui->language->hide();
    ui->languageTitle->hide();

    connect(ui->language, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [](int index){
        qDebug() << "language changed:" << index;
        static QTranslator translator;
        QLocale locale;
        switch (static_cast<Language>(index)) {
        case Chinese:
            qDebug() << "chinese....";
            translator.load(QString(":/i18n/zh_cn.qm"));
            qApp->installTranslator(&translator);
            break;
        case English:
            qDebug() << "english....";
            translator.load(QString(":/i18n/en_us.qm"));
            qApp->installTranslator(&translator);
            break;
        }
    });
    //connect(ui, &MainWind)

    connect(m_new, &QAction::triggered, [this](){
        newConnection(true);
    });
    connect(m_auth, &QAction::triggered, [this](){
        newConnection(false);
    });

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
//    if (ui->title->parentWidget() != ui->scrollArea) {
//        ui->title->setParent(ui->scrollArea);
//    }
    ConnectionButton *conn = new ConnectionButton(ui->serverButtons);
    conn->setCheckable(true);
    conn->setText(QString("Connection %1").arg(++m_id));
    m_group->addButton(conn);
    ui->serversLayout->insertWidget(ui->serversLayout->count() - 1, conn);

    QWidget *w;

    if (isNative) {
        w = new ConnectionWidget();
    } else {
        w = new MqttWidget();
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

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        if (ui->serverButtons->rect().contains(e->pos())) {
            m_menu->exec(QCursor::pos());
        }
    }

    QMainWindow::mouseReleaseEvent(e);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "in chagne event...";
        ui->retranslateUi(this);
        return;
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::setCurrentLanguage(const MainWindow::Language &lang)
{
    ui->language->setCurrentIndex(lang);
}
