#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>


class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QString text, QWidget *parent = nullptr);
    ~LineEdit();

    void focusOutEvent(QFocusEvent *);

signals:
    void finish(QString text);

public slots:

private:
    void notifyDelete();
};

#endif // LINEEDIT_H
