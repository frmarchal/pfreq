//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdlib.h>
#include <stdio.h>
#include <typeinfo.h>
#include <math.h>
#include "MainScreen.h"
#include "gausssmth.h"
#include "utils.h"
#include "savgol.h"
#include "SelOutFile.h"
#include "background.h"
#include "selcolumn.h"
#include "xrange.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PROG_VERSION 1
#define PROG_REVISION 6

//! Maximum number of columns in the file to load.
#define MAX_COLUMN 30

//! Configuration file for this program.
char ConfigFile[_MAX_PATH];

extern TBackgroundForm *BackgroundForm;

/*==========================================================================*/
/*!
  Constructor of the class. It loads the last settings saved the last time
  the program was used.

  \date
    \arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
__fastcall TMainForm::TMainForm(TComponent* Owner)
        : TForm(Owner)
{
  char Text[80];
  char Drive[_MAX_DRIVE],Dir[_MAX_DIR],Name[_MAX_FNAME],FileName[_MAX_PATH];
  char Format[80];
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

  strcpy(Format,ProgVersionLabel->Caption.c_str());
  sprintf(Text,Format,PROG_VERSION,PROG_REVISION);
  ProgVersionLabel->Caption=Text;

  GetModuleFileName(HInstance,ConfigFile,sizeof(ConfigFile));
  _splitpath(ConfigFile,Drive,Dir,Name,NULL);
  _makepath(ConfigFile,Drive,Dir,Name,"ini");
  GetPrivateProfileString("Input","FileName","*",FileName,sizeof(FileName),ConfigFile);
  DefaultFileName=FileName;

  GetPrivateProfileString("Axis","XFreq","250.",Text,sizeof(Text),ConfigFile);
  XFreq=atof(Text);
  sprintf(Text,"%.4lf",XFreq);
  XFrequency->Text=Text;
  GetPrivateProfileString("Axis","XTime0","-20.",Text,sizeof(Text),ConfigFile);
  Time0=atof(Text);
  sprintf(Text,"%.4lf",Time0);
  XTime0->Text=Text;

  GetPrivateProfileString("Axis","YGain","1.",Text,sizeof(Text),ConfigFile);
  YGain=atof(Text);
  sprintf(Text,"%lf",YGain);
  YGainCtrl->Text=Text;
  GetPrivateProfileString("Axis","YOffset","0.",Text,sizeof(Text),ConfigFile);
  YOffset=atof(Text);
  sprintf(Text,"%lf",YOffset);
  YOffsetCtrl->Text=Text;

  GetPrivateProfileString("Gauss","Width","10.",Text,sizeof(Text),ConfigFile);
  GaussWidth=atof(Text);
  sprintf(Text,"%.3lf",GaussWidth);
  GaussWidthCtrl->Text=Text;
  GetPrivateProfileString("Gauss","Neigh","50",Text,sizeof(Text),ConfigFile);
  GaussNeigh=atoi(Text);
  sprintf(Text,"%d",GaussNeigh);
  GaussNeighCtrl->Text=Text;
  LastGWidth=-1.;
  LastGNeigh=-1;

  GetPrivateProfileString("Derive","Mode","0",Text,sizeof(Text),ConfigFile);
  Selection=atoi(Text);
  if (Selection==0)
   RawSmoothButton->Checked=true;
  else
   SavGolButton->Checked=true;
  GetPrivateProfileString("Derive","Poly","2",Text,sizeof(Text),ConfigFile);
  SavGolPoly=atoi(Text);
  sprintf(Text,"%d",SavGolPoly);
  SavGolPolyCtrl->Text=Text;
  GetPrivateProfileString("Derive","Neigh","50",Text,sizeof(Text),ConfigFile);
  SavGolNeigh=atoi(Text);
  sprintf(Text,"%d",SavGolNeigh);
  SavGolNeighCtrl->Text=Text;
  LastSGPoly=-1.;
  LastSGNeigh=-1;

  Top=0;
  Left=0;
  //BackgroundForm=new TBackgroundForm(this);
  //BackgroundForm->Parent=this;

  int GHeight=(ClientHeight-20)/2;
  int GWidth=ClientWidth-MainGraphCtrl->Left-5;
  SetGraphSize(MainGraphCtrl,GWidth,GHeight);
  SetGraphSize(DervGraphCtrl,GWidth,GHeight);
  MainGraphic=new ImageGraph(this,MainGraphCtrl);
  DervGraphic=new ImageGraph(this,DervGraphCtrl);
}

/*==========================================================================*/
/*!
  Destructor of the class. It cleans up the memory.

  \date
    \arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
__fastcall TMainForm::~TMainForm(void)
{
  char Text[50];
  int Selection;

  /*sprintf(Text,"%lf",XFreq);
  WritePrivateProfileString("Axis","XFreq",Text,ConfigFile);
  if (!HasBeenCut)
   {
   sprintf(Text,"%lf",Time0);
   WritePrivateProfileString("Axis","XTime0",Text,ConfigFile);
   }*/

  sprintf(Text,"%lf",YGain);
  WritePrivateProfileString("Axis","YGain",Text,ConfigFile);
  sprintf(Text,"%lf",YOffset);
  WritePrivateProfileString("Axis","YOffset",Text,ConfigFile);

  sprintf(Text,"%.3lf",GaussWidth);
  WritePrivateProfileString("Gauss","Width",Text,ConfigFile);
  sprintf(Text,"%d",GaussNeigh);
  WritePrivateProfileString("Gauss","Neigh",Text,ConfigFile);

  if (RawSmoothButton->Checked)
   Selection=0;
  else
   Selection=1;
  sprintf(Text,"%d",Selection);
  WritePrivateProfileString("Derive","Mode",Text,ConfigFile);
  sprintf(Text,"%d",SavGolPoly);
  WritePrivateProfileString("Derive","Poly",Text,ConfigFile);
  sprintf(Text,"%d",SavGolNeigh);
  WritePrivateProfileString("Derive","Neigh",Text,ConfigFile);

  //delete BackgroundForm;
  Purge(Derive);
  Purge(Smooth);
  Purge(XData);
  Purge(YData);
  Purge(XPlot);
  Purge(YPlot);
  Purge(YSmooth);
  Purge(YDerv);
}

/*==========================================================================*/
/*!
  The user wants to exit the program.

  \date
    \arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::ExitMenuClick(TObject *Sender)
{
  Close();
}

/*==========================================================================*/
/*!
  The user wants to load a file.

  \date
    \arg 2003-03-24 created by Frederic
 */
/*==========================================================================*/
bool TMainForm::AddDataPoint(char **ColumnsTxt,double **XData,double **YData,int &NPoints,int &NAllocated,
    bool &InData,int XColumn,int YColumn,int Line)
{
  double XValue,YValue,*TData;
  bool CXValid,CYValid;

  if (XColumn>=0)
   {
   XValue=0.;
   CXValid=(sscanf(ColumnsTxt[XColumn],"%lf",&XValue)==1);
   if (!CXValid)
    {
    if (InData)
     {
     WriteMsg(__FILE__,__LINE__,"Column %d contains an invalid number at line %d",XColumn,Line);
     return(false);
     }
    return(true);
    }
   }
  YValue=0.;
  CYValid=(sscanf(ColumnsTxt[YColumn],"%lf",&YValue)==1);
  if (!CYValid)
   {
   if (InData)
    {
    WriteMsg(__FILE__,__LINE__,"Column %d contains an invalid number at line %d",YColumn,Line);
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
    WriteMsg(__FILE__,__LINE__,"Not enough memory to load all the file. Some points after line %d are missing",Line);
    return(false);
    }
   *YData=TData;
   if (XColumn>=0)
    {
    TData=(double *)realloc(*XData,NAllocated*sizeof(double));
    if (!TData)
     {
     WriteMsg(__FILE__,__LINE__,"Not enough memory to load all the file. Some points after line %d are missing",Line);
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
  Decode and load a CSV file.

  \param Buffer The buffer containing the file to convert.
  \param FSize The size of the file buffer.

  \return True if the data could be parsed or false if an error occured.

  \date 2004-02-06
 */
/*==========================================================================*/
bool TMainForm::LoadCsvFile(char *Buffer,unsigned int FSize)
{
#define NSTORED_LINES 25
  char Delim,Item[30],ColMark[30];
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
   Buffer[NRead]=0;
   if (*Ptr) ValidColumns+=sscanf(Ptr,"%lf",&Value);
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
      sprintf(Item,"%d",NColumn);
      GetPrivateProfileString("Columns",Item,"-1,-1",ColMark,sizeof(ColMark),ConfigFile);
      if (sscanf(ColMark,"%d,%d",&XColumn,&YColumn)!=2 || YColumn<0)
       {
       if (NColumn==1)
        {
        WritePrivateProfileString("Columns",Item,"-1,0",ConfigFile);
        XColumn=-1;
        YColumn=0;
        }
       else if (NColumn==2)
        {
        WritePrivateProfileString("Columns",Item,"0,1",ConfigFile);
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
       WriteMsg(__FILE__,__LINE__,"Invalid X column for a file containing %d columns",NColumn);
       break;
       }
      if (YColumn>=NColumn)
       {
       WriteMsg(__FILE__,__LINE__,"Invalid Y column for a file containing %d columns",NColumn);
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
      if (ValidColumns>0) WriteMsg(__FILE__,__LINE__,"Inconsistant number of columns in the file at line %d",Line);
      break;
      }
     }

    if (SelectingColumns)
     {
     TSelectColumn *SelectColumn;
     int Result;

     if ((ValidColumns>0 || Line>1) && ++NStoredLines>=NSTORED_LINES)
      {
      SelectColumn=new TSelectColumn(this);
      if (!SelectColumn)
       {
       WriteMsg(__FILE__,__LINE__,"Cannot create columns selector");
       break;
       }
      SelectColumn->PrepareList(NColumn);
      for (i=0 ; i<NStoredLines ; i++)
       SelectColumn->AddLine(StoredColumns[i],NColumn);

      Result=SelectColumn->ShowModal();
      if (Result==mrOk)
       SelectColumn->GetColumn(XColumn,YColumn);
      delete SelectColumn;
      if (Result!=mrOk) break;
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
bool TMainForm::LoadTirFile(char *Buffer,unsigned int FSize)
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
     WriteMsg(__FILE__,__LINE__,"Not enough memory to load all the file. %d points may be missing",NFilePoints-i);
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
void __fastcall TMainForm::LoadMenuClick(TObject *Sender)
{
  HANDLE hFile;
  DWORD FSize,NRead;
  char *Buffer;

  //***** get file name *****
  OpenFileDlg->FileName=DefaultFileName;
  if (!OpenFileDlg->Execute()) return;
  DefaultFileName=OpenFileDlg->FileName;
  WritePrivateProfileString("Input","FileName",DefaultFileName.c_str(),ConfigFile);

  //***** read file *****
  hFile=CreateFile(DefaultFileName.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
   {
   WriteMsg(__FILE__,__LINE__,"Cannot open %s",DefaultFileName.c_str());
   return;
   }
  FSize=GetFileSize(hFile,NULL);
  if (FSize==0xFFFFFFFF)
   {
   CloseHandle(hFile);
   WriteMsg(__FILE__,__LINE__,"Cannot get file size");
   return;
   }
  if (FSize==0)
   {
   CloseHandle(hFile);
   WriteMsg(__FILE__,__LINE__,"File is empty");
   return;
   }
  Buffer=(char *)malloc(FSize);
  if (!Buffer)
   {
   CloseHandle(hFile);
   WriteMsg(__FILE__,__LINE__,"Not enough memory to load the file");
   return;
   }
  if (!ReadFile(hFile,Buffer,FSize,&NRead,NULL) || NRead!=FSize)
   {
   free(Buffer);
   CloseHandle(hFile);
   WriteMsg(__FILE__,__LINE__,"Cannot read file");
   return;
   }
  CloseHandle(hFile);

  //***** parse file *****
  if (!XData && !HasBeenCut)  //if no X currently in memory, store the paramters
   {
   StoredXFreq=XFreq;
   StoredTime0=Time0;
   }
  MainGraphic->DeleteAllCurves();
  DervGraphic->DeleteAllCurves();
  NPoints=0;
  HasBeenCut=false;
  Purge(XData);
  Purge(YData);
  Purge(XPlot);
  Purge(YPlot);
  Purge(YSmooth);
  Purge(YDerv);
  Caption="MainForm "+DefaultFileName;

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
   WriteMsg(__FILE__,__LINE__,"Only %d data points in the file",NPoints);
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
  Recalculate the graphic with the data in memory.

  \date
    \arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void TMainForm::RecalculateGraphics(void)
{
  int i;
  DWORD t0;
  double NextX,Slope,x,SMax;
  double Offset,Bkgr;
  char Text[30];

  if (!YData || NPoints<=0) return;
  t0=GetTickCount();
  Memo1->Clear();
  Memo1->Lines->Add("Clear:"+AnsiString(GetTickCount()-t0));

  if (XData)
   {
   XFrequency->Enabled=false;
   XTime0->Enabled=false;
   for (i=0 ; i<NPoints ; i++) XPlot[i]=XData[i];
   }
  else
   {
   XFrequency->Enabled=true;
   XTime0->Enabled=true;
   for (i=0 ; i<NPoints ; i++) XPlot[i]=(double)i/XFreq+Time0;
   }
  sprintf(Text,"%.4lf",XFreq);
  XFrequency->Text=Text;
  sprintf(Text,"%.4lf",Time0);
  XTime0->Text=Text;

  for (i=0 ; i<NPoints ; i++) YPlot[i]=YData[i]*YGain+YOffset;
  MainGraphic->SetGraphic(0,XPlot,YPlot,NPoints);
  if (BackgroundForm)
   {
   BackgroundForm->CalculateAutoBackground();
   if (BackgroundForm->PrepareBkgr(Time0,XFreq,NPoints))
    MainGraphic->SetGraphic(2,BackgroundForm->XBkgr,BackgroundForm->YBkgr,BackgroundForm->NBgPoints);
   else
    MainGraphic->DeleteCurve(2);
   }
  else
   MainGraphic->DeleteCurve(2);
  Memo1->Lines->Add("Data:"+AnsiString(GetTickCount()-t0));

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
   MainGraphic->SetGraphic(1,XPlot,YSmooth,NPoints);
   sprintf(Text,"%.7lg",SMax);
   }
  else
   Text[0]=0;
  SmoothMaxCtrl->Text=Text;
  Memo1->Lines->Add("Smooth:"+AnsiString(GetTickCount()-t0));

  //***** redraw derivative curve *****
  if (YDerv)
   {
   if ((SavGolButton->Checked) && (SavGolPoly!=LastSGPoly || SavGolNeigh!=LastSGNeigh))
    {
    SavGolDervCalc(YData,&Derive,NPoints,SavGolPoly,SavGolNeigh);
    for (i=0 ; i<NPoints ; i++) Derive[i]*=XFreq;
    LastSGPoly=SavGolPoly;
    LastSGNeigh=SavGolNeigh;
    }
   if(RawSmoothButton->Checked && Smooth)
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
   sprintf(Text,"%.7lg",SMax);
   }
  else
   Text[0]=0;
  DervMaxCtrl->Text=Text;
  Memo1->Lines->Add("Derivative:"+AnsiString(GetTickCount()-t0));
}

/*==========================================================================*/
/*!
  Update the graphic with the data in memory.

  \date
    \arg 2001-10-22 created by Frederic
 */
/*==========================================================================*/
void TMainForm::UpdateGraphics(void)
{
  DWORD t0;

  if (!YData || NPoints<2) return;
  t0=GetTickCount();
  RecalculateGraphics();
  MainGraphic->Unzoom();
  DervGraphic->Unzoom();
  if (!YSmooth && TrackSmooth->Checked)
   {
   TrackSmooth->Checked=false;
   TrackData->Checked=true;
   }
  if (!YDerv && TrackDerv->Checked)
   {
   TrackDerv->Checked=false;
   TrackData->Checked=true;
   }
  TrackSmooth->Enabled=(YSmooth!=NULL);
  TrackDerv->Enabled=(YDerv!=NULL);
  Memo1->Lines->Add("Total:"+AnsiString(GetTickCount()-t0));
}

/*==========================================================================*/
/*!
  Write the new modified XFreq to the configuration file.

  \param XFreq X sampling frequency to write to the configuration file.

  \date
    \arg 2003-05-26 created by Frederic
 */
/*==========================================================================*/
void TMainForm::WriteXFreq(double XFreq)
{
  char Text[50];

  sprintf(Text,"%lf",XFreq);
  WritePrivateProfileString("Axis","XFreq",Text,ConfigFile);
}

/*==========================================================================*/
/*!
  The user pressed on the enter key while typing the frequency.

  \date
    \arg 2001-10-23 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::XFrequencyKeyPress(TObject *Sender, char &Key)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   Value=atof(Ctrl->Text.c_str());
   if (Value>=0.001) XFreq=Value;
   sprintf(Text,"%.4lf",XFreq);
   Ctrl->Text=Text;
   LastSGPoly=-1;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   WriteXFreq(XFreq);
   }
}

/*==========================================================================*/
/*!
  The user go to another control. Update the new value.

  \date
    \arg 2001-12-13 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::XFrequencyExit(TObject *Sender)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atof(Ctrl->Text.c_str());
  if (Value==XFreq) return;
  if (Value>=0.001) XFreq=Value;
  sprintf(Text,"%.0lf",XFreq);
  Ctrl->Text=Text;
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
void TMainForm::WriteXTime0(double Time0)
{
  char Text[50];

  sprintf(Text,"%lf",Time0);
  WritePrivateProfileString("Axis","XTime0",Text,ConfigFile);
}

/*==========================================================================*/
/*!
  The user pressed on the enter key while typing the start time.

  \date
    \arg 2001-10-23 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::XTime0KeyPress(TObject *Sender, char &Key)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   Value=atof(Ctrl->Text.c_str());
   Time0=Value;
   sprintf(Text,"%.4lf",Time0);
   Ctrl->Text=Text;
   LastSGPoly=-1;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   WriteXTime0(Time0);
   }
}

/*==========================================================================*/
/*!
  The user pressed on the enter key while typing the start time.

  \date
    \arg 2001-12-13 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::XTime0Exit(TObject *Sender)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atof(Ctrl->Text.c_str());
  if (Value==Time0) return;
  Time0=Value;
  sprintf(Text,"%.4lf",Time0);
  Ctrl->Text=Text;
  LastSGPoly=-1;
  WriteXTime0(Time0);
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: GaussWidthCtrlKeyPress
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the width for
//              the gaussian smoothing.
//
// Usage: void __fastcall TMainForm::GaussWidthCtrlKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 23/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::GaussWidthCtrlKeyPress(TObject *Sender, char &Key)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   Value=atof(Ctrl->Text.c_str());
   if (Value>0.) GaussWidth=Value;
   sprintf(Text,"%.3lf",GaussWidth);
   Ctrl->Text=Text;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: GaussWidthCtrlExit
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the width for
//              the gaussian smoothing.
//
// Usage: void __fastcall TMainForm::GaussWidthCtrlExit(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 13/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::GaussWidthCtrlExit(TObject *Sender)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atof(Ctrl->Text.c_str());
  if (Value==GaussWidth) return;
  if (Value>0.) GaussWidth=Value;
  sprintf(Text,"%.3lf",GaussWidth);
  Ctrl->Text=Text;
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: GaussNeighCtrlKeyPress
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the number of
//              neigbourg for the gaussian smoothing.
//
// Usage: void __fastcall TMainForm::GaussNeighCtrlKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 23/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::GaussNeighCtrlKeyPress(TObject *Sender, char &Key)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   Value=atoi(Ctrl->Text.c_str());
   if (Value>0) GaussNeigh=Value;
   sprintf(Text,"%d",GaussNeigh);
   Ctrl->Text=Text;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: GaussNeighCtrlExit
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the number of
//              neigbourg for the gaussian smoothing.
//
// Usage: void __fastcall TMainForm::GaussNeighCtrlExit(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 13/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::GaussNeighCtrlExit(TObject *Sender)
{
  int Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atoi(Ctrl->Text.c_str());
  if (Value==GaussNeigh) return;
  if (Value>0) GaussNeigh=Value;
  sprintf(Text,"%d",GaussNeigh);
  Ctrl->Text=Text;
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: YGainCtrlKeyPress
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the Y gain.
//
// Usage: void __fastcall TMainForm::YGainCtrlKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 28/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::YGainCtrlKeyPress(TObject *Sender, char &Key)
{
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   YGain=atof(Ctrl->Text.c_str());
   sprintf(Text,"%lf",YGain);
   Ctrl->Text=Text;
   LastSGPoly=-1;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: YGainCtrlExit
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the Y gain.
//
// Usage: void __fastcall TMainForm::YGainCtrlExit(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 13/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::YGainCtrlExit(TObject *Sender)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atof(Ctrl->Text.c_str());
  if (Value==YGain) return;
  YGain=Value;
  sprintf(Text,"%lf",YGain);
  Ctrl->Text=Text;
  LastSGPoly=-1;
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: YOffsetCtrlKeyPress
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the Y offset.
//
// Usage: void __fastcall TMainForm::YOffsetCtrlKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 28/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::YOffsetCtrlKeyPress(TObject *Sender, char &Key)
{
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   YOffset=atof(Ctrl->Text.c_str());
   sprintf(Text,"%lf",YOffset);
   Ctrl->Text=Text;
   LastSGPoly=-1;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: YOffsetCtrlExit
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the Y offset.
//
// Usage: void __fastcall TMainForm::YOffsetCtrlExit(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 28/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::YOffsetCtrlExit(TObject *Sender)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atof(Ctrl->Text.c_str());
  if (Value==YOffset) return;
  YOffset=Value;
  sprintf(Text,"%lf",YOffset);
  Ctrl->Text=Text;
  LastSGPoly=-1;
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SavGolPolyKeyCtrlPress
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the polynomial
//              for the derivative.
//
// Usage: void __fastcall TMainForm::SavGolPolyKeyCtrlPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 29/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SavGolPolyCtrlKeyPress(TObject *Sender, char &Key)
{
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   SavGolPoly=atoi(Ctrl->Text.c_str());
   sprintf(Text,"%d",SavGolPoly);
   Ctrl->Text=Text;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SavGolPolyCtrlExit
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the polynomial
//              for the derivative.
//
// Usage: void __fastcall TMainForm::SavGolPolyCtrlExit(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 13/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SavGolPolyCtrlExit(TObject *Sender)
{
  double Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atoi(Ctrl->Text.c_str());
  if (Value==SavGolPoly) return;
  SavGolPoly=Value;
  sprintf(Text,"%d",SavGolPoly);
  Ctrl->Text=Text;
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SavGolNeighKeyPress
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the number
//              the neighbourg for the derivative.
//
// Usage: void __fastcall TMainForm::SavGolNeighKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 29/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SavGolNeighCtrlKeyPress(TObject *Sender, char &Key)
{
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  if (Key=='\r')
   {
   SavGolNeigh=atoi(Ctrl->Text.c_str());
   sprintf(Text,"%d",SavGolNeigh);
   Ctrl->Text=Text;
   UpdateGraphics();
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SavGolNeighExit
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user pressed on the enter key while typing the number
//              the neighbourg for the derivative.
//
// Usage: void __fastcall TMainForm::SavGolNeighExit(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 13/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SavGolNeighCtrlExit(TObject *Sender)
{
  int Value;
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;

  Value=atoi(Ctrl->Text.c_str());
  if (Value==SavGolNeigh) return;
  SavGolNeigh=Value;
  sprintf(Text,"%d",SavGolNeigh);
  Ctrl->Text=Text;
  UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SaveMenuClick
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user wants to save the curves.
//
// Usage: void __fastcall TMainForm::SaveMenuClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 29/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SaveMenuClick(TObject *Sender)
{
  if (!YData)
   {
   SaveDataMenu->Enabled=false;
   SaveDerivativeMenu->Enabled=false;
   SaveSmoothMenu->Enabled=false;
   return;
   }
  SaveDataMenu->Enabled=true;
  SaveSmoothMenu->Enabled=(Smooth!=NULL);
  SaveDerivativeMenu->Enabled=(Derive!=NULL);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SaveDerivativeMenuClick
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user wants to save the curves.
//
// Usage: void __fastcall TMainForm::SaveDerivativeMenuClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 29/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SaveDerivativeMenuClick(TObject *Sender)
{
  char FileName[_MAX_PATH];
  int i;
  bool Error;
  FILE *fo;
  double NextX,Slope,Offset,x,Bkgr;

  if (!Derive) return;
  if (!YDerv)
   {
   WriteMsg(__FILE__,__LINE__,"No derivative curve");
   return;
   }
  GetPrivateProfileString("Output","Derivative","",FileName,_MAX_PATH,ConfigFile);
  if (!FileName[0])
   {
   WritePrivateProfileString("Output","Derivative","derive.txt",ConfigFile);
   WriteMsg(__FILE__,__LINE__,"No output file in the config file");
   return;
   }
  fo=fopen(FileName,"wt");
  if (!fo)
   {
   WriteMsg(__FILE__,__LINE__,"Cannot save file %s",FileName);
   return;
   }
  Error=false;
  //NextX=Time0;
  //if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
  if (XFreq<0.) //get last point if X axes reverted
   NextX=XPlot[NPoints-1];
  else
   NextX=XPlot[0];
  for (i=0 ; i<NPoints && !Error ; i++)
   {
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
   Bkgr=Slope;
   if (fprintf(fo,"%lf\n",YDerv[i]-Bkgr)==EOF) Error=true;
   }
  fclose(fo);
  if (Error)
   WriteMsg(__FILE__,__LINE__,"Error while writting %s",FileName);
  else
   WriteMsg(__FILE__,__LINE__,"Derivative saved in %s",FileName);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SaveSmoothMenuClick
//
// Type: Function
//
// Applies To: TMainForm
//
// Description: The user wants to save the curves.
//
// Usage: void __fastcall TMainForm::SaveSmoothMenuClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 29/10/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TMainForm::SaveSmoothMenuClick(TObject *Sender)
{
  char FileName[_MAX_PATH];
  int i;
  bool Error;
  FILE *fo;
  double x,Slope,Offset,NextX,Bkgr;

  if (!YSmooth)
   {
   WriteMsg(__FILE__,__LINE__,"No smoothed curve");
   return;
   }
  GetPrivateProfileString("Output","Smooth","",FileName,_MAX_PATH,ConfigFile);
  if (!FileName[0])
   {
   WritePrivateProfileString("Output","Smooth","smooth.txt",ConfigFile);
   WriteMsg(__FILE__,__LINE__,"No output file in the config file");
   return;
   }
  fo=fopen(FileName,"wt");
  if (!fo)
   {
   WriteMsg(__FILE__,__LINE__,"Cannot save file %s",FileName);
   return;
   }
  Error=false;
  //NextX=Time0;
  //if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
  if (XFreq<0.) //get last point if X axes reverted
   NextX=XPlot[NPoints-1];
  else
   NextX=XPlot[0];
  for (i=0 ; i<NPoints && !Error ; i++)
   {
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
   if (fprintf(fo,"%lf\n",YSmooth[i]-Bkgr)==EOF) Error=true;
   }
  fclose(fo);
  if (Error)
   WriteMsg(__FILE__,__LINE__,"Error while writting %s",FileName);
  else
   WriteMsg(__FILE__,__LINE__,"Smoothed curve saved in %s",FileName);
}

/*==========================================================================*/
/*!
  The user wants to save the curves.

  \date
    \arg 2001-10-29 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::SaveDataMenuClick(TObject *Sender)
{
  char FileName[_MAX_PATH],*Ptr;
  int i;
  FILE *fo;

  if (!XPlot || !YPlot)
   {
   WriteMsg(__FILE__,__LINE__,"No data to save");
   return;
   }
  strcpy(FileName,DefaultFileName.c_str());
  Ptr=strrchr(FileName,'.');
  if (!Ptr) Ptr=FileName+strlen(FileName);
  strcpy(Ptr,".txt");
  SaveDialog->FileName=FileName;
  SaveDialog->Filter="Data (*.txt)|*.txt";
  if (!SaveDialog->Execute()) return;
  fo=fopen(SaveDialog->FileName.c_str(),"wt");
  if (!fo)
   {
   WriteMsg(__FILE__,__LINE__,"Cannot open output file for writting");
   return;
   }
  for (i=0 ; i<NPoints ; i++) fprintf(fo,"%lf\t%lf\n",XPlot[i],YPlot[i]);
  fclose(fo);
}

/*==========================================================================*/
/*!
  The user wants to display or hide the dialog to edit the background.

  \date
    \arg 2001-11-22 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::BackgroundMenuClick(TObject *Sender)
{
  if (BackgroundForm->Visible)
   {
   BackgroundForm->Hide();
   }
  else
   {
   BackgroundForm->Show();
   BackgroundForm->SetFocus();
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
void TMainForm::BkgrLeftClick(void *Parent,double x,double y,Classes::TShiftState Shift)
{
  TMainForm *ParentPtr;

  ParentPtr=(TMainForm *)Parent;
  BackgroundForm->AddPoint(x,y);
  if (BackgroundForm->PrepareBkgr(ParentPtr->Time0,ParentPtr->XFreq,ParentPtr->NPoints))
   ParentPtr->MainGraphic->SetGraphic(2,BackgroundForm->XBkgr,BackgroundForm->YBkgr,BackgroundForm->NBgPoints);
  else
   ParentPtr->MainGraphic->DeleteCurve(2);
  ParentPtr->UpdateGraphics();
}

/*==========================================================================*/
/*!
  Function called when the user click on the main graphic and the
  background window is visible.

  \date
    \arg 2001-11-22 created by Frederic
 */
/*==========================================================================*/
void TMainForm::BkgrRightClick(void *Parent,double x,double y,Classes::TShiftState Shift)
{
  TMainForm *ParentPtr;

  ParentPtr=(TMainForm *)Parent;
  BackgroundForm->RemovePoint(x,y);
  if (BackgroundForm->PrepareBkgr(ParentPtr->Time0,ParentPtr->XFreq,ParentPtr->NPoints))
   ParentPtr->MainGraphic->SetGraphic(2,BackgroundForm->XBkgr,BackgroundForm->YBkgr,BackgroundForm->NBgPoints);
  else
   ParentPtr->MainGraphic->DeleteCurve(2);
  ParentPtr->UpdateGraphics();
}

/*==========================================================================*/
/*!
  Display the dialog box to select the output files.

  \date
    \arg 2001-12-01 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::SelectOutputFileMenuClick(TObject *Sender)
{
  TOutputFile *OutFile;

  OutFile=new TOutputFile(this);
  if (!OutFile)
   {
   WriteMsg(__FILE__,__LINE__,"Cannot open dialog box");
   return;
   }
  OutFile->ShowModal();
  delete OutFile;
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
bool TMainForm::WriteToClipboard(char * Text)
{
  int Size;
  HGLOBAL hBuff;
  char *Buff;

  Size=strlen(Text)+1;  //sizeof(Form)+3*15+MAXFILE+10;
  if ((hBuff=GlobalAlloc(GHND,Size))==NULL)
   {
   WriteMsg(__FILE__,__LINE__,"Not enough memory");
   return(false);
   }
  Buff=(char *)GlobalLock(hBuff);
  if (!OpenClipboard(Application->Handle))
   {
   GlobalUnlock(hBuff);
   GlobalFree(hBuff);
   WriteMsg(__FILE__,__LINE__,"Unable to open the clipboard");
   return(false);
   }
  EmptyClipboard();
  memcpy(Buff,Text,strlen(Text)+1);
  GlobalUnlock(hBuff);
  SetClipboardData(CF_TEXT,hBuff);
  CloseClipboard();
  return(true);
}

/*==========================================================================*/
/*!
  Copy the data curve to the clipboard.

  \date 2001-12-04
 */
/*==========================================================================*/
void __fastcall TMainForm::CopyDataClick(TObject *Sender)
{
  char *Buff,*TBuff;
  long i;
  double x,Bkgr,NextX,Slope,Offset;

  //***** réserver la mémoire *****
  i=NPoints*(2L*12L+6L)+1L;
  Buff=(char *)malloc(i);
  if (!Buff)
   {
   WriteMsg(__FILE__,__LINE__,"Not enough memory to copy the data");
   return;
   }

  //***** écrire les données *****
  TBuff=Buff;
  TBuff=Buff;
  //NextX=Time0;
  //if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
  if (XFreq<0.) //get last point if X axes reverted
   NextX=XPlot[NPoints-1];
  else
   NextX=XPlot[0];
  for (i=0 ; i<NPoints ; i++)
   {
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
   TBuff+=sprintf(TBuff,"%11.5g\t%11.5g\r\n", XPlot[i], YPlot[i]-Bkgr);
   }

  WriteToClipboard(Buff);
  free(Buff);
  return;
}

/*==========================================================================*/
/*!
  Copy the background curve to the clipboard.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::CopyBkgrClick(TObject *Sender)
{
  char *Buff,*TBuff;
  long i;
  double x,Bkgr,NextX,Slope,Offset;

  //***** réserver la mémoire *****
  i=NPoints*(2L*12L+6L)+1L;
  Buff=(char *)malloc(i);
  if (!Buff)
   {
   WriteMsg(__FILE__,__LINE__,"Not enough memory to copy the data");
   return;
   }

  //***** écrire les données *****
  TBuff=Buff;
  //NextX=Time0;
  //if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
  if (XFreq<0.) //get last point if X axes reverted
   NextX=XPlot[NPoints-1];
  else
   NextX=XPlot[0];
  for (i=0 ; i<NPoints ; i++)
   {
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
   TBuff+=sprintf(TBuff,"%11.5g\t%11.5g\r\n", XPlot[i], Bkgr);
   }

  WriteToClipboard(Buff);
  free(Buff);
  return;
}

/*==========================================================================*/
/*!
  Copy the smoothed curve to the clipboard.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::CopySmoothMenuClick(TObject *Sender)
{
  char *Buff,*TBuff;
  long i;
  double x,Bkgr,NextX,Slope,Offset;

  if (!YSmooth)
   {
   WriteMsg(__FILE__,__LINE__,"No smoothed curve");
   return;
   }

  //***** réserver la mémoire *****
  i=NPoints*(2L*12L+6L)+1L; //2 numbers of 12 characters + 1 tab + 2 CRLF
  Buff=(char *)malloc(i);
  if (!Buff)
   {
   WriteMsg(__FILE__,__LINE__,"Not enough memory to copy the data");
   return;
   }

  //***** écrire les données *****
  TBuff=Buff;
  //NextX=Time0;
  //if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
  if (XFreq<0.) //get last point if X axes reverted
   NextX=XPlot[NPoints-1];
  else
   NextX=XPlot[0];
  for (i=0 ; i<NPoints ; i++)
   {
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
   TBuff+=sprintf(TBuff,"%11.5g\t%11.5g\r\n", XPlot[i], YSmooth[i]-Bkgr);
   }

  WriteToClipboard(Buff);
  free(Buff);
  return;
}

/*==========================================================================*/
/*!
  Copy the derivative curve to the clipboard.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::CopyDeriveMenuClick(TObject *Sender)
{
  char *Buff,*TBuff;
  long i;
  double x,Bkgr,NextX,Slope,Offset;

  if (!YDerv)
   {
   WriteMsg(__FILE__,__LINE__,"No derivative curve");
   return;
   }

  //***** réserver la mémoire *****
  i=NPoints*(2L*12L+6L)+1L;
  Buff=(char *)malloc(i);
  if (!Buff)
   {
   WriteMsg(__FILE__,__LINE__,"Not enough memory to copy the data");
   return;
   }

  //***** écrire les données *****
  TBuff=Buff;
  //NextX=Time0;
  //if (XFreq<0.) NextX+=(double)(NPoints-1)/XFreq;  //get last point if X axes reverted
  if (XFreq<0.) //get last point if X axes reverted
   NextX=XPlot[NPoints-1];
  else
   NextX=XPlot[0];
  for (i=0 ; i<NPoints ; i++)
   {
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
   Bkgr=Slope;
   TBuff+=sprintf(TBuff,"%11.5g\t%11.5g\r\n", XPlot[i], YDerv[i]-Bkgr);
   }

  WriteToClipboard(Buff);
  free(Buff);
  return;
}

/*==========================================================================*/
/*!
  Enable the copy menu items according to the data available in memory.

  \date
    \arg 2001-12-04 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::CopyMenuClick(TObject *Sender)
{
  CopyData->Enabled=(YPlot!=NULL);
  CopyBkgrMenu->Enabled=(YPlot!=NULL && BackgroundForm->BackgroundOk);
  CopySmoothMenu->Enabled=(YSmooth!=NULL);
  CopyDeriveMenu->Enabled=(YDerv!=NULL);
}

void __fastcall TMainForm::RawSmoothButtonClick(TObject *Sender)
{
  UpdateGraphics();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::SavGolButtonClick(TObject *Sender)
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
void __fastcall TMainForm::XTrackerExit(TObject *Sender)
{
  TrackPosition(true);
}

/*==========================================================================*/
/*!
  The user pressed a key in the X tracker control. If the key is the enter key,
  we update the tracked position.

  \date 2003-03-22
 */
/*==========================================================================*/
void __fastcall TMainForm::XTrackerKeyPress(TObject *Sender, char &Key)
{
  if (Key=='\r')
   {
   TrackPosition(true);
   Key=0;
   }
}

/*==========================================================================*/
/*!
  The user is removing the focus out of the tracker control. We update the
  tracked position.

  \date 2003-03-22
 */
/*==========================================================================*/
void __fastcall TMainForm::YTrackerExit(TObject *Sender)
{
  TrackPosition(false);
}

/*==========================================================================*/
/*!
  The user pressed a key in the Y tracker control. If the key is the enter key,
  we update the tracked position.

  \date 2003-03-22
 */
/*==========================================================================*/
void __fastcall TMainForm::YTrackerKeyPress(TObject *Sender, char &Key)
{
  if (Key=='\r')
   {
   TrackPosition(false);
   Key=0;
   }
}

/*==========================================================================*/
/*!
  Display the X or Y position corresponding to the given coordinate.

  \param XSource If true, then we track the X position. If false, we track
         the Y position.

  \date 2003-03-22
 */
/*==========================================================================*/
void TMainForm::TrackPosition(bool XSource)
{
  int i;
  double XPosition,YPosition;
  double *XPtr,*YPtr,*XSrc,*YSrc;
  char Text[30];
  bool Found;

  LastTrackSrc=XSource;

  //***** get the source *****
  if (TrackSmooth->Checked)
   {
   XSrc=XPlot;
   YSrc=YSmooth;
   }
  else if (TrackDerv->Checked)
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
   if (XTracker->Text.IsEmpty())
    {
    YTracker->Text="";
    return;
    }
   XPosition=atof(XTracker->Text.c_str());
   XPtr=XSrc;
   YPtr=YSrc;
   }
  else
   {
   if (YTracker->Text.IsEmpty())
    {
    XTracker->Text="";
    return;
    }
   XPosition=atof(YTracker->Text.c_str());
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
   if (Found) sprintf(Text,"%.7lg",YPosition);
   YTracker->Text=Text;
   }
  else
   {
   if (Found) sprintf(Text,"%.4lg",YPosition);
   XTracker->Text=Text;
   }
}

/*=============================================================================*/
/*!
  Control to track the position on the data.
 */
/*=============================================================================*/
void __fastcall TMainForm::TrackDataClick(TObject *Sender)
{
  TrackData->Checked=true;
  TrackSmooth->Checked=false;
  TrackDerv->Checked=false;
  TrackMouse->Checked=false;
  MainGraphic->MouseMoveCallback=NULL;
  DervGraphic->MouseMoveCallback=NULL;
  TrackPosition(LastTrackSrc);
}

/*=============================================================================*/
/*!
  Control to track the position on the smoothed curve.
 */
/*=============================================================================*/
void __fastcall TMainForm::TrackSmoothClick(TObject *Sender)
{
  TrackData->Checked=false;
  TrackSmooth->Checked=true;
  TrackDerv->Checked=false;
  TrackMouse->Checked=false;
  MainGraphic->MouseMoveCallback=NULL;
  DervGraphic->MouseMoveCallback=NULL;
  TrackPosition(LastTrackSrc);
}

/*=============================================================================*/
/*!
  Control to track the position on the derivative.
 */
/*=============================================================================*/
void __fastcall TMainForm::TrackDervClick(TObject *Sender)
{
  TrackData->Checked=false;
  TrackSmooth->Checked=false;
  TrackDerv->Checked=true;
  TrackMouse->Checked=false;
  MainGraphic->MouseMoveCallback=NULL;
  DervGraphic->MouseMoveCallback=NULL;
  TrackPosition(LastTrackSrc);
}

/*=============================================================================*/
/*!
  Control to track the position of the mouse cursor.
 */
/*=============================================================================*/
void __fastcall TMainForm::TrackMouseClick(TObject *Sender)
{
  TrackData->Checked=false;
  TrackSmooth->Checked=false;
  TrackDerv->Checked=false;
  TrackMouse->Checked=true;
  MainGraphic->MouseMoveCallback=TrackMouseMove;
  DervGraphic->MouseMoveCallback=TrackMouseMove;
  //TrackPosition(LastTrackSrc);
}

/*==========================================================================*/
/*!
  The user wants to cut some data points and only leave the center part of
  the curve. A dialog box is displayed to ask the range to keep.

  \date
    \arg 2003-03-24 created by Frederic
 */
/*==========================================================================*/
void __fastcall TMainForm::CutMenuClick(TObject *Sender)
{
  TXRangeForm *XRangeForm;
  TModalResult Result;
  double XStart,XStop,StartTime;
  double *TData;
  int i,j;
  char Text[40];

  if (!YData) return;
  GetPrivateProfileString("Cut","XStart","0.",Text,sizeof(Text),ConfigFile);
  XStart=atof(Text);
  GetPrivateProfileString("Cut","XStop","0.",Text,sizeof(Text),ConfigFile);
  XStop=atof(Text);
  XRangeForm=new TXRangeForm(this);
  if (!XRangeForm) return;
  XRangeForm->XStart=XStart;
  XRangeForm->XStop=XStop;
  Result=XRangeForm->ShowModal();
  if (Result==mrOk)
   {
   XStart=XRangeForm->XStart;
   XStop=XRangeForm->XStop;
   }
  delete XRangeForm;
  if (Result!=mrOk) return;
  if (XStart==XStop)
   {
   WriteMsg(__FILE__,__LINE__,"XStart and XStop must be different");
   return;
   }
  if (!XData)
   {
   XData=(double *)malloc(NPoints*sizeof(double));
   if (!XData)
    {
    WriteMsg(__FILE__,__LINE__,"Not enough memory to store XData");
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
   WriteMsg(__FILE__,__LINE__,"Those limite would remove all the data points. Cut not performed");
   return;
   }
  NPoints=j;
  Time0=StartTime;
  sprintf(Text,"%.4lf",Time0);
  XTime0->Text=Text;
  HasBeenCut=true;

  sprintf(Text,"%lf",XStart);
  WritePrivateProfileString("Cut","XStart",Text,ConfigFile);
  sprintf(Text,"%lf",XStop);
  WritePrivateProfileString("Cut","XStop",Text,ConfigFile);

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

  MainGraphic->DeleteAllCurves();
  DervGraphic->DeleteAllCurves();
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
void TMainForm::TrackMouseMove(void *Parent,bool InGraph,double x,double y)
{
  char Text[40];
  TMainForm *ParentPtr;

  ParentPtr=(TMainForm *)Parent;
  if (!ParentPtr->TrackMouse->Checked) return;
  if (InGraph)
   {
   sprintf(Text,"%.7lg",x);
   ParentPtr->XTracker->Text=Text;
   sprintf(Text,"%.4lg",y);
   ParentPtr->YTracker->Text=Text;
   }
  else
   {
   ParentPtr->XTracker->Text="";
   ParentPtr->YTracker->Text="";
   }
}

/*=============================================================================*/
/*!
  Resize the graphics when the panel is resized.

  \date 2005-01-17
 */
/*=============================================================================*/
void __fastcall TMainForm::FormResize(TObject *Sender)
{
  int GHeight,GWidth;

  if (ExitProgram) return;
  GHeight=(ClientHeight-20)/2;
  GWidth=ClientWidth-MainGraphCtrl->Left-5;
  MainGraphCtrl->SetBounds(MainGraphCtrl->Left,5,GWidth,GHeight);
  SetGraphSize(MainGraphCtrl,GWidth,GHeight);
  MainGraphic->ResizeGraph();

  GWidth=ClientWidth-DervGraphCtrl->Left-5;
  DervGraphCtrl->SetBounds(DervGraphCtrl->Left,15+GHeight,GWidth,GHeight);
  SetGraphSize(DervGraphCtrl,GWidth,GHeight);
  DervGraphic->ResizeGraph();
}

/*=============================================================================*/
/*!
  Set the size of an image.

  \date 2005-01-18
 */
/*=============================================================================*/
void TMainForm::SetGraphSize(TImage *Control,int Width,int Height)
{
  /*Graphics::TBitmap *TempBmp=new Graphics::TBitmap();
  TempBmp->PixelFormat=pfDevice;
  TempBmp->Width=Width;
  TempBmp->Height=Height;
  TPicture *TempPict=new TPicture();
  TempPict->Assign(TempBmp);
  Control->Picture=TempPict;
  delete TempBmp;*/
  if (Width>10 && Height>10)
   {
   Control->Picture->Bitmap->Width=Width;
   Control->Picture->Bitmap->Height=Height;
   }
}

/*=============================================================================*/
/*!
  The main form is closing.
 */
/*=============================================================================*/
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  ExitProgram=true;
  MainGraphic->DeleteAllCurves();
  DervGraphic->DeleteAllCurves();
}
//---------------------------------------------------------------------------

