#include "messageviewer.h"
#include "ui_messageviewer.h"

MessageViewer::MessageViewer(QString event, QString topic, QString &msg, QString qos, QString retained, QString time, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageViewer)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    ui->event->setText(event);
    ui->topic->setText(topic);
    ui->message->document()->setPlainText(msg);
    ui->qos->setText(qos);
    ui->retained->setText(retained);
    ui->time->setText(time);


    setStyleSheet("QPlainTextEdit{background:transparent;}QLineEdit{background:transparent;}");
    //setStyleSheet("background:transparent;");
}

MessageViewer::~MessageViewer()
{
    delete ui;
}
