#include "xrange.h"
#include "ui_xrange.h"

XRange::XRange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XRange)
{
	ui->setupUi(this);
}

XRange::~XRange()
{
	delete ui;
}
