#include "connectionbutton.h"
#include "lineedit.h"
#include <QDebug>
#include <QEvent>

ConnectionButton::ConnectionButton(QWidget *widget, QWidget *parent) : QPushButton(parent)
{
    m_widget = widget;
    m_menu = new QMenu(this);
    m_action = new QAction("Delete", m_menu);

    m_menu->addAction(m_action);
    connect(m_action, &QAction::triggered, this, [=](){
        deleteLater();
    });
    qDebug() << "create connection button!";
}

ConnectionButton::~ConnectionButton()
{
    if (m_widget != nullptr)
        delete m_widget;
    if (m_action != nullptr)
        delete m_action;
    if (m_menu != nullptr)
        delete m_menu;

    qDebug() << "delete connection button!";
}

void ConnectionButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        return;

    qDebug() << "in left-key double";
    LineEdit *line = new LineEdit(text(), this);
    line->setGeometry(0, 0, width(), height());
    line->setFocus();

    connect(line, &LineEdit::finish, [=](QString text){
        this->setText(text);
    });

    QPushButton::mouseDoubleClickEvent(event);
}

void ConnectionButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        m_menu->move(e->globalPos());
        m_menu->show();
    }
    QPushButton::mouseReleaseEvent(e);
}
