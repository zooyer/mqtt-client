#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = nullptr);
    void focusOutEvent(QFocusEvent *e);

signals:
    void finish(QString text);

public slots:
};

#endif // LINEEDIT_H
