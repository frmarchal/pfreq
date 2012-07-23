//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>
#include "Background.h"
#include "utils.h"
#include "MainScreen.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBackgroundForm *BackgroundForm;

extern TMainForm *MainForm;
extern char ConfigFile[_MAX_PATH];  //configuration file for this program

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: TBackgroundForm
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Constructor of the dialog box to handle the background.
//
// Usage: __fastcall TBackgroundForm::TBackgroundForm(TComponent* Owner)
//    : TForm(Owner)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 22/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
__fastcall TBackgroundForm::TBackgroundForm(TComponent* Owner)
    : TForm(Owner)
{
  NPoints=0;
  Slope=0.;
  Offset=0.;
  AutoDefined=false;
  BackgroundOk=false;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: FormCreate
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The form is being create. We restore the default options
//
// Usage: void __fastcall TBackgroundForm::FormCreate(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 6/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::FormCreate(TObject *Sender)
{
  char Text[20];
  double dVal;
  int iVal;

  GetPrivateProfileString("Background","LRFirst","0.0",Text,sizeof(Text),ConfigFile);
  dVal=atof(Text);
  sprintf(Text,"%.1lf%%",dVal);
  LRFirst->Text=Text;
  GetPrivateProfileString("Background","LRLast","10.0",Text,sizeof(Text),ConfigFile);
  dVal=atof(Text);
  sprintf(Text,"%.1lf%%",dVal);
  LRLast->Text=Text;

  GetPrivateProfileString("Background","AverageStart","0",Text,sizeof(Text),ConfigFile);
  iVal=atoi(Text);
  sprintf(Text,"%d",iVal);
  AverageStart->Text=Text;
  GetPrivateProfileString("Background","AverageNPoints","100",Text,sizeof(Text),ConfigFile);
  iVal=atoi(Text);
  sprintf(Text,"%d",iVal);
  AverageNPoints->Text=Text;

  GetPrivateProfileString("Background","Type","1",Text,sizeof(Text),ConfigFile);
  iVal=atoi(Text);
  if (iVal==0)
   {
   AverageButton->Checked=false;
   LinearRegButton->Checked=true;
   }
  else if (iVal==1)
   {
   LinearRegButton->Checked=false;
   AverageButton->Checked=true;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: AddPoint
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Add a point to the background.
//
// Usage: void TBackgroundForm::AddPoint(double x, double y)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 22/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void TBackgroundForm::AddPoint(double x, double y)
{
  TListItem *ListItem;
  char Text[30];
  int i,j;

  if (BackgroundMode->ActivePage==AutomaticSheet) return;
  if (NPoints>=MAX_BKGRPOINTS) return;
  AutoDefined=false;  //disable automatic background
  BackgroundOk=false;
  for (i=0 ; i<NPoints && x>XData[i] ; i++);
  if (i<NPoints && fabs(y-YData[i])<1e20*fabs(x-XData[i]))
   {                                   //add the point only if the resoluting slope is not too big
   for (j=NPoints ; j>i ; j--)         //in the other case, replace the existing point
    {
    XData[j]=XData[j-1];
    YData[j]=YData[j-1];
    }
   }
  XData[i]=x;
  YData[i]=y;
  NPoints++;
  ListItem=BkgrPointsList->Items->Insert(i);
  sprintf(Text,"%.3lg",x);
  ListItem->Caption=Text;
  sprintf(Text,"%.3lg",y);
  ListItem->SubItems->Add(Text);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: RemovePoint
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Remove a point to the background.
//
// Usage: void TBackgroundForm::RemovePoint(double x, double y)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 22/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void TBackgroundForm::RemovePoint(double x, double y)
{
  int i,IMin;
  double dMin,d;

  if (BackgroundMode->ActivePage==AutomaticSheet) return;
  if (NPoints<=0) return;
  AutoDefined=false;  //disable automatic background
  BackgroundOk=false;
  IMin=-1;
  dMin=1e20;
  for (i=0 ; i<NPoints ; i++)
   {
   d=hypot(XData[i]-x,YData[i]-y);
   if (dMin>d)
    {
    dMin=d;
    IMin=i;
    }
   }
  if (IMin>=0)
   {
   NPoints--;
   for (i=IMin ; i<NPoints ; i++)
    {
    XData[i]=XData[i+1];
    YData[i]=YData[i+1];
    }
   BkgrPointsList->Items->Delete(IMin);
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: PrepareBkgr
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Prepare the background and make available the points to draw
//              in XBkgr, YBkgr and NBgPoints.
//
// Usage: bool TBackgroundForm::PrepareBkgr(double Time0, double XFreq,int NPts)
//
// Returns: true if the data can be read from XBkgr, YBkgr, NBgPoints, or false
//          if no background is available.
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
bool TBackgroundForm::PrepareBkgr(double Time0, double XFreq,int NPts)
{
  int i,j;
  double XMin,XMax;

  //***** test parameters and calculate min and max of X axes *****
  if (NPts<2) return(false);
  if ((double)(NPts-1)>=1e20*fabs(XFreq)) return(false);  //no background if XFreq is too small to calculate accurately XMax
  if (XFreq>0.)
   {
   XMin=Time0;
   XMax=(double)(NPts-1)/XFreq+Time0;
   }
  else
   {
   XMax=Time0;
   XMin=(double)(NPts-1)/XFreq+Time0;
   }

  //***** for automatic background *****
  if (BackgroundMode->ActivePage==AutomaticSheet)
   {
   if (!AutoDefined) return(false);
   XBkgr[0]=XMin;
   YBkgr[0]=XMin*Slope+Offset;
   XBkgr[1]=XMax;
   YBkgr[1]=XMax*Slope+Offset;
   NBgPoints=2;
   BackgroundOk=true;
   return(true);
   }

  //***** for manual background *****
  if (NPoints<2) return(false);
  j=0;
  if (XData[0]>XMin)
   {
   XBkgr[j]=XMin;
   YBkgr[j]=(XMin-XData[0])/(XData[1]-XData[0])*(YData[1]-YData[0])+YData[0];
   j=1;
   }
  for (i=0 ; i<NPoints ; i++ , j++)
   {
   XBkgr[j]=XData[i];
   YBkgr[j]=YData[i];
   }
  if (XData[--i]<XMax)
   {
   XBkgr[j]=XMax;
   YBkgr[j]=(XMax-XData[i])/(XData[i-1]-XData[i])*(YData[i-1]-YData[i])+YData[i];
   j++;
   }
  NBgPoints=j;
  BackgroundOk=true;
  return(true);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: GetNextSlope
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Get the slope of the background position x. x is updated to contains
//              the next X position when the background slope will change.
//
// Usage: double TBackgroundForm::GetNextSlope(double * x)
//
// Returns: The slope of the background. Note that if there is no background the slope
//          will be zero.
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
double TBackgroundForm::GetNextSlope(double * x)
{
  int i;
  double LocSlope;

  //***** automatic background *****
  if (BackgroundMode->ActivePage==AutomaticSheet)
   {
   *x=MAXDOUBLE;
   if (!AutoDefined) return(0.);
   return(Slope);
   }

  //***** manual background *****
  if (NPoints<2)
   {
   *x=MAXDOUBLE;
   return(0.);
   }
  for (i=1 ; i<NPoints ; i++)
   {
   LocSlope=(YData[i]-YData[i-1])/(XData[i]-XData[i-1]);
   if (*x<XData[i])
    {
    *x=XData[i];
    return(LocSlope);
    }
   }
  *x=MAXDOUBLE;
  return(LocSlope);
}

/*=============================================================================*/
/*!
  Get the value of the background at position x. x is updated to contains
  the next position at which the background changes.

  \param XFreq The distance between two X positions of the data.
  \param x The X position to compute. It is replaced by the next X position of
           a change in the background. Therefore, you should call this function
           again when you reach that X.
  \param LocSlope A variable to store the slope of the background from the x
         position until the next x position.
  \param LocOffset A variable to store the offset of the background from the x
         position until the next x position.

  \author F Marchal
  \date 2001-12-01
 */
/*=============================================================================*/
void TBackgroundForm::GetBackground(double XFreq,double *x,double *LocSlope,double *LocOffset)
{
	int i;

	//***** automatic background *****
	if (BackgroundMode->ActivePage==AutomaticSheet)
	{
		*x=(XFreq>=0.) ? MAXDOUBLE : -MAXDOUBLE;
		if (AutoDefined)
		{
			*LocSlope=Slope;
			*LocOffset=Offset;
		}
    else
    {
    	*LocSlope=0.;
      *LocOffset=0.;
    }
		return;
	}

	//***** manual background *****
	if (NPoints<2)
	{
		*x=(XFreq>=0.) ? MAXDOUBLE : -MAXDOUBLE;
		*LocSlope=0.;
		*LocOffset=0.;
		return;
	}
	if (XFreq>=0.)
	{
		if (*x<XData[0])
		{
			*x=XData[0];
			*LocSlope=(YData[1]-YData[0])/(XData[1]-XData[0]);
			*LocOffset=YData[0]-XData[0]*(*LocSlope);
			return;
		}
		for (i=1 ; i<NPoints ; i++)
		{
			*LocSlope=(YData[i]-YData[i-1])/(XData[i]-XData[i-1]);
			if (*x<XData[i] && *x>=XData[i-1])
			{
				*x=XData[i];
				*LocOffset=YData[i-1]-XData[i-1]*(*LocSlope);
				return;
			}
		}
	}
	else
	{
		if (*x<XData[NPoints-1])
		{
			*x=XData[NPoints-1];
			*LocSlope=(YData[NPoints-1]-YData[NPoints-2])/(XData[NPoints-1]-XData[NPoints-2]);
			*LocOffset=YData[NPoints-1]-XData[NPoints-1]*(*LocSlope);
			return;
		}
		for (i=NPoints-1 ; i>0 ; i--)
		{
			*LocSlope=(YData[i]-YData[i-1])/(XData[i]-XData[i-1]);
			if (*x>XData[i-1] && *x<=XData[i])
			{
				*x=XData[i-1];
				*LocOffset=YData[i-1]-XData[i-1]*(*LocSlope);
				return;
			}
		}
	}
	*x=(XFreq>=0.) ? MAXDOUBLE : -MAXDOUBLE;
	*LocOffset=YData[i-1]-XData[i-1]*(*LocSlope);
	return;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: CalcAutoButtonClick
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The user wants to calculate the automatic background.
//
// Usage: void __fastcall TBackgroundForm::CalcAutoButtonClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 2/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::CalcAutoButtonClick(TObject *Sender)
{
  AutoDefined=true;
  BackgroundOk=false;
  if (CalculateAutoBackground()) MainForm->UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user wants to invalidate the automatic background.

  \author F Marchal
  \date 2001-12-02
 */
/*=============================================================================*/
void __fastcall TBackgroundForm::DeleteAutoButtonClick(TObject *Sender)
{
  AutoDefined=false;
  BackgroundOk=false;
  MainForm->UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: LRFirstKeyPress
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The user press enter in the control.
//
// Usage: void __fastcall TBackgroundForm::LRFirstKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 2/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::LRFirstKeyPress(TObject *Sender, char &Key)
{
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;
  double Percent;

  if (Key=='\r')
   {
   Percent=atof(Ctrl->Text.c_str());
   if (Percent<0.) Percent=0.;
   else if (Percent>100.) Percent=100.;
   sprintf(Text,"%.1lf%%",Percent);
   Ctrl->Text=Text;
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   BackgroundOk=false;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: AverageKeyPress
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The user press enter in the control.
//
// Usage: void __fastcall TBackgroundForm::AverageKeyPress(TObject *Sender, char &Key)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 6/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::AverageKeyPress(TObject *Sender, char &Key)
{
  char Text[20];
  TEdit *Ctrl=(TEdit *)Sender;
  int Value;

  if (Key=='\r')
   {
   Value=atoi(Ctrl->Text.c_str());
   if (Value<0) Value=0;
   sprintf(Text,"%d",Value);
   Ctrl->Text=Text;
   Ctrl->SelStart=0;
   Ctrl->SelLength=-1;
   Key=0;
   BackgroundOk=false;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: CalculateAutoBackground
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Calculate the background if the automatic mode is enabled.
//
// Usage: bool TBackgroundForm::CalculateAutoBackground(void)
//
// Returns: true if the main window should be redraw, or false if nothing has to be
//          changed in the graphic.
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 3/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
bool TBackgroundForm::CalculateAutoBackground(void)
{
  int i,NPts,i0,i1;
  double sx,sy,sxx,sxy;
  double a,b,den,*XdData,*YdData,nx;
  char Text[20];

  if (BackgroundMode->ActivePage==ManualSheet) return(false);
  if (!AutoDefined) return(false);

  if (LinearRegButton->Checked)
   {
   if (!MainForm || !MainForm->XPlot || !MainForm->YPlot || MainForm->NPoints<2)
    {
    WriteMsg(__FILE__,__LINE__,"No data to calculate the linear regression");
    return(false);
    }
   XdData=MainForm->XPlot;
   YdData=MainForm->YPlot;
   NPts=MainForm->NPoints;
   i0=(int)((NPts-1)*atof(LRFirst->Text.c_str())/100.);
   i1=(int)((NPts-1)*atof(LRLast->Text.c_str())/100.);
   if (i0<0 || i1>=NPts || i1-i0<2)
    {
    WriteMsg(__FILE__,__LINE__,"Invalid range for the linear regression");
    return(false);
    }
   sx=sxx=sy=sxy=0.;
   for (i=i0 ; i<=i1 ; i++)
    {
    sx+=XdData[i];
    sxx+=XdData[i]*XdData[i];
    sy+=YdData[i];
    sxy+=XdData[i]*YdData[i];
    }
   nx=(double)(i1-i0);
   a=nx*sxy-sx*sy;
   b=sxx*sy-sx*sxy;
   den=nx*sxx-sx*sx;
   if (fabs(a)>=1e6*fabs(den) || fabs(b)>=256.*fabs(den))
    {
    WriteMsg(__FILE__,__LINE__,"Cannot calculate linear regression");
    return(false);
    }
   Slope=a/den;
   Offset=b/den;
   AutoDefined=true;
   BackgroundOk=false;

   //***** save options to the configuration file *****
   a=atof(LRFirst->Text.c_str());
   sprintf(Text,"%.1lf",a);
   WritePrivateProfileString("Background","LRFirst",Text,ConfigFile);
   a=atof(LRLast->Text.c_str());
   sprintf(Text,"%.1lf",a);
   WritePrivateProfileString("Background","LRLast",Text,ConfigFile);
   WritePrivateProfileString("Background","Type","0",ConfigFile);
   return(true);
   }

  if (AverageButton->Checked)
   {
   if (!MainForm || !MainForm->XPlot || !MainForm->YPlot || MainForm->NPoints<2)
    {
    WriteMsg(__FILE__,__LINE__,"No data to calculate the average");
    return(false);
    }
   XdData=MainForm->XPlot;
   YdData=MainForm->YPlot;
   NPts=MainForm->NPoints;
   i0=atoi(AverageStart->Text.c_str());
   i1=i0+atoi(AverageNPoints->Text.c_str());
   if (i0<0 || i1>=NPts || i1-i0<2)
    {
    WriteMsg(__FILE__,__LINE__,"Invalid range for the average");
    return(false);
    }
   sy=0.;
   for (i=i0 ; i<=i1 ; i++)
    {
    sy+=YdData[i];
    }
   nx=(double)(i1-i0);
   Slope=0.;
   Offset=sy/nx;
   AutoDefined=true;
   BackgroundOk=false;

   //***** save options to the configuration file *****
   i0=atoi(AverageStart->Text.c_str());
   sprintf(Text,"%d",i0);
   WritePrivateProfileString("Background","AverageStart",Text,ConfigFile);
   i1=atof(AverageNPoints->Text.c_str());
   sprintf(Text,"%d",i1);
   WritePrivateProfileString("Background","AverageNPoints",Text,ConfigFile);
   WritePrivateProfileString("Background","Type","1",ConfigFile);
   return(true);
   }

  return(false);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: DeleteAllButtonClick
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: Delete all points in the manual background.
//
// Usage: void TBackgroundForm::DeleteAllButtonClick(void)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 3/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::DeleteAllButtonClick(TObject *Sender)
{
  NPoints=0;
  Slope=0.;
  Offset=0.;
  AutoDefined=false;
  BackgroundOk=false;
  BkgrPointsList->Items->Clear();
  MainForm->UpdateGraphics();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: BackgroundModeChange
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The user toggle between the automatic background and the manual
//              background.
//
// Usage: void __fastcall TBackgroundForm::BackgroundModeChange(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 31/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::BackgroundModeChange(TObject *Sender)
{
  if (BackgroundMode->ActivePage==ManualSheet)
   {
   MainForm->MainGraphic->LeftMouseClick=MainForm->BkgrLeftClick;
   MainForm->MainGraphic->RightMouseClick=MainForm->BkgrRightClick;
   }
  else
   {
   MainForm->MainGraphic->LeftMouseClick=NULL;
   MainForm->MainGraphic->RightMouseClick=NULL;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: FormHide
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The background window is hidden.
//
// Usage: void __fastcall TBackgroundForm::FormHide(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 31/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::FormHide(TObject *Sender)
{
  MainForm->MainGraphic->LeftMouseClick=NULL;
  MainForm->MainGraphic->RightMouseClick=NULL;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: FormShow
//
// Type: Function
//
// Applies To: TBackgroundForm
//
// Description: The background window is visible again.
//
// Usage: void __fastcall TBackgroundForm::FormShow(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 31/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TBackgroundForm::FormShow(TObject *Sender)
{
  if (BackgroundMode->ActivePage==ManualSheet)
   {
   MainForm->MainGraphic->LeftMouseClick=MainForm->BkgrLeftClick;
   MainForm->MainGraphic->RightMouseClick=MainForm->BkgrRightClick;
   }
}

