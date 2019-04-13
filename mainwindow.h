#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMenu>
#include <QMainWindow>
#include <QButtonGroup>
#include <QMouseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Language
    {
        English = 0,
        Chinese
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int           m_id;
    QButtonGroup *m_group;
    QMenu        *m_menu;
    QAction      *m_new;
    QAction      *m_auth;

public slots:
    void newConnection(bool isNative);
    void mouseReleaseEvent(QMouseEvent *e);
    void changeEvent(QEvent *event);
    void setCurrentLanguage(const Language &lang);
};

#endif // MAINWINDOW_H
