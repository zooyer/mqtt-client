#ifndef CONNECTIONBUTTON_H
#define CONNECTIONBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QMouseEvent>
#include <QMenu>

class ConnectionButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ConnectionButton(QWidget *widget, QWidget *parent = nullptr);
    ~ConnectionButton();
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *e);

signals:

public slots:

public:
    QWidget *m_widget;

private:
    QMenu *m_menu;
    QAction *m_action;
};

#endif // CONNECTIONBUTTON_H
