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
    explicit MessageViewer(QWidget *parent = nullptr);
    ~MessageViewer();
    void setEvent(QString text);
    void setTopic(QString text);
    void setMessage(QString text);
    void setQos(QString text);
    void setRetained(QString text);
    void setTime(QString text);

private:
    Ui::MessageViewer *ui;
};

#endif // MESSAGEVIEWER_H
