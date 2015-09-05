#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Settings)
{
	ui->setupUi(this);
}

Settings::~Settings()
{
	delete ui;
}

void Settings::SetDecimalDot(bool UseDot)
{
	ui->DecimalSeparator->setChecked(UseDot);
}

bool Settings::GetDecimalDot()
{
	return(ui->DecimalSeparator->isChecked());
}
