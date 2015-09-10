#include <qfile.h>
#include <qelapsedtimer.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qclipboard.h>
#include <qtimer.h>
#include <math.h>
#include "mainscreen.h"
#include "ui_mainscreen.h"
#include "GaussSmth.h"
#include "Utils.h"
#include "Savgol.h"
#include "seloutfile.h"
#include "background.h"
#include "selectcolumn.h"
#include "xrange.h"
#include "config.h"
#include "about.h"
#include "selectcolors.h"
#include "settings.h"

#define PROG_VERSION 2
#define PROG_REVISION 4

//! Maximum number of columns in the file to load.
#define MAX_COLUMN 30

MainScreen *MainForm=NULL;

extern ConfigObject *ConfigFile;

extern BackgroundForm *BgForm;

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
	MainForm=this;

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

	DefaultFileName=ConfigFile->Config_GetFileName("Input","FileName","*");
	NumbersWithDot=(ConfigFile->Config_GetInt("Settings","NumbersWithDot",0)!=0);

	XFreq=ConfigFile->Config_GetDouble("Axis","XFreq",250.);
	DisplayXFreq();
	Time0=ConfigFile->Config_GetDouble("Axis","XTime0",-20.);
	DisplayTime0();
	XGain=ConfigFile->Config_GetDouble("Axis","XGain",1.);
	DisplayXGain();

	YGain=ConfigFile->Config_GetDouble("Axis","YGain",1.);
	Text.sprintf("%lf",YGain);
	ui->YGainCtrl->setText(Text);
	YOffset=ConfigFile->Config_GetDouble("Axis","YOffset",0.);
	Text.sprintf("%lf",YOffset);
	ui->YOffsetCtrl->setText(Text);

	int SmoothAlgo=ConfigFile->Config_GetInt("Smoothing","Algorithm",0);
	ui->SmoothTab->setCurrentIndex(SmoothAlgo);

	GaussWidth=ConfigFile->Config_GetDouble("Gauss","Width",10.);
	Text.sprintf("%.3lf",GaussWidth);
	ui->GaussWidthCtrl->setText(Text);
	GaussNeigh=ConfigFile->Config_GetInt("Gauss","Neigh",50);
	Text.sprintf("%d",GaussNeigh);
	ui->GaussNeighCtrl->setText(Text);
	LastGWidth=-1.;
	LastGNeigh=-1;

	SavGolSmoothPoly=ConfigFile->Config_GetInt("SavGolSmooth","Degree",4);
	Text.sprintf("%d",SavGolSmoothPoly);
	ui->SavGolSPolyCtrl->setText(Text);
	SavGolSmoothNeigh=ConfigFile->Config_GetInt("SavGolSmooth","Neigh",50);
	Text.sprintf("%d",SavGolSmoothNeigh);
	ui->SavGolSNeighCtrl->setText(Text);
	LastSGSPoly=-1.;
	LastSGSNeigh=-1;

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

	ui->ViewPanelsMenu->addAction(ui->XAxisDock->toggleViewAction());
	ui->ViewPanelsMenu->addAction(ui->YAxisDock->toggleViewAction());
	ui->ViewPanelsMenu->addAction(ui->TrackDock->toggleViewAction());
	ui->ViewPanelsMenu->addAction(ui->SmoothDock->toggleViewAction());
	ui->ViewPanelsMenu->addAction(ui->DeriveDock->toggleViewAction());

	restoreGeometry(ConfigFile->Config_GetBytes("Window","Geometry",QByteArray()));
	restoreState(ConfigFile->Config_GetBytes("Window","State",QByteArray()));

	ConfigureColors();

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
	ConfigFile->Config_WriteInt("Gauss","Neigh",GaussNeigh);

	ConfigFile->Config_WriteInt("SavGolSmooth","Degree",SavGolSmoothPoly);
	ConfigFile->Config_WriteInt("SavGolSmooth","Neigh",SavGolSmoothNeigh);

	ConfigFile->Config_WriteInt("Smoothing","Algorithm",ui->SmoothTab->currentIndex());

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
  Function called when the window is closed.
 */
/*==========================================================================*/
void MainScreen::closeEvent(QCloseEvent *event)
{
	ConfigFile->Config_Write("Window","Geometry",saveGeometry());
	ConfigFile->Config_Write("Window","State",saveState());
	QMainWindow::closeEvent(event);
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

/*==========================================================================*/
/*!
  The user wants to load a file.

  \date
	\arg 2003-03-24 created by Frederic
 */
/*==========================================================================*/
bool MainScreen::AddDataPoint(char **ColumnsTxt,double **XData,double **YData,int &NPoints,int &NAllocated,
	bool &InData,int XColumn,int YColumn,int Line)
{
	double XValue,YValue,*TData;
	bool CXValid,CYValid;

	if (XColumn>=0)
	{
		XValue=0.;
		CXValid=StrToDouble(ColumnsTxt[XColumn],&XValue);
		if (!CXValid)
		{
			if (InData)
			{
				WriteMsg(__FILE__,__LINE__,tr("Column %1 contains an invalid number at line %2").arg(XColumn).arg(Line));
				return(false);
			}
			return(true);
		}
	}
	YValue=0.;
	CYValid=StrToDouble(ColumnsTxt[YColumn],&YValue);
	if (!CYValid)
	{
		if (InData)
		{
			WriteMsg(__FILE__,__LINE__,tr("Column %1 contains an invalid number at line %2").arg(YColumn).arg(Line));
			return(false);
		}
		return(true);
	}
	InData=true;

	if (NPoints>=NAllocated)
	{
		NAllocated+=256;
		TData=(double *)realloc(*YData,NAllocated*sizeof(double));
		if (!TData)
		{
			WriteMsg(__FILE__,__LINE__,tr("Not enough memory to load all the file. Some points after line %1 are missing").arg(Line));
			return(false);
		}
		*YData=TData;
		if (XColumn>=0)
		{
			TData=(double *)realloc(*XData,NAllocated*sizeof(double));
			if (!TData)
			{
				WriteMsg(__FILE__,__LINE__,tr("Not enough memory to load all the file. Some points after line %1 are missing").arg(Line));
				return(false);
			}
			*XData=TData;
		}
	}
	if (XColumn>=0) (*XData)[NPoints]=XValue;
	(*YData)[NPoints++]=YValue;
	return(true);
}

/*==========================================================================*/
/*!
  Get the column indexes corresponding to the specified number of columns.

  \param NColumn The number of columns in the file.
  \param XColumn The index of the X column is returned in this variable.
  \param YColumn The index of the Y column is returned in this variable.

  \return \c True if the column indexes are correct or \c false if an error
  is detected.
 */
/*==========================================================================*/
bool MainScreen::GetColumns(int NColumn,int &XColumn,int &YColumn)
{
	QString Item;
	QString ColMark;

	XColumn=-1;
	YColumn=-1;
	Item=QString::number(NColumn);
	ColMark=ConfigFile->Config_GetStringNoWrite("Columns",Item,"-1,-1");
	QRegExp ColRe("^(\\d+),(\\d+)$");
	if (ColRe.indexIn(ColMark)<0) return(false);
	QStringList Match=ColRe.capturedTexts();
	bool OkX=false;
	XColumn=Match.at(1).toInt(&OkX);
	bool OkY=false;
	YColumn=Match.at(2).toInt(&OkY);
	if (!OkX) return(false);
	if (!OkY) return(false);
	return(true);
}

/*==========================================================================*/
/*!
  Decode and load a CSV file.

  \param Buffer The buffer containing the file to convert.
  \param FSize The size of the file buffer.

  \return True if the data could be parsed or false if an error occured.

  \date 2004-02-06
 */
/*==========================================================================*/
bool MainScreen::LoadCsvFile(char *Buffer,unsigned int FSize)
{
#define NSTORED_LINES 25
	char Delim;
	char *StoredColumns[NSTORED_LINES][MAX_COLUMN];
	double Value;
	int i,XColumn,YColumn;

	int Line=1;
	int NAllocated=0;
	int NColumn=0;
	int Column=0;
	int ValidColumns=0;
	bool InData=false;
	int NStoredLines=0;
	bool NonSpaces;
	bool SelectingColumns=false;
	unsigned int NRead=0;
	while (NRead<FSize)
	{
		while (NRead<FSize && (Buffer[NRead]=='\r' || Buffer[NRead]=='\n')) NRead++;
		if (Buffer[NRead]=='%' || Buffer[NRead]==';') //comment till end of line
		{
			while (NRead<FSize && Buffer[NRead]!='\r' && Buffer[NRead]!='\n') NRead++;
			continue;
		}
		Delim='\0';
		Column=0;
		ValidColumns=0;
		while (NRead<FSize && Delim!='\r' && Delim!='\n')
		{
			char *Number=Buffer+NRead;
			if (NStoredLines<NSTORED_LINES) StoredColumns[NStoredLines][Column]=Number;
			NonSpaces=false;
			for ( ; NRead<FSize && ((unsigned char)Buffer[NRead]>' ' || (Buffer[NRead]==' ' && !NonSpaces)) ; NRead++)
				if (Buffer[NRead]!=' ') NonSpaces=true;
			if (NRead>=FSize) break;
			Delim=Buffer[NRead];
			Buffer[NRead++]='\0';
			if (*Number) ValidColumns+=StrToDouble(Number,&Value) ? 1 : 0;
			Column++;
			if (Column>=MAX_COLUMN)
			{
				WriteMsg(__FILE__,__LINE__,tr("Too many columns in file. Reading aborted."));
				return(false);
			}
		}
		if (!NColumn)
		{
			if (ValidColumns>0 || Line>1)
			{
				NColumn=Column;
				if (!GetColumns(NColumn,XColumn,YColumn) || YColumn<0)
				{
					if (NColumn==1)
					{
						QString Item=QString::number(NColumn);
						ConfigFile->Config_WriteString("Columns",Item,"-1,0");
						XColumn=-1;
						YColumn=0;
					}
					else if (NColumn==2)
					{
						QString Item=QString::number(NColumn);
						ConfigFile->Config_WriteString("Columns",Item,"0,1");
						XColumn=0;
						YColumn=1;
					}
					else
					{
						SelectingColumns=true;
					}
				}
				if (XColumn>=NColumn)
				{
					WriteMsg(__FILE__,__LINE__,tr("Invalid X column for a file containing %1 columns").arg(NColumn));
					return(false);
				}
				if (YColumn>=NColumn)
				{
					WriteMsg(__FILE__,__LINE__,tr("Invalid Y column for a file containing %1 columns").arg(NColumn));
					return(false);
				}
			}
			else
			{
				SelectingColumns=true;
			}
		}
		else
		{
			if (NColumn!=Column)
			{
				if (ValidColumns>0) WriteMsg(__FILE__,__LINE__,tr("Inconsistant number of columns in the file at line %1").arg(Line));
				return(false);
			}
		}

		if (SelectingColumns)
		{
			int Result;

			if ((ValidColumns>0 || Line>1) && ++NStoredLines>=NSTORED_LINES)
			{
				SelectColumn ColSel(this);
				ColSel.PrepareList(NColumn);
				for (i=0 ; i<NStoredLines ; i++)
					ColSel.AddLine(StoredColumns[i],NColumn);
				ColSel.SetColumn(XColumn,YColumn);

				Result=ColSel.exec();
				if (Result==QDialog::Accepted)
					ColSel.GetColumn(XColumn,YColumn);
				if (Result!=QDialog::Accepted) break;
				if (YColumn<0) break;
				SelectingColumns=false;
				for (i=0 ; i<NStoredLines ; i++)
				{
					if (!AddDataPoint(StoredColumns[i],&XData,&YData,NPoints,NAllocated,InData,XColumn,YColumn,i))
						return(false);
				}
				if (i<NStoredLines)
					return(false);
				NStoredLines=0;
			}
		}
		else
		{
			if (!AddDataPoint(StoredColumns[0],&XData,&YData,NPoints,NAllocated,InData,XColumn,YColumn,Line))
				return(false);
		}
		Line++;
	}
	return(true);
}

/*=============================================================================*/
/*!
  Convert a binary file. The file is only made of float in big endian order.

  \param Buffer The buffer containing the file to convert.
  \param FSize The size of the file buffer.

  \return True if the data could be parsed or false if an error occured.

  \date 2004-01-30
 */
/*=============================================================================*/
bool MainScreen::LoadTirFile(char *Buffer,unsigned int FSize)
{
	char Swap,*Data;
	float Value;
	int i,NFilePoints;
	double *TData;

	int NAllocated=0;

	//***** convert the data *****
	NFilePoints=FSize/4;
	Data=Buffer;
	for (i=0 ; i<NFilePoints ; i++)
	{
		Swap=Data[0];
		Data[0]=Data[3];
		Data[3]=Swap;
		Swap=Data[1];
		Data[1]=Data[2];
		Data[2]=Swap;
		Value=*((float *)Data);
		Data+=4;

		if (NPoints>=NAllocated)
		{
			NAllocated+=256;
			TData=(double *)realloc(YData,NAllocated*sizeof(double));
			if (!TData)
			{
				WriteMsg(__FILE__,__LINE__,
						 tr("Not enough memory to load all the file. %1 points may be missing").arg(NFilePoints-i));
				return(false);
			}
			YData=TData;
		}
		YData[NPoints++]=Value;
	}
	return(true);
}

/*==========================================================================*/
/*!
  The user wants to load a file.

  \date
	\arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_LoadMenu_triggered()
{
	qint64 FSize,NRead;
	char *Buffer;

	//***** get file name *****
	QString FileName=QFileDialog::getOpenFileName(this, tr("Open File"),
												  DefaultFileName.filePath(),
												  tr("All (*.*)"));
	if (FileName.isEmpty()) return;
	DefaultFileName=FileName;
	ConfigFile->Config_WriteFileName("Input","FileName",DefaultFileName);

	//***** read file *****
	{
		QFile hFile(FileName);
		if (!hFile.open(QIODevice::ReadOnly))
		{
			WriteMsg(__FILE__,__LINE__,tr("Cannot open %1").arg(FileName));
			return;
		}
		FSize=hFile.size();
		if (FSize==0)
		{
			hFile.close();
			WriteMsg(__FILE__,__LINE__,tr("File is empty"));
			return;
		}
		Buffer=(char *)malloc(FSize);
		if (!Buffer)
		{
			hFile.close();
			WriteMsg(__FILE__,__LINE__,tr("Not enough memory to load the file"));
			return;
		}
		NRead=hFile.read(Buffer,FSize);
		if (NRead!=FSize)
		{
			free(Buffer);
			hFile.close();
			WriteMsg(__FILE__,__LINE__,tr("Cannot read file"));
			return;
		}
		hFile.close();
	}

	//***** parse file *****
	if (!XData && !HasBeenCut)  //if no X currently in memory, store the paramters
	{
		StoredXFreq=XFreq;
		StoredTime0=Time0;
	}
	ui->MainGraphCtrl->DeleteAllCurves();
	ui->DervGraphCtrl->DeleteAllCurves();
	NPoints=0;
	HasBeenCut=false;
	Purge(XData);
	Purge(YData);
	Purge(XPlot);
	Purge(YPlot);
	Purge(YSmooth);
	Purge(YDerv);
	setWindowTitle("MainForm "+DefaultFileName.fileName());

	//***** test the type of the file *****
	int NonText=0;
	int InvalidText=0;
	char *Ptr=Buffer;
	int Column=0;
	int ValidLine=0,ValidColumn=0;
	int NDots=0,NSigns=0,NExp=0;
	bool MultiSpaces=false;
	bool LineBegin=true;
	bool CommentedLine=false;
	for (unsigned int i=0 ; i<FSize ; i++ , Ptr++)
	{
		if (*Ptr=='\t' || *Ptr=='\r' || *Ptr=='\n' || *Ptr==' ')
		{
			if (!MultiSpaces)
			{
				if (Column>0) ValidColumn++;
				if (*Ptr=='\r' || *Ptr=='\n')
				{
					MultiSpaces=true;
					if (ValidColumn>0) ValidLine++;
					ValidColumn=0;
					LineBegin=true;
					CommentedLine=false;
				}
				Column=0;
				NDots=0;
				NSigns=0;
				NExp=0;
			}
			continue;
		}
		MultiSpaces=false;
		if (LineBegin)
		{
			if (*Ptr=='%' || *Ptr==';') //comment till end of line
				CommentedLine=true;
			LineBegin=false;
		}
		if (CommentedLine) continue;
		if (Column>=0)
		{
			if (*Ptr=='.' || *Ptr==',')
			{
				if (NDots>0)
					Column=-1;
				else
					NDots++;
				continue;
			}
			if (*Ptr=='-' || *Ptr=='+')
			{
				if (NSigns>1)
					Column=-1;
				else
					NSigns++;
				continue;
			}
			if (*Ptr=='E' || *Ptr=='e')
			{
				if (NExp>0)
					Column=-1;
				else
					NExp++;
				continue;
			}
			if (isdigit(*Ptr))
				Column++;
			else
				Column=-1;
		}
		if (!(isdigit(*Ptr) || *Ptr=='.' || *Ptr==',' || *Ptr=='-' || *Ptr=='+' || toupper(*Ptr)=='E' || //number
			  *Ptr=='/' || *Ptr==':' || *Ptr=='A' || *Ptr=='P' || *Ptr=='M' || //date
			  *Ptr==' '))
			NonText++;
		if ((unsigned char)*Ptr<' ')
			InvalidText++;
	}
	if ((InvalidText || NonText>10) && ValidLine<20)
		LoadTirFile(Buffer,FSize);
	else
		LoadCsvFile(Buffer,FSize);

	free(Buffer);
	if (NPoints<2)
	{
		WriteMsg(__FILE__,__LINE__,tr("Only %1 data points in the file").arg(NPoints));
		Purge(XData);
		Purge(YData);
		NPoints=0;
	}

	//***** resize memory *****
	if (NPoints>0 && YData)
	{
		double *TData;

		TData=(double *)realloc(YData,NPoints*sizeof(double));
		if (TData) YData=TData;
		XPlot=(double *)malloc(NPoints*sizeof(double));
		if (!XPlot)
		{
			WriteMsg(__FILE__,__LINE__,tr("Not enough memory to load the file"));
			Purge(XData);
			Purge(YData);
			NPoints=0;
		}
		YPlot=(double *)malloc(NPoints*sizeof(double));
		if (!YPlot)
		{
			WriteMsg(__FILE__,__LINE__,tr("Not enough memory to load the file"));
			Purge(XData);
			Purge(YData);
			NPoints=0;
		}
		YSmooth=(double *)malloc(NPoints*sizeof(double));
		YDerv=(double *)malloc(NPoints*sizeof(double));
	}

	if (XData)
	{
		int i;
		double StepSize,Diff,MinDiff,MaxDiff;

		for (i=0 ; i<NPoints ; i++) XData[i]*=XGain;
		Time0=XData[0];
		Diff=XData[1]-XData[0];
		MinDiff=MaxDiff=Diff;
		for (i=2 ; i<NPoints ; i++)
		{
			Diff=XData[i]-XData[i-1];
			if (MinDiff>Diff) MinDiff=Diff;
			else if (MaxDiff<Diff) MaxDiff=Diff;
		}
		StepSize=fabs((XData[NPoints-1]-XData[0])/(NPoints-1));
		if (StepSize<1e-15)
			XFreq=0.;
		else
			XFreq=1/StepSize;
	}
	else
	{
		XFreq=StoredXFreq;
		Time0=StoredTime0;
	}

	LastGWidth=-1.;
	LastGNeigh=-1;
	LastSGPoly=-1;
	LastSGNeigh=-1;
	UpdateGraphics();
}

/*==========================================================================*/
/*!
  Recalculate the graphic with the data in memory.

  \date
	\arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::RecalculateGraphics()
{
	int i;
	double NextX,Slope,x,SMax;
	double Offset,Bkgr;
	QString Text;

	if (!YData || NPoints<=0) return;

	DisplayXFreq();
	ui->XFrequency->setEnabled(XData==NULL);
	DisplayTime0();
	ui->XTime0->setEnabled(XData==NULL);
	DisplayXGain();
	ui->XGain->setEnabled(XData!=NULL);
	if (XData)
	{
		for (i=0 ; i<NPoints ; i++) XPlot[i]=XData[i];
	}
	else
	{
		for (i=0 ; i<NPoints ; i++) XPlot[i]=(double)i/XFreq+Time0;
	}

	for (i=0 ; i<NPoints ; i++) YPlot[i]=YData[i]*YGain+YOffset;
	ui->MainGraphCtrl->SetGraphic(0,XPlot,YPlot,NPoints);
	if (BgForm)
	{
		BgForm->CalculateAutoBackground();
		if (BgForm->PrepareBkgr(Time0,XFreq,NPoints))
			ui->MainGraphCtrl->SetGraphic(2,BgForm->XBkgr,BgForm->YBkgr,BgForm->NBgPoints);
		else
			ui->MainGraphCtrl->DeleteCurve(2);
	}
	else
		ui->MainGraphCtrl->DeleteCurve(2);

	//***** redraw smoothed curve *****
	QLineEdit *SmoothMaxCtrl=NULL;
	if (YSmooth)
	{
		if (ui->SmoothTab->currentWidget()==ui->GaussianSmooth)
		{
			if (GaussWidth!=LastGWidth || GaussNeigh!=LastGNeigh)
			{
				CalcGaussSmooth(YData,XFreq,&Smooth,NPoints,GaussWidth,GaussNeigh);
				LastGWidth=GaussWidth;
				LastGNeigh=GaussNeigh;
			}
			SmoothMaxCtrl=ui->SmoothMaxCtrl;
		}
		if (ui->SmoothTab->currentWidget()==ui->SavGolSmooth)
		{
			if (SavGolSmoothPoly!=LastSGSPoly || SavGolSmoothNeigh!=LastSGSNeigh)
			{
				if (!Smooth)
				{
					Smooth=(double *)malloc(NPoints*sizeof(double));
					if (!Smooth)
					{
						WriteMsg(__FILE__,__LINE__,tr("Not enough memory to smooth the data"));
						return;
					}
				}
				SavGolSmooth(YData,Smooth,NPoints,SavGolSmoothPoly,SavGolSmoothNeigh);
				LastSGSPoly=SavGolSmoothPoly;
				LastSGSNeigh=SavGolSmoothNeigh;
			}
			SmoothMaxCtrl=ui->SavGolSMaxCtrl;
		}
	}
	if (YSmooth && Smooth)
	{
		//NextX=Time0;
		//if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
		if (XFreq<0.) //get last point if X axes reverted
			NextX=XPlot[NPoints-1];
		else
			NextX=XPlot[0];
		Slope=0.;
		Offset=0.;
		if (BgForm) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		Bkgr=Time0*Slope+Offset;
		SMax=Smooth[0]*YGain+YOffset-Bkgr;
		for (i=0 ; i<NPoints ; i++)
		{
			YSmooth[i]=Smooth[i]*YGain+YOffset;
			/*x=(double)i/XFreq+Time0;
			if ((XFreq>0. && x>=NextX) || (XFreq<0. && x<=NextX))
			 BackgroundForm->GetBackground(XFreq,&NextX,&Slope,&Offset);*/
			if (XFreq>0.)
			{
				x=XPlot[i];
				if (BgForm && x>=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
			}
			else
			{
				x=XPlot[NPoints-1-i];
				if (BgForm && x<=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
			}
			Bkgr=x*Slope+Offset;
			if (YSmooth[i]-Bkgr>SMax) SMax=YSmooth[i]-Bkgr;
		}
		ui->MainGraphCtrl->SetGraphic(1,XPlot,YSmooth,NPoints);
		Text.sprintf("%.7lg",SMax);
	}
	else
		Text.clear();
	if (SmoothMaxCtrl) SmoothMaxCtrl->setText(Text);

	//***** redraw derivative curve *****
	if (YDerv)
	{
		if ((ui->SavGolButton->isChecked()) && (SavGolPoly!=LastSGPoly || SavGolNeigh!=LastSGNeigh))
		{
			SavGolDervCalc(YData,&Derive,NPoints,SavGolPoly,SavGolNeigh);
			for (i=0 ; i<NPoints ; i++) Derive[i]*=XFreq;
			LastSGPoly=SavGolPoly;
			LastSGNeigh=SavGolNeigh;
		}
		if (ui->RawSmoothButton->isChecked() && Smooth)
		{
			LastSGPoly=-1;
			Purge(Derive);
			Derive=(double *)malloc(NPoints*sizeof(double));
			if (Derive==NULL)
			{
				WriteMsg(__FILE__,__LINE__,tr("Derivative buffer cannot be allocated"));
			}
			else
			{
				x=0.5*XFreq;
				for (i=1 ; i<NPoints-1 ; i++) Derive[i]=(Smooth[i+1]-Smooth[i-1])*x;
				*Derive=Derive[1];
				Derive[NPoints-1]=Derive[NPoints-2];
			}
		}
	}
	if (YDerv && Derive)
	{
		if (XFreq>0.)
		{
			NextX=Time0;
			for (i=0 ; i<NPoints ; i++)
			{
				if (BgForm && XPlot[i]>=NextX) Slope=BgForm->GetNextSlope(&NextX);
				YDerv[i]=Derive[i]*YGain-Slope;
			}
		}
		else
		{
			NextX=(double)(NPoints-1)/XFreq+Time0;
			for (i=NPoints-1 ; i>=0 ; i--)
			{
				if (BgForm && XPlot[i]>=NextX) Slope=BgForm->GetNextSlope(&NextX);
				YDerv[i]=Derive[i]*YGain-Slope;
			}
		}
		ui->DervGraphCtrl->SetGraphic(0,XPlot,YDerv,NPoints);
		SMax=YDerv[0];
		for (i=1 ; i<NPoints-1 ; i++)
			if (YDerv[i]>SMax) SMax=YDerv[i];
		Text.sprintf("%.7lg",SMax);
	}
	else
		Text.clear();
	ui->DervMaxCtrl->setText(Text);
}

/*==========================================================================*/
/*!
  Update the graphic with the data in memory.

  \date
	\arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::UpdateGraphics()
{
	QElapsedTimer t0;

	if (!YData || NPoints<2) return;
	t0.start();
	RecalculateGraphics();
	ui->MainGraphCtrl->Unzoom();
	ui->DervGraphCtrl->Unzoom();
	if (!YSmooth && ui->TrackSmooth->isChecked())
	{
		ui->TrackSmooth->setChecked(false);
		ui->TrackData->setChecked(true);
	}
	if (!YDerv && ui->TrackDerv->isChecked())
	{
		ui->TrackDerv->setChecked(false);
		ui->TrackData->setChecked(true);
	}
	ui->TrackSmooth->setEnabled(YSmooth!=NULL);
	ui->TrackDerv->setEnabled(YDerv!=NULL);
}

/*==========================================================================*/
/*!
  Write the new modified XFreq to the configuration file.

  \param XFreq X sampling frequency to write to the configuration file.

  \date
	\arg 2003-05-26 created by Frederic
 */
/*==========================================================================*/
void MainScreen::WriteXFreq(double XFreq)
{
	ConfigFile->Config_WriteDouble("Axis","XFreq",XFreq);
}

/*==========================================================================*/
/*!
  Display the sampling frequency in KHz on the main window.
 */
/*==========================================================================*/
void MainScreen::DisplayXFreq()
{
	QString Text;
	Text.sprintf("%.3lf",XFreq);
	bool block=ui->XFrequency->blockSignals(true);
	ui->XFrequency->setText(Text);
	ui->XFrequency->blockSignals(block);
}

/*==========================================================================*/
/*!
  The user go to another control. Update the new value.

  \date
	\arg 2001-12-13 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_XFrequency_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->XFrequency->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==XFreq) return;
	if (Value>=1E-3) XFreq=Value;

	DisplayXFreq();
	LastSGPoly=-1;
	WriteXFreq(XFreq);
	UpdateGraphics();
}

/*==========================================================================*/
/*!
  Write the new modified XTime0 to the configuration file.

  \param Time0 First time of the X axis to write to the configuration file.

  \date
	\arg 2003-05-26 created by Frederic
 */
/*==========================================================================*/
void MainScreen::WriteXTime0(double Time0)
{
	ConfigFile->Config_WriteDouble("Axis","XTime0",Time0);
}

/*==========================================================================*/
/*!
  Display the time offset in ms on the main window.
 */
/*==========================================================================*/
void MainScreen::DisplayTime0()
{
	QString Text;
	Text.sprintf("%.4lf",Time0);
	bool block=ui->XTime0->blockSignals(true);
	ui->XTime0->setText(Text);
	ui->XTime0->blockSignals(block);
}

/*==========================================================================*/
/*!
  The user pressed on the enter key while typing the start time.

  \date
	\arg 2001-12-13 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_XTime0_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->XTime0->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==Time0) return;
	Time0=Value;

	DisplayTime0();
	LastSGPoly=-1;
	WriteXTime0(Time0);
	UpdateGraphics();
}

/*==========================================================================*/
/*!
  Write the new modified XGain to the configuration file.

  \param XGain Gain to write into the configuration file.
 */
/*==========================================================================*/
void MainScreen::WriteXGain(double XGain)
{
	ConfigFile->Config_WriteDouble("Axis","XGain",XGain);
}

/*==========================================================================*/
/*!
  Display the time offset in ms on the main window.
 */
/*==========================================================================*/
void MainScreen::DisplayXGain()
{
	QString Text;
	Text.sprintf("%.4lf",XGain);
	bool block=ui->XGain->blockSignals(true);
	ui->XGain->setText(Text);
	ui->XGain->blockSignals(block);
}

/*==========================================================================*/
/*!
  The user changed the X gain.
 */
/*==========================================================================*/
void MainScreen::on_XGain_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->XGain->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==XGain) return;
	if (Value!=0.)
	{
		if (XData && XGain!=0.)
		{
			double Scale=Value/XGain;
			for (int i=0 ; i<NPoints ; i++)
				XData[i]*=Scale;
			XFreq/=Scale;
			DisplayXFreq();
		}
		XGain=Value;
	}

	DisplayXGain();
	LastSGPoly=-1;
	WriteXGain(XGain);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user pressed on the enter key while typing the width for
  the gaussian smoothing.

  \date 2001-12-13
*/
/*=============================================================================*/
void MainScreen::on_GaussWidthCtrl_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->GaussWidthCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==GaussWidth) return;
	if (Value>0.) GaussWidth=Value;

	QString Text;
	Text.sprintf("%.3lf",GaussWidth);
	ui->GaussWidthCtrl->setText(Text);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user pressed on the enter key while typing the number of
  neigbourg for the gaussian smoothing.

  \date: 2001-10-23
 */
/*=============================================================================*/
void MainScreen::on_GaussNeighCtrl_editingFinished()
{
	int Value;
	bool Ok=false;

	Value=ui->GaussNeighCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==GaussNeigh) return;
	if (Value>0) GaussNeigh=Value;

	QString Text;
	Text.sprintf("%d",GaussNeigh);
	ui->GaussNeighCtrl->setText(Text);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user changed the value of the polynomial for the Savitsky-Golay smoothing.

  \date 2012-08-06
*/
/*=============================================================================*/
void MainScreen::on_SavGolSPolyCtrl_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->SavGolSPolyCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==SavGolSmoothPoly) return;
	if (Value>0.) SavGolSmoothPoly=Value;

	QString Text;
	Text.sprintf("%d",SavGolSmoothPoly);
	ui->SavGolSPolyCtrl->setText(Text);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user edited the number of neighbour to use in the Savitsky-Golay smoothing.

  \date 2012-08-06
 */
/*=============================================================================*/
void MainScreen::on_SavGolSNeighCtrl_editingFinished()
{
	int Value;
	bool Ok=false;

	Value=ui->SavGolSNeighCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==SavGolSmoothNeigh) return;
	if (Value>0) SavGolSmoothNeigh=Value;

	QString Text;
	Text.sprintf("%d",SavGolSmoothNeigh);
	ui->SavGolSNeighCtrl->setText(Text);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The smoothing algorithm is changed.

  \date 2012-08-06
 */
/*=============================================================================*/
void MainScreen::on_SmoothTab_currentChanged(int Index)
{
	if (Index==0)//gaussian
	{
		LastGWidth=-1;
		LastGNeigh=-1;
	}
	else if (Index==1)//savitsky-golay
	{
		LastSGSPoly=-1;
		LastSGSNeigh=-1;
	}
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user pressed on the enter key while typing the Y gain.

  \date 2001-12-13
 */
/*=============================================================================*/
void MainScreen::on_YGainCtrl_editingFinished()
{
	double Value;
	QString Text;
	bool Ok=false;

	Value=ui->YGainCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==YGain) return;
	YGain=Value;
	Text.sprintf("%lf",YGain);
	ui->YGainCtrl->setText(Text);
	LastGNeigh=-1;
	LastSGPoly=-1;
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user pressed on the enter key while typing the Y offset.

  \date 2001-12-28
 */
/*=============================================================================*/
void MainScreen::on_YOffsetCtrl_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->YOffsetCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==YOffset) return;
	YOffset=Value;

	QString Text;
	Text.sprintf("%lf",YOffset);
	ui->YOffsetCtrl->setText(Text);
	LastSGPoly=-1;
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user pressed on the enter key while typing the polynomial
  for the derivative.

  \date 2001-12-13
 */
/*=============================================================================*/
void MainScreen::on_SavGolPolyCtrl_editingFinished()
{
	double Value;
	bool Ok=false;

	Value=ui->SavGolPolyCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==SavGolPoly) return;
	SavGolPoly=Value;

	QString Text;
	Text.sprintf("%d",SavGolPoly);
	ui->SavGolPolyCtrl->setText(Text);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user pressed on the enter key while typing the number
  the neighbourg for the derivative.

  \date 2001-12-13
 */
/*=============================================================================*/
void MainScreen::on_SavGolNeighCtrl_editingFinished()
{
	int Value;
	bool Ok=false;

	Value=ui->SavGolNeighCtrl->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value==SavGolNeigh) return;
	SavGolNeigh=Value;

	QString Text;
	Text.sprintf("%d",SavGolNeigh);
	ui->SavGolNeighCtrl->setText(Text);
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user wants to save the curves.

  \date 2001-10-29
 */
/*=============================================================================*/
void MainScreen::on_SaveMenu_aboutToShow()
{
	if (!YData)
	{
		ui->SaveDataMenu->setEnabled(false);
		ui->SaveDerivativeMenu->setEnabled(false);
		ui->SaveSmoothMenu->setEnabled(false);
	}
	else
	{
		ui->SaveDataMenu->setEnabled(true);
		ui->SaveSmoothMenu->setEnabled(Smooth!=NULL);
		ui->SaveDerivativeMenu->setEnabled(Derive!=NULL);
	}
}

/*=============================================================================*/
/*!
  The user wants to save the curves.

  \date 2001-10-29
 */
/*=============================================================================*/
void MainScreen::on_SaveDerivativeMenu_triggered()
{
	int i;
	double NextX,Slope,Offset,x,Bkgr;

	if (!Derive) return;
	if (!YDerv)
	{
		WriteMsg(__FILE__,__LINE__,tr("No derivative curve"));
		return;
	}
	QFileInfo FileName=ConfigFile->Config_GetFileName("Output","Derivative","");
	if (FileName.filePath().isEmpty())
	{
		ConfigFile->Config_WriteFileName("Output","Derivative","derive.txt");
		WriteMsg(__FILE__,__LINE__,tr("No output file in the config file"));
		return;
	}
	QFile fo(FileName.filePath());
	if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		WriteMsg(__FILE__,__LINE__,tr("Cannot save file %1").arg(FileName.filePath()));
		return;
	}
	QTextStream Out(&fo);
	//NextX=Time0;
	//if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
	if (XFreq<0.) //get last point if X axes reverted
		NextX=XPlot[NPoints-1];
	else
		NextX=XPlot[0];
	bool Error=false;
	for (i=0 ; i<NPoints ; i++)
	{
		/*x=(double)i/XFreq+Time0;
   if ((XFreq>0. && x>=NextX) || (XFreq<0. && x<=NextX))
	BackgroundForm->GetBackground(XFreq,&NextX,&Slope,&Offset);*/
		if (XFreq>0.)
		{
			x=XPlot[i];
			if (BgForm && x>=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		else
		{
			x=XPlot[NPoints-1-i];
			if (BgForm && x<=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		Bkgr=Slope;
		Out << YDerv[i]-Bkgr << endl;
		//if (fprintf(fo,"%lf\n",YDerv[i]-Bkgr)==EOF) Error=true;

		if (Out.status()!=QTextStream::Ok)
		{
			Error=true;
			break;
		}
	}
	fo.close();
	if (Error)
		WriteMsg(__FILE__,__LINE__,tr("Error while writting %1").arg(FileName.filePath()));
	else
		WriteMsg(__FILE__,__LINE__,tr("Derivative saved in %1").arg(FileName.filePath()));
}

/*=============================================================================*/
/*!
  The user wants to save the curves.

  \date 2001-10-29
 */
/*=============================================================================*/
void MainScreen::on_SaveSmoothMenu_triggered()
{
	int i;
	double x,Slope,Offset,NextX,Bkgr;

	if (!YSmooth)
	{
		WriteMsg(__FILE__,__LINE__,tr("No smoothed curve"));
		return;
	}
	QFileInfo FileName=ConfigFile->Config_GetFileName("Output","Smooth","");
	if (FileName.filePath().isEmpty())
	{
		ConfigFile->Config_WriteFileName("Output","Smooth","smooth.txt");
		WriteMsg(__FILE__,__LINE__,tr("No output file in the config file"));
		return;
	}
	QFile fo(FileName.filePath());
	if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		WriteMsg(__FILE__,__LINE__,tr("Cannot save file %1").arg(FileName.filePath()));
		return;
	}
	QTextStream Out(&fo);
	//NextX=Time0;
	//if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
	if (XFreq<0.) //get last point if X axes reverted
		NextX=XPlot[NPoints-1];
	else
		NextX=XPlot[0];
	bool Error=false;
	for (i=0 ; i<NPoints && !Error ; i++)
	{
		/*x=(double)i/XFreq+Time0;
   if ((XFreq>0. && x>=NextX) || (XFreq<0. && x<=NextX))
	BackgroundForm->GetBackground(XFreq,&NextX,&Slope,&Offset);*/
		if (XFreq>0.)
		{
			x=XPlot[i];
			if (BgForm && x>=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		else
		{
			x=XPlot[NPoints-1-i];
			if (BgForm && x<=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		Bkgr=x*Slope+Offset;
		Out << YSmooth[i]-Bkgr << endl;
		if (Out.status()!=QTextStream::Ok)
		{
			Error=true;
			break;
		}
	}
	fo.close();
	if (Error)
		WriteMsg(__FILE__,__LINE__,tr("Error while writting %1").arg(FileName.filePath()));
	else
		WriteMsg(__FILE__,__LINE__,tr("Smoothed curve saved in %1").arg(FileName.filePath()));
}

/*==========================================================================*/
/*!
  The user wants to save the curves.

  \date
	\arg 2001-10-29 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_SaveDataMenu_triggered()
{
	int i;

	if (!XPlot || !YPlot)
	{
		WriteMsg(__FILE__,__LINE__,tr("No data to save"));
		return;
	}
	QFileInfo SrcName(DefaultFileName);

	QString FileName=QFileDialog::getSaveFileName(this, tr("Save File"),
												  SrcName.completeBaseName()+".txt",
												  tr("Data (*.txt)"));
	if (FileName.isEmpty()) return;

	QFile fo(FileName);
	if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		WriteMsg(__FILE__,__LINE__,tr("Cannot open output file %1 for writting").arg(FileName));
		return;
	}
	QTextStream Out(&fo);
	for (i=0 ; i<NPoints ; i++)
		Out << XPlot[i] << "\t" << YPlot[i] << endl;

	fo.close();
}

/*==========================================================================*/
/*!
  The user wants to display or hide the dialog to edit the background.

  \date
	\arg 2001-11-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_BackgroundMenu_triggered()
{
	if (!BgForm)
	{
		BgForm=new BackgroundForm(this);
		BgForm->setModal(false);
		connect(BgForm,SIGNAL(UpdateGraphics()),this,SLOT(UpdateGraphics()));
		BgForm->show();
	}
	else if (BgForm->isVisible())
	{
		BgForm->setVisible(false);
	}
	else
	{
		BgForm->setVisible(true);
		BgForm->setFocus(Qt::OtherFocusReason);
	}
}

/*==========================================================================*/
/*!
  Set the function to call when the click on the main grah.
 */
/*==========================================================================*/
void MainScreen::SetBgMouseClick(bool Active)
{
	if (Active)
	{
		connect(ui->MainGraphCtrl,SIGNAL(LeftMouseClick(QMouseEvent*)),this,SLOT(BkgrLeftClick(QMouseEvent*)));
		connect(ui->MainGraphCtrl,SIGNAL(RightMouseClick(QMouseEvent*)),this,SLOT(BkgrRightClick(QMouseEvent*)));
	}
	else
	{
		disconnect(ui->MainGraphCtrl,SIGNAL(LeftMouseClick(QMouseEvent*)),this,SLOT(BkgrLeftClick(QMouseEvent*)));
		disconnect(ui->MainGraphCtrl,SIGNAL(RightMouseClick(QMouseEvent*)),this,SLOT(BkgrRightClick(QMouseEvent*)));
	}
}

/*==========================================================================*/
/*!
  Function called when the user click on the main graphic and the
  background window is visible.

  \date
	\arg 2001-11-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::BkgrLeftClick(QMouseEvent *event)
{
	double x,y;

	if (ui->MainGraphCtrl->GetMousePos(event->x(),event->y(),&x,&y))
	{
		BgForm->AddPoint(x,y);
		if (BgForm->PrepareBkgr(Time0,XFreq,NPoints))
			ui->MainGraphCtrl->SetGraphic(2,BgForm->XBkgr,BgForm->YBkgr,BgForm->NBgPoints);
		else
			ui->MainGraphCtrl->DeleteCurve(2);
		UpdateGraphics();
	}
}

/*==========================================================================*/
/*!
  Function called when the user click on the main graphic and the
  background window is visible.

  \date
	\arg 2001-11-22 created by Frederic
 */
/*==========================================================================*/
void MainScreen::BkgrRightClick(QMouseEvent *event)
{
	double x,y;

	if (ui->MainGraphCtrl->GetMousePos(event->x(),event->y(),&x,&y))
	{
		BgForm->RemovePoint(x,y);
		if (BgForm->PrepareBkgr(Time0,XFreq,NPoints))
			ui->MainGraphCtrl->SetGraphic(2,BgForm->XBkgr,BgForm->YBkgr,BgForm->NBgPoints);
		else
			ui->MainGraphCtrl->DeleteCurve(2);
		UpdateGraphics();
	}
}

/*==========================================================================*/
/*!
  Display the dialog box to select the output files.

  \date
    \arg 2001-12-01 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_SelectOutputFile_triggered()
{
	SelOutFile Form(this);
	Form.exec();
}

/*==========================================================================*/
/*!
  Display the dialog box to select the colors.

  \date
	\arg 2012-11-25 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_SelectColors_triggered()
{
	SelectColors Form(this);
	Form.exec();
}

/*==========================================================================*/
/*!
  Display the settings editor panel.
 */
/*==========================================================================*/
void MainScreen::on_Settings_triggered()
{
	Settings Form(this);

	Form.SetDecimalDot(NumbersWithDot);
	if (Form.exec()==QDialog::Accepted)
	{
		NumbersWithDot=Form.GetDecimalDot();
		ConfigFile->Config_WriteInt("Settings","NumbersWithDot",(NumbersWithDot) ? 1 : 0);
	}
}

/*==========================================================================*/
/*!
  Copy some text to the clipboard.

  \return true if the text was copied on the clipboard or false in case
           of failure.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
bool MainScreen::WriteToClipboard(const QString *Text)
{
	QClipboard *clipboard=QApplication::clipboard();
	clipboard->setText(*Text);
	return(true);
}

/*==========================================================================*/
/*!
  Copy a data curve to the clipboard.
 */
/*==========================================================================*/
void MainScreen::CopyDataPoints(double *Data)
{
	long i;
	double x,Bkgr,NextX;
	double Value;
	double Slope=0;
	double Offset=0;
	QString TBuff;
	QTextStream Buff(&TBuff);
	QLocale l=QLocale::system();

	if (XFreq>0.)
		NextX=XPlot[0];
	else
		NextX=XPlot[NPoints-1]; //get last point if X axes reverted
	for (i=0 ; i<NPoints ; i++)
	{
		if (XFreq>0.)
		{
			x=XPlot[i];
			if (BgForm && x>=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		else
		{
			x=XPlot[NPoints-1-i];
			if (BgForm && x<=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		Bkgr=x*Slope+Offset;
		if (Data)
			Value=Data[i]-Bkgr;
		else
			Value=Bkgr;
		if (NumbersWithDot)
			Buff << QString::number(XPlot[i]) << "\t" << QString::number(Value) << "\n";
		else
			Buff << l.toString(XPlot[i]) << "\t" << l.toString(Value) << "\n";
	}

	WriteToClipboard(Buff.string());
}

/*==========================================================================*/
/*!
  Copy the data curve to the clipboard.

  \date 2001-12-04
 */
/*==========================================================================*/
void MainScreen::on_CopyData_triggered()
{
	CopyDataPoints(YPlot);
}

/*==========================================================================*/
/*!
  Copy the background curve to the clipboard.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_CopyBkgrMenu_triggered()
{
	CopyDataPoints(NULL);
}

/*==========================================================================*/
/*!
  Copy the smoothed curve to the clipboard.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_CopySmoothMenu_triggered()
{
	if (!YSmooth)
	{
		WriteMsg(__FILE__,__LINE__,tr("No smoothed curve"));
		return;
	}
	CopyDataPoints(YSmooth);
}

/*==========================================================================*/
/*!
  Copy the derivative curve to the clipboard.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_CopyDeriveMenu_triggered()
{
	if (!YDerv)
	{
		WriteMsg(__FILE__,__LINE__,tr("No derivative curve"));
		return;
	}
	CopyDataPoints(YDerv);
}

/*==========================================================================*/
/*!
  Enable the copy menu items according to the data available in memory.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_CopyMenu_aboutToShow()
{
	ui->CopyData->setEnabled(YPlot!=NULL);
	ui->CopyBkgrMenu->setEnabled(YPlot!=NULL && BgForm && BgForm->BackgroundOk);
	ui->CopySmoothMenu->setEnabled(YSmooth!=NULL);
	ui->CopyDeriveMenu->setEnabled(YDerv!=NULL);
}

/*==========================================================================*/
/*==========================================================================*/
void MainScreen::on_RawSmoothButton_clicked()
{
	UpdateGraphics();
}

/*==========================================================================*/
/*==========================================================================*/
void MainScreen::on_SavGolButton_clicked()
{
	UpdateGraphics();
}

/*==========================================================================*/
/*!
  The user is removing the focus out of the tracker control. We update the
  tracked position.

  \date 2003-03-22
 */
/*==========================================================================*/
void MainScreen::on_XTracker_editingFinished()
{
	TrackPosition(true);
}

/*==========================================================================*/
/*!
  The user is removing the focus out of the tracker control. We update the
  tracked position.

  \date 2003-03-22
 */
/*==========================================================================*/
void MainScreen::on_YTracker_editingFinished()
{
	TrackPosition(false);
}

/*==========================================================================*/
/*!
  Display the X or Y position corresponding to the given coordinate.

  \param XSource If true, then we track the X position. If false, we track
         the Y position.

  \date 2003-03-22
 */
/*==========================================================================*/
void MainScreen::TrackPosition(bool XSource)
{
	int i;
	double XPosition,YPosition=0.;
	double *XPtr,*YPtr,*XSrc,*YSrc;
	QString Text;
	bool Found;

	LastTrackSrc=XSource;

	//***** get the source *****
	if (ui->TrackSmooth->isChecked())
	{
		XSrc=XPlot;
		YSrc=YSmooth;
	}
	else if (ui->TrackDerv->isChecked())
	{
		XSrc=XPlot;
		YSrc=YDerv;
	}
	else
	{
		XSrc=XPlot;
		YSrc=YPlot;
	}
	if (XSource)
	{
		if (ui->XTracker->text().isEmpty())
		{
			ui->YTracker->setText("");
			return;
		}
		XPosition=ui->XTracker->text().toDouble();
		XPtr=XSrc;
		YPtr=YSrc;
	}
	else
	{
		if (ui->YTracker->text().isEmpty())
		{
			ui->XTracker->setText("");
			return;
		}
		XPosition=ui->YTracker->text().toDouble();
		XPtr=YSrc;
		YPtr=XSrc;
	}

	//***** scan the data for the corresponding point *****
	Found=false;
	if (XPtr && YPtr)
	{
		for (i=1 ; i<NPoints ; i++)
		{
			if ((XPosition-XPtr[i-1])*(XPosition-XPtr[i])<=0.)
			{
				if (fabs(YPtr[i]-YPtr[i-1])>=1e50*fabs(XPtr[i]-XPtr[i-1]))
				{
					YPosition=YPtr[i];
				}
				else
				{
					YPosition=(XPosition-XPtr[i])/(XPtr[i-1]-XPtr[i])*(YPtr[i-1]-YPtr[i])+YPtr[i];
				}
				Found=true;
				break;
			}
		}
	}

	//***** display the corresponding value *****
	Text[0]=0;
	if (XSource)
	{
		if (Found) Text.sprintf("%.7lg",YPosition);
		ui->YTracker->setText(Text);
	}
	else
	{
		if (Found) Text.sprintf("%.4lg",YPosition);
		ui->XTracker->setText(Text);
	}
}


/*=============================================================================*/
/*!
  Control to track the position on the data.
 */
/*=============================================================================*/
void MainScreen::on_TrackData_clicked()
{
	disconnect(ui->MainGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	disconnect(ui->DervGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	TrackPosition(LastTrackSrc);
}

/*=============================================================================*/
/*!
  Control to track the position on the smoothed curve.
 */
/*=============================================================================*/
void MainScreen::on_TrackSmooth_clicked()
{
	disconnect(ui->MainGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	disconnect(ui->DervGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	TrackPosition(LastTrackSrc);
}

/*=============================================================================*/
/*!
  Control to track the position on the derivative.
 */
/*=============================================================================*/
void MainScreen::on_TrackDerv_clicked()
{
	disconnect(ui->MainGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	disconnect(ui->DervGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	TrackPosition(LastTrackSrc);
}

/*=============================================================================*/
/*!
  Control to track the position on the derivative.
 */
/*=============================================================================*/
void MainScreen::on_TrackMouse_clicked()
{
	connect(ui->MainGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
	connect(ui->DervGraphCtrl,SIGNAL(MouseMove(bool,double,double)),this,SLOT(TrackMouseMove(bool,double,double)));
}

/*==========================================================================*/
/*!
  The user wants to cut some data points and only leave the center part of
  the curve. A dialog box is displayed to ask the range to keep.

  \date
    \arg 2003-03-24 created by Frederic
 */
/*==========================================================================*/
void MainScreen::on_CutMenu_triggered()
{
	double XStart,XStop,StartTime=0.;
	double *TData;
	int i,j;

	if (!YData) return;

	{
		XRange XRangeForm(this);
		int Result=XRangeForm.exec();
		if (Result!=QDialog::Accepted)
		{
			return;
		}
		XRangeForm.GetRange(&XStart,&XStop);
	}
	if (XStart==XStop)
	{
		WriteMsg(__FILE__,__LINE__,tr("XStart and XStop must be different"));
		return;
	}
	if (!XData)
	{
		XData=(double *)malloc(NPoints*sizeof(double));
		if (!XData)
		{
			WriteMsg(__FILE__,__LINE__,tr("Not enough memory to store XData"));
			return;
		}
		for (i=0 ; i<NPoints ; i++) XData[i]=(double)i/XFreq+Time0;
	}

	//***** remove the data point *****
	if (XStart<XStop)
	{
		for (i=0 , j=0 ; i<NPoints && XPlot[i]<=XStop ; i++)
			if (XPlot[i]>=XStart)
			{
				if (j==0) StartTime=XPlot[i];
				if (XData) XData[j]=XData[i];
				YData[j]=YData[i];
				j++;
			}
	}
	else
	{
		for (i=0 , j=0 ; i<NPoints ; i++)
			if (XPlot[i]<=XStart || XPlot[i]>=XStop)
			{
				if (j==0) StartTime=XPlot[i];
				if (XData) XData[j]=XData[i];
				YData[j]=YData[i];
				j++;
			}
	}
	if (j<2)
	{
		WriteMsg(__FILE__,__LINE__,tr("Those limits would remove every data points. Cut not performed"));
		return;
	}
	NPoints=j;
	Time0=StartTime;
	DisplayTime0();
	HasBeenCut=true;

	ConfigFile->Config_WriteDouble("Cut","XStart",XStart);
	ConfigFile->Config_WriteDouble("Cut","XStop",XStop);

	if (XData)
	{
		TData=(double *)realloc(XData,NPoints*sizeof(double));
		if (TData) XData=TData;
	}
	TData=(double *)realloc(YData,NPoints*sizeof(double));
	if (TData) YData=TData;
	TData=(double *)realloc(XPlot,NPoints*sizeof(double));
	if (TData) XPlot=TData;
	TData=(double *)realloc(YPlot,NPoints*sizeof(double));
	if (TData) YPlot=TData;
	if (YSmooth)
	{
		TData=(double *)realloc(YSmooth,NPoints*sizeof(double));
		if (TData) YSmooth=TData;
	}
	if (YDerv)
	{
		TData=(double *)realloc(YDerv,NPoints*sizeof(double));
		if (TData) YDerv=TData;
	}

	ui->MainGraphCtrl->DeleteAllCurves();
	ui->DervGraphCtrl->DeleteAllCurves();
	LastGWidth=-1.;
	LastGNeigh=-1;
	LastSGPoly=-1;
	LastSGNeigh=-1;
	UpdateGraphics();
}

/*=============================================================================*/
/*!
  Function called when the mouse move on the graph.

  \date 2004-02-03
 */
/*=============================================================================*/
void MainScreen::TrackMouseMove(bool InGraph,double x,double y)
{
	QString Text;

	if (!ui->TrackMouse->isChecked()) return;
	if (InGraph)
	{
		Text.sprintf("%.7lg",x);
		ui->XTracker->setText(Text);
		Text.sprintf("%.4lg",y);
		ui->YTracker->setText(Text);
	}
	else
	{
		ui->XTracker->setText("");
		ui->YTracker->setText("");
	}
}

/*=============================================================================*/
/*!
  Display the about window.

  \date 2012-08-06
 */
/*=============================================================================*/
void MainScreen::on_AboutMenu_triggered()
{
	About form(this);

	form.Version(PROG_VERSION,PROG_REVISION);
	form.exec();
}

/*==========================================================================*/
/*!
  Set the graphs colors.
 */
/*==========================================================================*/
void MainScreen::ConfigureColors()
{
	QString Name=ConfigFile->Config_GetString("Colors","DispBg","#FFFFFF");
	QColor Color(Name);
	ui->MainGraphCtrl->SetBackgroundColor(Color);
	ui->DervGraphCtrl->SetBackgroundColor(Color);

	Name=ConfigFile->Config_GetString("Colors","Data","#FF0000");
	Color.setNamedColor(Name);
	ui->MainGraphCtrl->SetCurveColor(0,Color);

	Name=ConfigFile->Config_GetString("Colors","Smooth","#00FF00");
	Color.setNamedColor(Name);
	ui->MainGraphCtrl->SetCurveColor(1,Color);

	Name=ConfigFile->Config_GetString("Colors","Bkgr","#0000FF");
	Color.setNamedColor(Name);
	ui->MainGraphCtrl->SetCurveColor(2,Color);

	Name=ConfigFile->Config_GetString("Colors","Derv","#FF0000");
	Color.setNamedColor(Name);
	ui->DervGraphCtrl->SetCurveColor(0,Color);
}

/*=============================================================================*/
/*!
  Get the background color of the graphs.

  \return The background color.
 */
/*=============================================================================*/
QColor MainScreen::GetBackgroundColor()
{
	return(ui->MainGraphCtrl->GetBackgroundColor());
}

/*=============================================================================*/
/*!
  Set the background color of the graphs.

  \param Color The new color.
 */
/*=============================================================================*/
void MainScreen::SetBackgroundColor(const QColor &Color)
{
	ui->MainGraphCtrl->SetBackgroundColor(Color);
	ui->DervGraphCtrl->SetBackgroundColor(Color);
	ConfigFile->Config_WriteString("Colors","DispBg",Color.name());
}

/*=============================================================================*/
/*!
  Get the color of the data curve.

  \return The color.
 */
/*=============================================================================*/
QColor MainScreen::GetDataColor()
{
	return(ui->MainGraphCtrl->GetCurveColor(0));
}

/*=============================================================================*/
/*!
  Set the data curve color of the graphs.

  \param Color The new color.
 */
/*=============================================================================*/
void MainScreen::SetDataColor(const QColor &Color)
{
	ui->MainGraphCtrl->SetCurveColor(0,Color);
	ConfigFile->Config_WriteString("Colors","Data",Color.name());
}

/*=============================================================================*/
/*!
  Get the color of the smoothed data curve.

  \return The color.
 */
/*=============================================================================*/
QColor MainScreen::GetSmoothColor()
{
	return(ui->MainGraphCtrl->GetCurveColor(1));
}

/*=============================================================================*/
/*!
  Set the smoothed data curve color of the graphs.

  \param Color The new color.
 */
/*=============================================================================*/
void MainScreen::SetSmoothColor(const QColor &Color)
{
	ui->MainGraphCtrl->SetCurveColor(1,Color);
	ConfigFile->Config_WriteString("Colors","Smooth",Color.name());
}

/*=============================================================================*/
/*!
  Get the color of the derivative data curve.

  \return The color.
 */
/*=============================================================================*/
QColor MainScreen::GetDervColor()
{
	return(ui->DervGraphCtrl->GetCurveColor(0));
}

/*=============================================================================*/
/*!
  Set the data curve color of the graphs.

  \param Color The new color.
 */
/*=============================================================================*/
void MainScreen::SetDervColor(const QColor &Color)
{
	ui->DervGraphCtrl->SetCurveColor(0,Color);
	ConfigFile->Config_WriteString("Colors","Derv",Color.name());
}

/*=============================================================================*/
/*!
  Get the color of the background data curve.

  \return The color.
 */
/*=============================================================================*/
QColor MainScreen::GetBkgrColor()
{
	return(ui->MainGraphCtrl->GetCurveColor(2));
}

/*=============================================================================*/
/*!
  Set the data curve color of the graphs.

  \param Color The new color.
 */
/*=============================================================================*/
void MainScreen::SetBkgrColor(const QColor &Color)
{
	ui->MainGraphCtrl->SetCurveColor(2,Color);
	ConfigFile->Config_WriteString("Colors","Bkgr",Color.name());
}
