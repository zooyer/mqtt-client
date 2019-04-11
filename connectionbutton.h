#ifndef CONNECTIONBUTTON_H
#define CONNECTIONBUTTON_H

#include <QPushButton>
#include <QMouseEvent>
#include <QMenu>
#include "lineedit.h"

class ConnectionButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ConnectionButton(QWidget *parent = nullptr);
    ~ConnectionButton();
    void setText(const QString &text);
    void setWidget(QWidget *widget);
    QWidget* getWidget();

private:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *e);

signals:
    void addConnection(bool isNative);
    void namedChanged();

public slots:

private:
    QWidget  *m_widget;
    LineEdit *m_line;
    QString   m_qss;
    QMenu    *m_menu;
    QAction  *m_delete;
    QAction  *m_new;
    QAction  *m_auth;
};

#endif // CONNECTIONBUTTON_H
