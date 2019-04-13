#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "logger.h"

int main(int argc, char *argv[])
{
#ifndef _DEBUG
    qInstallMessageHandler(Logger::getInstance()->handler());
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QTranslator translator;
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        translator.load(QString(":/i18n/zh_cn.qm"));
        a.installTranslator(&translator);
        w.setCurrentLanguage(MainWindow::Chinese);
    }

    return a.exec();
}
