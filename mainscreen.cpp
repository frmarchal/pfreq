#include <qfile.h>
#include <qelapsedtimer.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <math.h>
#include "mainscreen.h"
#include "ui_mainscreen.h"
#include "GaussSmth.h"
#include "Utils.h"
//#include "SavGol.h"
#include "seloutfile.h"
#include "background.h"
#include "selectcolumn.h"
//#include "xrange.h"
#include "config.h"

#define PROG_VERSION 1
#define PROG_REVISION 6

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
				WriteMsg(__FILE__,__LINE__,QString("Column %1 contains an invalid number at line %2").arg(XColumn).arg(Line));
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
			WriteMsg(__FILE__,__LINE__,QString("Column %1 contains an invalid number at line %2").arg(YColumn).arg(Line));
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
			WriteMsg(__FILE__,__LINE__,QString("Not enough memory to load all the file. Some points after line %1 are missing").arg(Line));
			return(false);
		}
		*YData=TData;
		if (XColumn>=0)
		{
			TData=(double *)realloc(*XData,NAllocated*sizeof(double));
			if (!TData)
			{
				WriteMsg(__FILE__,__LINE__,QString("Not enough memory to load all the file. Some points after line %1 are missing").arg(Line));
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
	QString Item;
	QString ColMark;
	char *StoredColumns[NSTORED_LINES][MAX_COLUMN];
	double Value;
	int i,XColumn,YColumn;

	int Line=1;
	char *Ptr=Buffer;
	int NAllocated=0;
	int NColumn=0;
	int Column=0;
	int ValidColumns=0;
	bool InData=false;
	int NStoredLines=0;
	int NonSpaces;
	bool SelectingColumns=false;
	for (unsigned int NRead=0 ; NRead<FSize ; NRead++)
	{
		//while (NRead<FSize && Buffer[NRead]<=' ') NRead++;
		//if (NRead>=FSize) break;
		Ptr=Buffer+NRead;
		if (NStoredLines<NSTORED_LINES) StoredColumns[NStoredLines][Column]=Ptr;
		NonSpaces=0;
		for ( ; NRead<FSize && (Buffer[NRead]>' ' || (Buffer[NRead]==' ' && !NonSpaces)) ; NRead++)
			if (Buffer[NRead]!=' ') NonSpaces=1;
		if (NRead>=FSize) break;
		Delim=Buffer[NRead];
		Buffer[NRead]='\0';
		if (*Ptr) ValidColumns+=StrToDouble(Ptr,&Value) ? 1 : 0;
		Column++;
		if (Column>=MAX_COLUMN)
		{
			WriteMsg(__FILE__,__LINE__,"Too many columns in file. Reading aborted.");
			break;
		}
		if (Delim=='\r' || Delim=='\n')
		{
			if (!NColumn)
			{
				if (ValidColumns>0 || Line>1)
				{
					NColumn=Column;
					if (!GetColumns(NColumn,XColumn,YColumn) || YColumn<0)
					{
						if (NColumn==1)
						{
							ConfigFile->Config_WriteString("Columns",Item,"-1,0");
							XColumn=-1;
							YColumn=0;
						}
						else if (NColumn==2)
						{
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
						WriteMsg(__FILE__,__LINE__,QString("Invalid X column for a file containing %1 columns").arg(NColumn));
						break;
					}
					if (YColumn>=NColumn)
					{
						WriteMsg(__FILE__,__LINE__,QString("Invalid Y column for a file containing %1 columns").arg(NColumn));
						break;
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
					if (ValidColumns>0) WriteMsg(__FILE__,__LINE__,QString("Inconsistant number of columns in the file at line %1").arg(Line));
					break;
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

					Result=ColSel.exec();
					if (Result==QDialog::Accepted)
						ColSel.GetColumn(XColumn,YColumn);
					if (Result!=QDialog::Accepted) break;
					SelectingColumns=false;
					for (i=0 ; i<NStoredLines ; i++)
					{
						if (!AddDataPoint(StoredColumns[i],&XData,&YData,NPoints,NAllocated,InData,XColumn,YColumn,i)) break;
					}
					if (i<NStoredLines) break;
					NStoredLines=0;
				}
			}
			else
			{
				if (!AddDataPoint(StoredColumns[0],&XData,&YData,NPoints,NAllocated,InData,XColumn,YColumn,Line)) break;
			}
			Column=0;
			ValidColumns=0;
			if (Delim=='\r' && Buffer[NRead+1]=='\n') NRead++;
			Line++;
		}
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
			WriteMsg(__FILE__,__LINE__,QString("Cannot open %1").arg(FileName));
			return;
		}
		FSize=hFile.size();
		if (FSize==0)
		{
			hFile.close();
			WriteMsg(__FILE__,__LINE__,"File is empty");
			return;
		}
		Buffer=(char *)malloc(FSize);
		if (!Buffer)
		{
			hFile.close();
			WriteMsg(__FILE__,__LINE__,"Not enough memory to load the file");
			return;
		}
		NRead=hFile.read(Buffer,FSize);
		if (NRead!=FSize)
		{
			free(Buffer);
			hFile.close();
			WriteMsg(__FILE__,__LINE__,"Cannot read file");
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
	int MultiSpaces=0;
	for (unsigned int i=0 ; i<FSize ; i++ , Ptr++)
	{
		if (*Ptr=='\t' || *Ptr=='\r' || *Ptr=='\n' || *Ptr==' ')
		{
			if (!MultiSpaces)
			{
				if (Column>0) ValidColumn++;
				if (*Ptr!='\t' || *Ptr==' ')
				{
					MultiSpaces++;
					if (ValidColumn>0) ValidLine++;
					ValidColumn=0;
				}
				Column=0;
				NDots=0;
				NSigns=0;
				NExp=0;
			}
			continue;
		}
		MultiSpaces=0;
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
		if (!(isdigit(*Ptr) || *Ptr=='.' || *Ptr==',' || *Ptr=='\r' ||
			  *Ptr=='\n' || *Ptr=='-' || *Ptr=='+' || toupper(*Ptr)=='E' ||
			  *Ptr=='/' || *Ptr==':' || *Ptr=='A' || *Ptr=='P' || *Ptr=='M' ||
			  *Ptr==' '))
			NonText++;
		if ((unsigned char)*Ptr<' ' && *Ptr!='\n' && *Ptr!='\r' && *Ptr!='\t')
			InvalidText++;
	}
	if ((InvalidText || NonText>10) && ValidLine<20)
		LoadTirFile(Buffer,FSize);
	else
		LoadCsvFile(Buffer,FSize);

	free(Buffer);
	if (NPoints<2)
	{
		WriteMsg(__FILE__,__LINE__,QString("Only %1 data points in the file").arg(NPoints));
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
			WriteMsg(__FILE__,__LINE__,"Not enough memory to load the file");
			Purge(XData);
			Purge(YData);
			NPoints=0;
		}
		YPlot=(double *)malloc(NPoints*sizeof(double));
		if (!YPlot)
		{
			WriteMsg(__FILE__,__LINE__,"Not enough memory to load the file");
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
			XFreq=1./StepSize;
		/*if (fabs(MaxDiff-MinDiff)>=1E-3*StepSize)
	{
	Purge(YSmooth);
	Purge(YDerv);
	}*/
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
  Add a string to the memo.

  \param Text The text to add.
 */
/*==========================================================================*/
void MainScreen::AddMemoLine(const QString &Text)
{
	QTextCursor Cursor=ui->Memo1->textCursor();
	Cursor.insertText(Text);
	ui->Memo1->setTextCursor(Cursor);
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
	QElapsedTimer t0;
	double NextX,Slope,x,SMax;
	double Offset,Bkgr;
	QString Text;

	if (!YData || NPoints<=0) return;
	t0.start();
	QTextDocument *Doc=ui->Memo1->document();
	Doc->clear();
	AddMemoLine("Clear:"+QString::number(t0.elapsed())+"\n");

	if (XData)
	{
		ui->XFrequency->setEnabled(false);
		ui->XTime0->setEnabled(false);
		for (i=0 ; i<NPoints ; i++) XPlot[i]=XData[i];
	}
	else
	{
		ui->XFrequency->setEnabled(true);
		ui->XTime0->setEnabled(true);
		for (i=0 ; i<NPoints ; i++) XPlot[i]=(double)i/XFreq+Time0;
	}
	Text.sprintf("%.4lf",XFreq);
	ui->XFrequency->setText(Text);
	Text.sprintf("%.4lf",Time0);
	ui->XTime0->setText(Text);

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
	AddMemoLine("Data:"+QString::number(t0.elapsed())+"\n");

#if 0
	//***** redraw smoothed curve *****
	if (YSmooth && (GaussWidth!=LastGWidth || GaussNeigh!=LastGNeigh))
	{
		CalcGaussSmooth(YData,XFreq,&Smooth,NPoints,GaussWidth,GaussNeigh);
		LastGWidth=GaussWidth;
		LastGNeigh=GaussNeigh;
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
		BackgroundForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
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
				if (x>=NextX) BackgroundForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
			}
			else
			{
				x=XPlot[NPoints-1-i];
				if (x<=NextX) BackgroundForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
			}
			Bkgr=x*Slope+Offset;
			if (YSmooth[i]-Bkgr>SMax) SMax=YSmooth[i]-Bkgr;
		}
		ui->MainGraphCtrl->SetGraphic(1,XPlot,YSmooth,NPoints);
		Text.sprintf("%.7lg",SMax);
	}
	else
		Text.clear();
	ui->SmoothMaxCtrl->setText(Text);
	Memo1->Lines->Add("Smooth:"+QString::number(t0.elapsed()));

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
				WriteMsg(__FILE__,__LINE__,"Derivative cannot be allocated");
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
				if (XPlot[i]>=NextX) Slope=BackgroundForm->GetNextSlope(&NextX);
				YDerv[i]=Derive[i]*YGain-Slope;
			}
		}
		else
		{
			NextX=(double)(NPoints-1)/XFreq+Time0;
			for (i=NPoints-1 ; i>=0 ; i--)
			{
				if (XPlot[i]>=NextX) Slope=BackgroundForm->GetNextSlope(&NextX);
				YDerv[i]=Derive[i]*YGain-Slope;
			}
		}
		DervGraphic->SetGraphic(0,XPlot,YDerv,NPoints);
		SMax=YDerv[0];
		for (i=1 ; i<NPoints-1 ; i++)
			if (YDerv[i]>SMax) SMax=YDerv[i];
		Text.sprintf("%.7lg",SMax);
	}
	else
		Text.clear();
	ui->DervMaxCtrl->setText(Text);
	Memo1->Lines->Add("Derivative:"+QString::number(t0.elapsed()));
#endif
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
	AddMemoLine("Total:"+QString::number(t0.elapsed())+"\n");
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
	if (Value>=0.001) XFreq=Value;

	QString Text;
	Text.sprintf("%.0lf",XFreq);
	ui->XFrequency->setText(Text);
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

	QString Text;
	Text.sprintf("%.4lf",Time0);
	ui->XTime0->setText(Text);
	LastSGPoly=-1;
	WriteXTime0(Time0);
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
		WriteMsg(__FILE__,__LINE__,"No derivative curve");
		return;
	}
	QFileInfo FileName=ConfigFile->Config_GetFileName("Output","Derivative","");
	if (FileName.filePath().isEmpty())
	{
		ConfigFile->Config_WriteFileName("Output","Derivative","derive.txt");
		WriteMsg(__FILE__,__LINE__,"No output file in the config file");
		return;
	}
	QFile fo(FileName.filePath());
	if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		WriteMsg(__FILE__,__LINE__,QString("Cannot save file %1").arg(FileName.filePath()));
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
			if (x>=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		else
		{
			x=XPlot[NPoints-1-i];
			if (x<=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
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
		WriteMsg(__FILE__,__LINE__,QString("Error while writting %1").arg(FileName.filePath()));
	else
		WriteMsg(__FILE__,__LINE__,QString("Derivative saved in %1").arg(FileName.filePath()));
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
		WriteMsg(__FILE__,__LINE__,"No smoothed curve");
		return;
	}
	QFileInfo FileName=ConfigFile->Config_GetFileName("Output","Smooth","");
	if (FileName.filePath().isEmpty())
	{
		ConfigFile->Config_WriteFileName("Output","Smooth","smooth.txt");
		WriteMsg(__FILE__,__LINE__,"No output file in the config file");
		return;
	}
	QFile fo(FileName.filePath());
	if (!fo.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		WriteMsg(__FILE__,__LINE__,QString("Cannot save file %1").arg(FileName.filePath()));
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
			if (x>=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
		}
		else
		{
			x=XPlot[NPoints-1-i];
			if (x<=NextX) BgForm->GetBackground(XFreq,&NextX,&Slope,&Offset);
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
		WriteMsg(__FILE__,__LINE__,QString("Error while writting %1").arg(FileName.filePath()));
	else
		WriteMsg(__FILE__,__LINE__,QString("Smoothed curve saved in %1").arg(FileName.filePath()));
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
		WriteMsg(__FILE__,__LINE__,"No data to save");
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
		WriteMsg(__FILE__,__LINE__,QString("Cannot open output file %1 for writting").arg(FileName));
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
		BgForm->open();
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
	BgForm->AddPoint(event->x(),event->y());
	if (BgForm->PrepareBkgr(Time0,XFreq,NPoints))
		ui->MainGraphCtrl->SetGraphic(2,BgForm->XBkgr,BgForm->YBkgr,BgForm->NBgPoints);
	else
		ui->MainGraphCtrl->DeleteCurve(2);
	UpdateGraphics();
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
	BgForm->RemovePoint(event->x(),event->y());
	if (BgForm->PrepareBkgr(Time0,XFreq,NPoints))
		ui->MainGraphCtrl->SetGraphic(2,BgForm->XBkgr,BgForm->YBkgr,BgForm->NBgPoints);
	else
		ui->MainGraphCtrl->DeleteCurve(2);
	UpdateGraphics();
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

