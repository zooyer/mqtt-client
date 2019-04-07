#ifndef MESSAGEVIEWER_H
#define MESSAGEVIEWER_H

#include <QDialog>

namespace Ui {
class MessageViewer;
}

class MessageViewer : public QDialog
{
    Q_OBJECT

public:
    explicit MessageViewer(QString event, QString topic, QString &msg, QString qos, QString retained, QString time, QWidget *parent = 0);
    ~MessageViewer();

private:
    Ui::MessageViewer *ui;
};

#endif // MESSAGEVIEWER_H
