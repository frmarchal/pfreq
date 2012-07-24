#include "mainscreen.h"
#include "ui_mainscreen.h"
#include "GaussSmth.h"
#include "Utils.h"
//#include "SavGol.h"
//#include "SelOutFile.h"
//#include "background.h"
//#include "selcolumn.h"
//#include "xrange.h"
#include "config.h"

#define PROG_VERSION 1
#define PROG_REVISION 6

//! Maximum number of columns in the file to load.
#define MAX_COLUMN 30

extern ConfigObject *ConfigFile;

/*==========================================================================*/
/*!
  Constructor of the class. It loads the last settings saved the last time
  the program was used.

  \date
	\arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
MainScreen::MainScreen(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainScreen)
{
    ui->setupUi(this);

	QString Text;
	int Selection;

	//***** initialize variables *****
	NPoints=0;
	XData=NULL;
	YData=NULL;
	Smooth=NULL;
	Derive=NULL;
	XPlot=NULL;
	YPlot=NULL;
	YSmooth=NULL;
	YDerv=NULL;
	LastTrackSrc=true;
	ExitProgram=false;

	Text.sprintf("PFreq v%d.%02d",PROG_VERSION,PROG_REVISION);
	ui->ProgVersionLabel->setText(Text);

	DefaultFileName=ConfigFile->Config_GetFileName("Input","FileName","*");

	XFreq=ConfigFile->Config_GetDouble("Axis","XFreq",250.);
	Text.sprintf("%.4lf",XFreq);
	ui->XFrequency->setText(Text);
	Time0=ConfigFile->Config_GetDouble("Axis","XTime0",-20.);
	Text.sprintf("%.4lf",Time0);
	ui->XTime0->setText(Text);

	YGain=ConfigFile->Config_GetDouble("Axis","YGain",1.);
	Text.sprintf("%lf",YGain);
	ui->YGainCtrl->setText(Text);
	YOffset=ConfigFile->Config_GetDouble("Axis","YOffset",0.);
	Text.sprintf("%lf",YOffset);
	ui->YOffsetCtrl->setText(Text);

	GaussWidth=ConfigFile->Config_GetDouble("Gauss","Width",10.);
	Text.sprintf("%.3lf",GaussWidth);
	ui->GaussWidthCtrl->setText(Text);
	GaussNeigh=ConfigFile->Config_GetInt("Gauss","Neigh",50);
	Text.sprintf("%d",GaussNeigh);
	ui->GaussNeighCtrl->setText(Text);
	LastGWidth=-1.;
	LastGNeigh=-1;

	Selection=ConfigFile->Config_GetInt("Derive","Mode",0);
	if (Selection==0)
		ui->RawSmoothButton->setChecked(true);
	else
		ui->SavGolButton->setChecked(true);
	SavGolPoly=ConfigFile->Config_GetInt("Derive","Poly",2);
	Text.sprintf("%d",SavGolPoly);
	ui->SavGolPolyCtrl->setText(Text);
	SavGolNeigh=ConfigFile->Config_GetInt("Derive","Neigh",50);
	Text.sprintf("%d",SavGolNeigh);
	ui->SavGolNeighCtrl->setText(Text);
	LastSGPoly=-1.;
	LastSGNeigh=-1;

	//Top=0;
	//Left=0;
	//BackgroundForm=new TBackgroundForm(this);
	//BackgroundForm->Parent=this;
}

/*==========================================================================*/
/*!
  Destructor of the class. It cleans up the memory.

  \date
	\arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
MainScreen::~MainScreen()
{
	int Selection;

	/*sprintf(Text,"%lf",XFreq);
	WritePrivateProfileString("Axis","XFreq",Text,ConfigFile);
	if (!HasBeenCut)
	 {
	 sprintf(Text,"%lf",Time0);
	 WritePrivateProfileString("Axis","XTime0",Text,ConfigFile);
	 }*/

	ConfigFile->Config_WriteDouble("Axis","YGain",YGain);
	ConfigFile->Config_WriteDouble("Axis","YOffset",YOffset);

	ConfigFile->Config_WriteDouble("Gauss","Width",GaussWidth);
	ConfigFile->Config_WriteDouble("Gauss","Neigh",GaussNeigh);

	if (ui->RawSmoothButton->isChecked())
		Selection=0;
	else
		Selection=1;
	ConfigFile->Config_WriteInt("Derive","Mode",Selection);
	ConfigFile->Config_WriteInt("Derive","Poly",SavGolPoly);
	ConfigFile->Config_WriteInt("Derive","Neigh",SavGolNeigh);

	//delete BackgroundForm;
	Purge(Derive);
	Purge(Smooth);
	Purge(XData);
	Purge(YData);
	Purge(XPlot);
	Purge(YPlot);
	Purge(YSmooth);
	Purge(YDerv);
	delete ui;
}

/*==========================================================================*/
/*!
  The user wants to exit the program.

  \date
	\arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_ExitMenu_triggered()
{
	close();
}
