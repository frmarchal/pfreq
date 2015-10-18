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
	ui->Version->setText(QStringLiteral("v%1.%2").arg(Version).arg(Revision,2,10,QChar('0')));
}
