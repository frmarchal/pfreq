#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
}

About::~About()
{
    delete ui;
}

void About::Version(int Version,int Revision)
{
    QString Text = QString::asprintf("v%d.%02d",Version,Revision);
    ui->Version->setText(Text);
}
