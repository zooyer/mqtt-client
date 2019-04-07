#include "lineedit.h"
#include <QDebug>

LineEdit::LineEdit(QString text, QWidget *parent) : QLineEdit(parent)
{
    this->setText(text);
    this->show();
    connect(this, &LineEdit::returnPressed, [this](){
        notifyDelete();
    });
    qDebug() << "create LineEdit!";
}

LineEdit::~LineEdit()
{
    qDebug() << "delete LineEdit!";
}

void LineEdit::focusOutEvent(QFocusEvent *)
{
    notifyDelete();
}

void LineEdit::notifyDelete()
{
    emit finish(text());
    delete this;
}

