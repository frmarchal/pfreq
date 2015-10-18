#include <math.h>
#include <limits>
#include "mainscreen.h"
#include "background.h"
#include "ui_background.h"
#include "Utils.h"
#include "config.h"

BackgroundForm *BgForm=NULL;

extern ConfigObject *ConfigFile;
extern MainScreen *MainForm;

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
BackgroundForm::BackgroundForm(QWidget *parent) :
	QDialog(parent,Qt::WindowMinimizeButtonHint),
    ui(new Ui::BackgroundForm)
{
	ui->setupUi(this);

	NPoints=0;
	Slope=0.;
	Offset=0.;
	AutoDefined=false;
	BackgroundOk=false;

	Configure();
}

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
BackgroundForm::~BackgroundForm()
{
	delete ui;
}

/*=============================================================================*/
/*!
 */
/*=============================================================================*/
void BackgroundForm::Configure()
{
	QString Text;
	double dVal;
	int iVal;

	dVal=ConfigFile->Config_GetDouble("Background","LRFirst",0.0);
	Text=QString::number(dVal,'f',1)+QStringLiteral("%");
	ui->LRFirst->setText(Text);
	dVal=ConfigFile->Config_GetDouble("Background","LRLast",10.0);
	Text=QString::number(dVal,'f',1)+QStringLiteral("%");
	ui->LRLast->setText(Text);

	iVal=ConfigFile->Config_GetInt("Background","AverageStart",0);
	Text.setNum(iVal);
	ui->AverageStart->setText(Text);
	iVal=ConfigFile->Config_GetInt("Background","AverageNPoints",100);
	Text.setNum(iVal);
	ui->AverageNPoints->setText(Text);

	iVal=ConfigFile->Config_GetInt("Background","Type",1);
	if (iVal==0)
	{
		ui->AverageButton->setChecked(false);
		ui->LinearRegButton->setChecked(true);
	}
	else if (iVal==1)
	{
		ui->LinearRegButton->setChecked(false);
		ui->AverageButton->setChecked(true);
	}

}

/*=============================================================================*/
/*!
  Add a point to the background.

  \date 2001-11-22
 */
/*=============================================================================*/
void BackgroundForm::AddPoint(double x, double y)
{
	int i;

	if (ui->BackgroundMode->currentWidget()==ui->AutomaticSheet) return;
	if (NPoints>=MAX_BKGRPOINTS) return;
	AutoDefined=false;  //disable automatic background
	BackgroundOk=false;
	for (i=0 ; i<NPoints && x>XData[i] ; i++);
	if (i>=NPoints || fabs(y-YData[i])<1e20*fabs(x-XData[i]))
	{                                   //add the point only if the resoluting slope is not too big
		for (int j=NPoints ; j>i ; j--)         //in the other case, replace the existing point
		{
			XData[j]=XData[j-1];
			YData[j]=YData[j-1];
		}
		NPoints++;
	}
	XData[i]=x;
	YData[i]=y;

	if (NPoints>ui->BkgrPointsList->rowCount())
	{
		ui->BkgrPointsList->insertRow(i);
	}

	QString Text;
	Text.setNum(x,'g',3);
	QTableWidgetItem *XListItem=new QTableWidgetItem(Text);
	ui->BkgrPointsList->setItem(i,0,XListItem);

	Text.setNum(y,'g',3);
	QTableWidgetItem *YListItem=new QTableWidgetItem(Text);
	ui->BkgrPointsList->setItem(i,1,YListItem);
}

/*=============================================================================*/
/*!
  Remove a point to the background.

  \date 2001-11-22
 */
/*=============================================================================*/
void BackgroundForm::RemovePoint(double x, double y)
{
	int i,IMin;
	double dMin,d;

	if (ui->BackgroundMode->currentWidget()==ui->AutomaticSheet) return;
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
		ui->BkgrPointsList->removeRow(IMin);
	}
}

/*=============================================================================*/
/*!
  Prepare the background and make available the points to draw
  in XBkgr, YBkgr and NBgPoints.

  \return \c true if the data can be read from XBkgr, YBkgr, NBgPoints, or \c false
		  if no background is available.

  \date 2001-12-01
 */
/*=============================================================================*/
bool BackgroundForm::PrepareBkgr(double Time0, double XFreq,int NPts)
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
	if (ui->BackgroundMode->currentWidget()==ui->AutomaticSheet)
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

/*=============================================================================*/
/*!
  Get the slope of the background position x. x is updated to contains
  the next X position when the background slope will change.

  \return The slope of the background. Note that if there is no background the slope
          will be zero.
 */
/*=============================================================================*/
double BackgroundForm::GetNextSlope(double * x)
{
	int i;
	double LocSlope;

	//***** automatic background *****
	if (ui->BackgroundMode->currentWidget()==ui->AutomaticSheet)
	{
		*x=std::numeric_limits<double>::max();
		if (!AutoDefined) return(0.);
		return(Slope);
	}

	//***** manual background *****
	if (NPoints<2)
	{
		*x=std::numeric_limits<double>::max();
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
	*x=std::numeric_limits<double>::max();
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
void BackgroundForm::GetBackground(double XFreq,double *x,double *LocSlope,double *LocOffset)
{
	int i;

	//***** automatic background *****
	if (ui->BackgroundMode->currentWidget()==ui->AutomaticSheet)
	{
		*x=(XFreq>=0.) ? std::numeric_limits<double>::max() : -std::numeric_limits<double>::max();
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
		*x=(XFreq>=0.) ? std::numeric_limits<double>::max() : -std::numeric_limits<double>::max();
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
	*x=(XFreq>=0.) ? std::numeric_limits<double>::max() : -std::numeric_limits<double>::max();
	*LocOffset=YData[i-1]-XData[i-1]*(*LocSlope);
	return;
}

/*=============================================================================*/
/*!
  The user wants to calculate the automatic background.

  \author F Marchal
  \date 2001-12-02
 */
/*=============================================================================*/
void BackgroundForm::on_CalcAutoButton_clicked()
{
	AutoDefined=true;
	BackgroundOk=false;
	if (CalculateAutoBackground()) emit UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user wants to invalidate the automatic background.

  \author F Marchal
  \date 2001-12-02
 */
/*=============================================================================*/
void BackgroundForm::on_DeleteAutoButton_clicked()
{
	AutoDefined=false;
	BackgroundOk=false;
	emit UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user press enter in the control.

  \author F Marchal
  \date 2001-12-02
 */
/*=============================================================================*/
void BackgroundForm::on_LRFirst_editingFinished()
{
	double Percent;
	bool Ok=false;

	Percent=ui->LRFirst->text().toDouble(&Ok);
	if (!Ok) return;
	if (Percent<0.) Percent=0.;
	else if (Percent>100.) Percent=100.;

	QString Text=QString::number(Percent,'f',1)+QStringLiteral("%");
	ui->LRFirst->setText(Text);
	ui->LRFirst->selectAll();
	BackgroundOk=false;
}

/*=============================================================================*/
/*!
  The user press enter in the control.

  \author F Marchal
  \date 2001-12-06
 */
/*=============================================================================*/
void BackgroundForm::on_AverageStart_editingFinished()
{
	int Value;
	bool Ok=false;

	Value=ui->AverageStart->text().toDouble(&Ok);
	if (!Ok) return;
	if (Value<0) Value=0;

	QString Text;
	Text.setNum(Value);
	ui->AverageStart->setText(Text);
	ui->AverageStart->selectAll();
	BackgroundOk=false;
}

/*=============================================================================*/
/*!
  Calculate the background if the automatic mode is enabled.

  \return \c true if the main window should be redraw, or \c false if nothing has to be
          changed in the graphic.
 */
/*=============================================================================*/
bool BackgroundForm::CalculateAutoBackground(void)
{
	int i,NPts,i0,i1;
	double sx,sy,sxx,sxy;
	double a,b,den,*XdData,*YdData,nx;

	if (ui->BackgroundMode->currentWidget()==ui->ManualSheet) return(false);
	if (!AutoDefined) return(false);

	if (ui->LinearRegButton->isChecked())
	{
		if (!MainForm || !MainForm->XPlot || !MainForm->YPlot || MainForm->NPoints<2)
		{
			WriteMsg(__FILE__,__LINE__,tr("No data to calculate the linear regression"));
			return(false);
		}
		bool Ok=false;
		QString Value=ui->LRFirst->text();
		if (Value.endsWith('%')) Value.chop(1);
		double FirstPercent=Value.toDouble(&Ok);
		if (!Ok)
		{
			WriteMsg(__FILE__,__LINE__,tr("Invalid starting point"));
			ui->LRFirst->selectAll();
			ui->LRFirst->setFocus();
			return(false);
		}
		Value=ui->LRLast->text();
		if (Value.endsWith('%')) Value.chop(1);
		double LastPercent=Value.toDouble(&Ok);
		if (!Ok)
		{
			WriteMsg(__FILE__,__LINE__,tr("Invalid ending point"));
			ui->LRLast->selectAll();
			ui->LRLast->setFocus();
			return(false);
		}
		XdData=MainForm->XPlot;
		YdData=MainForm->YPlot;
		NPts=MainForm->NPoints;
		i0=(int)((NPts-1)*FirstPercent/100.);
		i1=(int)((NPts-1)*LastPercent/100.);
		if (i0<0 || i1>=NPts || i1-i0<2)
		{
			WriteMsg(__FILE__,__LINE__,tr("Invalid range for the linear regression"));
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
			WriteMsg(__FILE__,__LINE__,tr("Cannot calculate linear regression"));
			return(false);
		}
		Slope=a/den;
		Offset=b/den;
		AutoDefined=true;
		BackgroundOk=false;

		//***** save options to the configuration file *****
		ConfigFile->Config_WriteDouble("Background","LRFirst",FirstPercent);
		ConfigFile->Config_WriteDouble("Background","LRLast",LastPercent);
		ConfigFile->Config_WriteInt("Background","Type",0);
		return(true);
	}

	if (ui->AverageButton->isChecked())
	{
		if (!MainForm || !MainForm->XPlot || !MainForm->YPlot || MainForm->NPoints<2)
		{
			WriteMsg(__FILE__,__LINE__,tr("No data to calculate the average"));
			return(false);
		}
		XdData=MainForm->XPlot;
		YdData=MainForm->YPlot;
		NPts=MainForm->NPoints;
		bool Ok=false;
		double AvgStart=ui->AverageStart->text().toDouble(&Ok);
		if (!Ok)
		{
			WriteMsg(__FILE__,__LINE__,tr("Invalid starting point"));
			ui->AverageStart->selectAll();
			ui->AverageStart->setFocus();
			return(false);
		}
		double AvgNPoints=ui->AverageNPoints->text().toDouble(&Ok);
		if (!Ok)
		{
			WriteMsg(__FILE__,__LINE__,tr("Invalid number of point"));
			ui->AverageNPoints->selectAll();
			ui->AverageNPoints->setFocus();
			return(false);
		}

		i0=AvgStart;
		i1=i0+AvgNPoints;
		if (i0<0 || i1>=NPts || i1-i0<2)
		{
			WriteMsg(__FILE__,__LINE__,tr("Invalid range for the average"));
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
		ConfigFile->Config_WriteInt("Background","AverageStart",AvgStart);
		ConfigFile->Config_WriteInt("Background","AverageNPoints",AvgNPoints);
		ConfigFile->Config_WriteInt("Background","Type",1);
		return(true);
	}

	return(false);
}

/*=============================================================================*/
/*!
  Delete all points in the manual background.
 */
/*=============================================================================*/
void BackgroundForm::on_DeleteAllButton_clicked()
{
	NPoints=0;
	Slope=0.;
	Offset=0.;
	AutoDefined=false;
	BackgroundOk=false;
	ui->BkgrPointsList->clearContents();
	emit UpdateGraphics();
}

/*=============================================================================*/
/*!
  The user toggle between the automatic background and the manual background.
 */
/*=============================================================================*/
void BackgroundForm::on_BackgroundMode_currentChanged(int Index)
{
	Index=Index;//compiler pacifier
	if (ui->BackgroundMode->currentWidget()==ui->ManualSheet)
	{
		MainForm->SetBgMouseClick(true);
	}
	else
	{
		MainForm->SetBgMouseClick(false);
	}
}

/*=============================================================================*/
/*!
  The background window is hidden.

  \todo Enable the function with Qt
 */
/*=============================================================================*/
/*void BackgroundForm::FormHide()
{
	MainForm->SetBgMouseClick(false);
}*/

/*=============================================================================*/
/*!
  The background window is visible again.

  \todo Enable the function with Qt
 */
/*=============================================================================*/
/*void BackgroundForm::FormShow()
{
	if (BackgroundMode->ActivePage==ManualSheet)
	{
		MainForm->SetBgMouseClick(true);
	}
}*/

/*=============================================================================*/
/*!
  The linear regression button is checked or unchecked.

  \param Checked The state of the button.
 */
/*=============================================================================*/
void BackgroundForm::on_LinearRegButton_toggled(bool Checked)
{
	bool Signal;

	(void)Checked; //compiler pacifier

	Signal=ui->AverageButton->blockSignals(true);
	ui->AverageButton->setChecked(false);
	ui->AverageButton->blockSignals(Signal);

	Signal=ui->LinearRegButton->blockSignals(true);
	ui->LinearRegButton->setChecked(true);
	ui->LinearRegButton->blockSignals(Signal);
}

/*=============================================================================*/
/*!
  The average button is checked or unchecked.

  \param Checked The state of the button.
 */
/*=============================================================================*/
void BackgroundForm::on_AverageButton_toggled(bool Checked)
{
	bool Signal;

	(void)Checked;//compiler pacifier

	Signal=ui->LinearRegButton->blockSignals(true);
	ui->LinearRegButton->setChecked(false);
	ui->LinearRegButton->blockSignals(Signal);

	Signal=ui->AverageButton->blockSignals(true);
	ui->AverageButton->setChecked(true);
	ui->AverageButton->blockSignals(Signal);
}
