#include "xrange.h"
#include "ui_xrange.h"
#include "config.h"

extern ConfigObject *ConfigFile;

XRange::XRange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XRange)
{
	ui->setupUi(this);

	QString Text;

	XStart=ConfigFile->Config_GetDouble("Cut","XStart",0.);
	XStop=ConfigFile->Config_GetInt("Cut","XStop",0.);

	Text.setNum(XStart,'f',2);
	ui->XStartText->setText(Text);
	Text.setNum(XStop,'f',2);
	ui->XStopText->setText(Text);
}

XRange::~XRange()
{
	delete ui;
}

void XRange::GetRange(double *XStart, double *XStop)
{
	if (XStart) *XStart=ui->XStartText->text().toDouble();
	if (XStop) *XStop=ui->XStopText->text().toDouble();
}
