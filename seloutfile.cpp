#include <qfiledialog.h>
#include "seloutfile.h"
#include "ui_seloutfile.h"
#include "config.h"

extern ConfigObject *ConfigFile;

/*=============================================================================*/
/*!
  Select the output file name.

  \param Parent The parent form.
 */
/*=============================================================================*/
void SelectOutputFile(QWidget *Parent)
{
	SelOutFile Form(Parent);

	int Result=Form.exec();

	if (Result==QDialog::Accepted)
	{
		ConfigFile->Config_WriteString("Output","Smooth",Form.SmoothFile);
		ConfigFile->Config_WriteString("Output","Derivative",Form.DeriveFile);
	}
}

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
SelOutFile::SelOutFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelOutFile)
{
	ui->setupUi(this);

	SmoothFile=ConfigFile->Config_GetString("Output","Smooth","");
	ui->SmoothName->setText(SmoothFile);
	DeriveFile=ConfigFile->Config_GetString("Output","Derivative","");
	ui->DeriveName->setText(DeriveFile);
}

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
SelOutFile::~SelOutFile()
{
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
												  DeriveFile,
												  tr("Text (*.txt);All (*.*)"));
	if (!FileName.isEmpty())
	{
		DeriveFile=FileName;
		ui->DeriveName->setText(DeriveFile);
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
												  SmoothFile,
												  tr("Text (*.txt);All (*.*)"));
	if (!FileName.isEmpty())
	{
		SmoothFile=FileName;
		ui->SmoothName->setText(SmoothFile);
	}
}

