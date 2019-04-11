#include "lineedit.h"

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this, &LineEdit::returnPressed, this, [this](){
        focusOutEvent(nullptr);
    });
}

void LineEdit::focusOutEvent(QFocusEvent * e)
{
    emit finish(text());
    hide();

    if (e != nullptr)
        QLineEdit::focusOutEvent(e);
}
