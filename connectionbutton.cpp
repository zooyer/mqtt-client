#include "connectionbutton.h"
#include <QDebug>

ConnectionButton::ConnectionButton(QWidget *parent) : QPushButton(parent)
{
    m_menu = new QMenu(this);
    m_new = new QAction(QIcon(":/image/add.gif"), tr("New Connection"), m_menu);
    m_delete = new QAction(QIcon(":/image/remove.gif"), tr("Delete"), m_menu);
    m_auth = new QAction(QIcon(":/image/auth.gif"), tr("New Auth Connection"), m_menu);

    m_menu->addAction(m_new);
    m_menu->addAction(m_auth);
    m_menu->addAction(m_delete);

    connect(m_delete, &QAction::triggered, this, &ConnectionButton::deleteLater);
    connect(m_new, &QAction::triggered, [this](){
        emit addConnection(true);
    });
    connect(m_auth, &QAction::triggered, [this](){
        emit addConnection(false);
    });

    setIcon(QIcon(":/image/connection.gif"));
    m_qss += "QPushButton{background:transparent; border:0px; height:25px; text-align:left; padding-left:5px}";
    m_qss += "QPushButton::hover{background-color:rgba(200,200,200,200);}";
    m_qss += "QPushButton::checked{background-color:rgba(100,100,100,100);}";
    m_qss += "QPushButton::pressed{background-color:rgba(100,100,100,100);}";
    setStyleSheet(m_qss);

    m_line = new LineEdit(this);
    m_line->hide();
    connect(m_line, &LineEdit::finish, [this](QString text){
        setText(text);
    });
}

ConnectionButton::~ConnectionButton()
{
    qDebug() << "the connection button distroy.";
    if (m_delete != nullptr)
        delete m_delete;
    if (m_new != nullptr)
        delete m_new;
    if (m_auth != nullptr)
        delete m_auth;
    if (m_menu != nullptr)
        delete m_menu;
    if (m_widget != nullptr)
        delete m_widget;
    if (m_line != nullptr)
        delete m_line;
}

void ConnectionButton::setText(const QString &text)
{
    emit namedChanged();
    QPushButton::setText(text);
}

void ConnectionButton::setWidget(QWidget *widget)
{
    m_widget = widget;
}

QWidget *ConnectionButton::getWidget()
{
    return m_widget;
}

void ConnectionButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "the button in mouse double click event:" << event->button();
    if (event->button() == Qt::LeftButton) {
        qDebug() << "the double click button is left.";
        m_line->setText(text());
        m_line->setGeometry(0, 0, width(), height());
        m_line->show();
        m_line->setFocus();
    }

    QPushButton::mouseDoubleClickEvent(event);
}

void ConnectionButton::mouseReleaseEvent(QMouseEvent *e)
{
    qDebug() << "the button in mouse release event:" << e->button();
    if (e->button() == Qt::RightButton) {
        qDebug() << "the release button is right.";
        m_menu->move(e->globalPos());
        m_menu->show();
        return;
    }

    QPushButton::mouseReleaseEvent(e);
}
