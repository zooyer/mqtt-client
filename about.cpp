#include "about.h"
#include "ui_about.h"
#include "license.h"
#include <QUrl>
#include <QDesktopServices>

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint);
    connect(ui->okButton, &QPushButton::clicked, this, &About::close);
    connect(ui->license, &QLabel::linkActivated, this, [this](){
        License l(this);
        l.exec();
    });
    connect(ui->github, &QLabel::linkActivated, [](const QString& link){
        QDesktopServices::openUrl(QUrl(link));
    });
}

About::~About()
{
    delete ui;
}
