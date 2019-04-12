#include "license.h"
#include "ui_license.h"
#include <QFile>
#include <QDebug>

License::License(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::License)
{
    ui->setupUi(this);

    QFile file(":/LICENSE");
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "open file error:" << file.errorString();
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    ui->edit->document()->setPlainText(data);
    QTextCursor cursor = ui->edit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ui->edit->setTextCursor(cursor);
}

License::~License()
{
    delete ui;
}
