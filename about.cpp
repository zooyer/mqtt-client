#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint);
    connect(ui->okButton, &QPushButton::clicked, this, &About::close);
}

About::~About()
{
    delete ui;
}
