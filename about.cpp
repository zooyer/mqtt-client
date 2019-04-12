#include "about.h"
#include "ui_about.h"
#include "license.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint);
    connect(ui->okButton, &QPushButton::clicked, this, &About::close);
    connect(ui->license, &QPushButton::clicked, this, [this](){
        License l(this);
        l.exec();
    });
}

About::~About()
{
    delete ui;
}
