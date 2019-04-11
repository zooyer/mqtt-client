#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        translator.load(QString(":/i18n/zh_cn.qm"));
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
