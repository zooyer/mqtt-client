#include "messageviewer.h"
#include "ui_messageviewer.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MessageViewer::MessageViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageViewer)
{
    ui->setupUi(this);

    connect(ui->close, &QPushButton::clicked, this, &MessageViewer::close);
    connect(ui->save, &QPushButton::clicked, [this](){
        qDebug() << "clicked save button, this message will save to file.";
        QString filename = QFileDialog::getSaveFileName(this, tr("Save As"), "", tr("Text Files(*.txt);;All Files(*.*)"));
        if (!filename.isNull()) {
            QFile file(filename);
            if (!file.open(QFile::WriteOnly)) {
                qWarning() << "the" << filename << "open error:" << file.errorString();
                QMessageBox::critical(this, tr("Save error"), file.errorString());
                return;
            }
            if (file.write(ui->message->toPlainText().toLocal8Bit()) < 0) {
                qWarning() << "the" << filename << "write error:" << file.errorString();
                QMessageBox::critical(this, tr("Save error"), file.errorString());
            }
            file.close();
        }
    });
}

MessageViewer::~MessageViewer()
{
    delete ui;
}

void MessageViewer::setEvent(QString text)
{
    ui->event->setText(text);
}

void MessageViewer::setTopic(QString text)
{
    ui->topic->setText(text);
}

void MessageViewer::setMessage(QString text)
{
    ui->message->document()->setPlainText(text);
}

void MessageViewer::setQos(QString text)
{
    ui->qos->setText(text);
}

void MessageViewer::setRetained(QString text)
{
    ui->retained->setText(text);
}

void MessageViewer::setTime(QString text)
{
    ui->time->setText(text);
}
