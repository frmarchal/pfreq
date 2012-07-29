#include <qfiledialog.h>
#include <qfileinfo.h>
#include "seloutfile.h"
#include "ui_seloutfile.h"
#include "config.h"

extern ConfigObject *ConfigFile;

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
SelOutFile::SelOutFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelOutFile)
{
	ui->setupUi(this);

	QFileInfo SmoothFile=ConfigFile->Config_GetFileName("Output","Smooth","");
	ui->SmoothName->setText(SmoothFile.filePath());
	QFileInfo DeriveFile=ConfigFile->Config_GetFileName("Output","Derivative","");
	ui->DeriveName->setText(DeriveFile.filePath());
}

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
SelOutFile::~SelOutFile()
{
	if (result()==QDialog::Accepted)
	{
		ConfigFile->Config_WriteFileName("Output","Smooth",ui->SmoothName->text());
		ConfigFile->Config_WriteFileName("Output","Derivative",ui->DeriveName->text());
	}
	delete ui;
}

/*=============================================================================*/
/*!
  The user want to select the output file for the derivative.

  \date 2001-12-01
 */
/*=============================================================================*/
void SelOutFile::on_BrowseDerv_clicked()
{
	QString FileName=QFileDialog::getSaveFileName(this, tr("Derivative File"),
												  ui->DeriveName->text(),
												  tr("Text (*.txt);All (*.*)"));
	if (!FileName.isEmpty())
	{
		ui->DeriveName->setText(FileName);
	}
}

/*=============================================================================*/
/*!
  The user want to select the output file for the smoothed curve.

  \date 2001-12-01
 */
/*=============================================================================*/
void SelOutFile::on_BrowseSmooth_clicked()
{
	QString FileName=QFileDialog::getSaveFileName(this, tr("Smooth File"),
												  ui->SmoothName->text(),
												  tr("Text (*.txt);All (*.*)"));
	if (!FileName.isEmpty())
	{
		ui->SmoothName->setText(FileName);
	}
}
