#include "seloutfile.h"
#include "ui_seloutfile.h"

SelOutFile::SelOutFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelOutFile)
{
	ui->setupUi(this);
}

SelOutFile::~SelOutFile()
{
	delete ui;
}
