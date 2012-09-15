#include "GraphImage.h"
#include <math.h>
#include <limits.h>
#include <limits>
#include <values.h>

#define MIN_ZOOM_AREA 5  //minimum size of the zoom area in pixels
#define MIN_ZOOM_FACTOR 1E-10  //zoom area must be bigger than MIN_ZOOM_FACTOR*(Full axis range of unzoomed data)

/*=============================================================================*/
/*!
  Create a new graphic using the specified image.

  \param ParentClass Pointer to the parent class that can be passed
         to the functions invoked when the user click on the graphic.
  \param Parent Image control where we must draw the graphic.

  \date 2001-11-06
 */
/*=============================================================================*/
GraphImage::GraphImage(QWidget *Parent) :
	QWidget(Parent),
	Selection(NULL)
{
	//***** init variables *****
	for (int i=0 ; i<MAX_CURVES ; i++)
	{
		NPoints[i]=0;
		XData[i]=NULL;
		YData[i]=NULL;
	}
	Zooming=false;
	ZoomEnabled=false;

	//***** get informations about the parent image *****
	ImageHeight=0;
	ImageWidth=0;

	//***** get default colors *****
	CText=QColor(0x00,0x00,0x00);
	CTextBg=QColor(0xE0,0xE0,0xE0);
	CFrame=QColor(0xFF,0x00,0x00);
	CData[0]=QColor(0xFF,0x00,0x00);
	CData[1]=QColor(0x00,0xFF,0x00);
	CData[2]=QColor(0x00,0x00,0xFF);
	CDataBg=QColor(0xFF,0xFF,0xFF);

	//***** get font size *****
	const QFontMetrics &tm=fontMetrics();
	FontHeight = tm.height();
	FontWidth = tm.maxWidth();

	//***** set default view region *****
	SetZoom(0.,10.,0.,10.);
	PlotXMin=0.;
	PlotXMax=10.;
	PlotYMin=0.;
	PlotYMax=10.;
	DispXMin=0.;
	DispXMax=0.;
	DispYMin=0.;
	DispYMax=0.;
	XName=tr("X axis");
	YName=tr("Y axis");

	setMouseTracking(true);
}

/*=============================================================================*/
/*!
  Redraw the content of the graph.
 */
/*=============================================================================*/
void GraphImage::paintEvent(QPaintEvent *event)
{
	event=event;//compiler pacifier
	Redraw();
}

/*=============================================================================*/
/*!
  Redraw both axis.

  \date 2001-11-11
 */
/*=============================================================================*/
void GraphImage::Redraw()
{
	double *x,*y,x0,x1,y0,y1;
	int i,j,NPts;
	QPoint MultiPoint[100];
	double xx0, yy0, y00, xx1, yy1, y11, h,ox0,oy0;
	int ix0, iy0, ix1, iy1;

	QPainter PCanvas(this);

	if (ImageHeight!=height() || ImageWidth!=width())
	{
		ImageHeight=height();
		ImageWidth=width();
		GTop=0;
		GBottom=ImageHeight-2*FontHeight;
		GRight=ImageWidth;
		ComputeGLeft(PCanvas,PlotYMin,PlotYMax);
		ScaleDisplay();
	}

	// draw graph area
	QPen Pen;
	Pen.setStyle(Qt::SolidLine);
	Pen.setColor(CTextBg);
	PCanvas.setPen(Pen);
	PCanvas.fillRect(0, GTop, GLeft, GBottom-GTop,CTextBg);
	PCanvas.fillRect(0, GBottom, GRight, ImageHeight-GBottom,CTextBg);
	Pen.setColor(CFrame);
	PCanvas.setPen(Pen);
	//PCanvas.drawRect(GLeft,GTop,GRight-GLeft,GBottom-GTop);
	PCanvas.fillRect(GLeft, GTop, GRight-GLeft, GBottom-GTop,CDataBg);

	// plot axis
	SetXTicks(PCanvas,PlotXMin,PlotXMax);
	SetYTicks(PCanvas,PlotYMin,PlotYMax);

	// draw curves
	for (j=0 ; j<MAX_CURVES ; j++)
	{
		if (!XData[j] || !YData[j]) continue;
		x=XData[j];
		y=YData[j];
		PCanvas.setPen(CData[j]);
		ox0=std::numeric_limits<double>::max();
		oy0=std::numeric_limits<double>::max();
		ix1=0;
		iy1=0;
		NPts=0;
		for (i=1 ; i<NPoints[j] ; i++)
		{
			x0=x[i-1];
			x1=x[i];
			if ((x0<PlotXMin && x1<PlotXMin) || (x0>PlotXMax && x1>PlotXMax)) continue;
			y0=y[i-1];
			y1=y[i];
			if ((y0<PlotYMin && y1<PlotYMin) || (y0>PlotYMax && y1>PlotYMax)) continue;

			if (x0<PlotXMin || x1<PlotXMin || x0>PlotXMax || x1>PlotXMax ||
					y0<PlotYMin || y1<PlotYMin || y0>PlotYMax || y1>PlotYMax)
			{
				if (x0!=x1)
				{
					if (x0>PlotXMax) xx0=PlotXMax;
					else if (x0<PlotXMin) xx0=PlotXMin;
					else xx0=x0;
					if (x1>PlotXMax) xx1=PlotXMax;
					else if (x1<PlotXMin) xx1=PlotXMin;
					else xx1=x1;
					h = (y1 - y0)/(x1 - x0);
					y00 = y0 + h*(xx0 - x0);
					y11 = y1 + h*(xx1 - x1);
					if (y00!=y11)
					{
						if (y00>PlotYMax) yy0=PlotYMax;
						else if (y00<PlotYMin) yy0=PlotYMin;
						else yy0=y00;
						if (y11>PlotYMax) yy1=PlotYMax;
						else if (y11<PlotYMin) yy1=PlotYMin;
						else yy1=y11;
						xx0 = x0 + (yy0 - y0)/h;
						xx1 = x1 + (yy1 - y1)/h;
					}
					else
					{
						yy0 = y00;
						yy1 = y11;
					}
				}
				else
				{
					xx0 = xx1 = x0;
					if (y0>PlotYMax) yy0=PlotYMax;
					else if (y0<PlotYMin) yy0=PlotYMin;
					else yy0=y0;
					if (y1>PlotYMax) yy1=PlotYMax;
					else if (y1<PlotYMin) yy1=PlotYMin;
					else yy1=y1;
				}
			}
			else
			{
				xx0=x0;
				yy0=y0;
				xx1=x1;
				yy1=y1;
			}

			if (xx0==ox0 && yy0==oy0)
			{
				ix0=ix1;
				iy0=iy1;
			}
			else
			{
				if (NPts>0) PCanvas.drawPolyline(MultiPoint,NPts);
				NPts=0;
				wts(xx0, yy0, &ix0, &iy0);
				if (ix0>GRight) ix0=GRight;
				else if (ix0<GLeft) ix0=GLeft;
				if (iy0>GBottom) iy0=GBottom;
				else if (iy0<GTop) iy0=GTop;
				MultiPoint[NPts++]=QPoint(ix0,iy0);
			}
			ox0=xx1;
			oy0=yy1;
			wts(xx1, yy1, &ix1, &iy1);
			if (ix1>GRight) ix1=GRight;
			else if (ix1<GLeft) ix1=GLeft;
			if (iy1>GBottom) iy1=GBottom;
			else if (iy1<GTop) iy1=GTop;
			if (ix0!=ix1 || iy0!=iy1)
			{
				if (NPts>=(int)(sizeof(MultiPoint)/sizeof(MultiPoint[0])))
				{
					PCanvas.drawPolyline(MultiPoint,NPts);
					NPts=0;
				}
				MultiPoint[NPts++]=QPoint(ix1,iy1);
			}
		}
		if (NPts>0) PCanvas.drawPolyline(MultiPoint,NPts);
	}

}

/*=============================================================================*/
/*!
  Set the Ticks on the X axis.

  \date 2001-11-11
 */
/*=============================================================================*/
void GraphImage::SetXTicks(QPainter &PCanvas,double min,double max)
{
	double ex, fen, del=0., i,c,start,y10;
	double y, xoffset, yoffset,ix;
	char Deci=0;
	QString val;
	QRect Size;

	fen=max-min;
	if (fen==0.)
	{
		max=10.;
		min=0.;
		fen=10.;
	}
	if (fen<0.)
	{
		c=min;
		max=min;
		min=c;
		xoffset = (double)GRight;
		//ex = (double)(scr_l-scr_r)/fen;
	}
	else
	{
		xoffset = (double)GLeft;
		//ex = (double)(scr_r-scr_l)/fen;
	}

	ex = (double)(GRight-GLeft)/fen;
	yoffset=(double)GBottom;
	c=log10(fabs(fen)/3.);
	y=floor(c);
	y10=pow(10.,y);
	if (c-y<2.) del=5.*y10;
	if (c-y<.8) del=4.*y10;
	if (c-y<.6) del=2.*y10;
	if (c-y<.3) del=1.*y10;
	start=ceil(min/y10)*y10;
	if (del<1.) Deci=1;
	PCanvas.setBrush(CTextBg);
	for (i = start ; i <= max ; i += del)
	{
		ix = xoffset + (i - min) * ex;
		PCanvas.setPen(CFrame);
		PCanvas.drawLine((int)ix,(int)yoffset+1,(int)ix,(int)yoffset+GRAD);
		//PCanvas.drawLine((int)ix,(int)GTop+1,(int)ix,(int)GTop+GRAD);
		if (Deci)
			val.sprintf("%.2f",i);
		else
			val.sprintf("%.0f",i);
		Size=PCanvas.boundingRect(QRect(ix,yoffset+GRAD,0,0),Qt::AlignHCenter | Qt::AlignTop,val);
		if (ix+Size.width()/2<ImageWidth)
		{
			PCanvas.setPen(CText);
			PCanvas.drawText(Size,Qt::AlignHCenter | Qt::AlignTop,val);
		}
	}

	PCanvas.setPen(CText);
	Size=PCanvas.boundingRect(QRect(GRight,ImageHeight,0,0),Qt::AlignRight | Qt::AlignBottom,XName);
	PCanvas.drawText(Size,Qt::AlignRight | Qt::AlignBottom,XName);
}

/*=============================================================================*/
/*!
  Set the Ticks on the Y axis.

  \date 2001-11-11
 */
/*=============================================================================*/
void GraphImage::ComputeGLeft(QPainter &PCanvas,double min,double max)
{
	double ex, fen, del=0., i, y10,c,start;
	double y, yoffset, iy,Bottom=0.;
	QString val;
	QRect Size,Size1;
	int wmax=0;

	fen = max - min;
	if (fen==0.)
	{
		min=0.;
		max=10.;
		fen=10.;
	}

	if (fen<0.)
	{
		i=max;
		max=min;
		min=i;
		yoffset = (double)GTop;
	}
	else
	{
		yoffset = (double)GBottom;
	}

	ex = (double)(GBottom-GTop)/fen;
	c=log10(fabs(fen)/3.);
	y=floor(c);
	y10=pow(10.,y);
	if (c-y<2.) del=5.*y10;
	if (c-y<.8) del=4.*y10;
	if (c-y<.6) del=2.*y10;
	if (c-y<.3) del=1.*y10;
	start=ceil(min/y10)*y10;
	//y--;
	y10=pow(10.,y);

	PCanvas.setBrush(CTextBg);
	for (i = start ; i <= max ; i += del)
	{
		iy = yoffset-((i - min) * ex);
		if (iy>GTop+3+3*FontHeight/2)
		{
			val.sprintf("%5ld",(long int)((i-Bottom) / y10));
			Size=PCanvas.boundingRect(QRect(0,iy,0,0),Qt::AlignRight | Qt::AlignVCenter,val);
			if (wmax<Size.width()) wmax=Size.width();
		}
	}

	val.sprintf("%+02d",(int)y);
	Size=PCanvas.boundingRect(QRect(0,GTop,0,0),Qt::AlignLeft | Qt::AlignTop,"x10");
	Size1=PCanvas.boundingRect(QRect(Size.width(),GTop,0,0),Qt::AlignLeft | Qt::AlignTop,val);
	GLeft=Size.width()+Size1.width()+wmax+GRAD;
}

/*=============================================================================*/
/*!
  Set the Ticks on the Y axis.

  \date 2001-11-11
 */
/*=============================================================================*/
void GraphImage::SetYTicks(QPainter &PCanvas,double min,double max)
{
	double ex, fen, del=0., i, y10,c,start;
	double y, xoffset, yoffset, iy,Bottom=0.;
	QString val;
	QRect Size,Size1;

	//if (min<0.) min=0.;
	//if (max<0.) max=10.;

	// compute scale
	fen = max - min;
	if (fen==0.)
	{
		min=0.;
		max=10.;
		fen=10.;
	}

	if (fen<0.)
	{
		i=max;
		max=min;
		min=i;
		//fen=-fen;
		yoffset = (double)GTop;
		//ex = (double)(scr_l-scr_r)/fen;
	}
	else
	{
		yoffset = (double)GBottom;
		//ex = (double)(scr_r-scr_l)/fen;
	}

	ex = (double)(GBottom-GTop)/fen;
	xoffset = (double)GLeft;
	//yoffset = (double)scr_b;
	c=log10(fabs(fen)/3.);
	y=floor(c);
	y10=pow(10.,y);
	if (c-y<2.) del=5.*y10;
	if (c-y<.8) del=4.*y10;
	if (c-y<.6) del=2.*y10;
	if (c-y<.3) del=1.*y10;
	start=ceil(min/y10)*y10;
	//y--;
	y10=pow(10.,y);

	// display scale
	PCanvas.setBrush(CTextBg);
	for (i = start ; i <= max ; i += del)
	{
		iy = yoffset-((i - min) * ex);
		PCanvas.setPen(CFrame);
		PCanvas.drawLine((int)xoffset-1,(int)iy,(int)xoffset-GRAD,(int)iy);
		//PCanvas.drawLine((int)GRight-1,(int)iy,(int)GRight-GRAD,(int)iy);

		if (iy>GTop+3+3*FontHeight/2)
		{
			val.sprintf("%5ld",(long int)((i-Bottom) / y10));
			PCanvas.setPen(CText);
			Size=PCanvas.boundingRect(QRect(xoffset-GRAD,iy,0,0),Qt::AlignRight | Qt::AlignVCenter,val);
			PCanvas.drawText(Size,Qt::AlignRight | Qt::AlignVCenter,val);
		}
	}

	// display scaling factor
	val.sprintf("%+02d",(int)y);
	Size=PCanvas.boundingRect(QRect(0,GTop,0,0),Qt::AlignLeft | Qt::AlignTop,"x10");
	Size1=PCanvas.boundingRect(QRect(Size.width(),GTop,0,0),Qt::AlignLeft | Qt::AlignTop,val);
	Size.translate(0,Size1.height()/2);
	PCanvas.setPen(CText);
	PCanvas.drawText(Size,Qt::AlignLeft | Qt::AlignTop,"x10");
	PCanvas.drawText(Size1,Qt::AlignLeft | Qt::AlignTop,val);

	//if (h->DivEner)
	/*SetTextAlign(DC,TA_RIGHT|TA_TOP);
	PrintText(DC,0,scr_t+3*FontHeight,Spect->YName,900);
	SetTextAlign(DC,TA_LEFT|TA_TOP);
	if (!HasDC) ReleaseDC(MainHWind,DC);*/
	/*Size=PCanvas->TextExtent(YName);
	PCanvas->TextOut(0,GTop+Size1.cy/2,YName);*/
	return;
}

/*=============================================================================*/
/*!
  Convert a point coordinate in a screen position.

  \date 2001-11-21
 */
/*=============================================================================*/
void GraphImage::wts(double x, double y, int * ix, int* iy)
{
	double x1,y1;

	x1=x*ax+bx+0.5;
	if (x1<-INT_MAX) x1=-INT_MAX;         //reduce the values to avoid conversions error when x1 or y1 is far to big
	else if (x1>INT_MAX) x1=INT_MAX;
	y1=y*ay+by+0.5;
	if (y1<-INT_MAX) y1=-INT_MAX;
	else if (y1>INT_MAX) y1=INT_MAX;
	*ix=(int)x1;                                    //*ix and *iy should be initialized to avoid drawing errors
	*iy=(int)y1;
}

/*=============================================================================*/
/*!
  Compute the scale factors to display the axis.

  \date 2001-11-21
 */
/*=============================================================================*/
void GraphImage::ScaleDisplay()
{
	if (fabs(GLeft-GRight)>=1e20*(PlotXMax-PlotXMin))
	{
		PlotXMin-=1.;
		PlotXMax+=1.;
	}
	ax=(double)(GRight-GLeft)/(PlotXMax-PlotXMin);
	bx=(double)GLeft-PlotXMin*ax;

	if (fabs(GBottom-GTop)>=1e20*(PlotYMax-PlotYMin))
	{
		PlotYMin-=1.;
		PlotYMax+=1.;
	}
	ay=(double)(GTop-GBottom)/(PlotYMax-PlotYMin);
	by=(double)GBottom-PlotYMin*ay;
}

/*=============================================================================*/
/*!
  Set the zoom of the window.

  \date 2001-11-21
 */
/*=============================================================================*/
void GraphImage::SetZoom(double XMin, double XMax, double YMin, double YMax)
{
	if (XMin<=XMax)
	{
		PlotXMin=XMin;
		PlotXMax=XMax;
	}
	else
	{
		PlotXMin=XMax;
		PlotXMax=XMin;
	}

	if (YMin<=YMax)
	{
		PlotYMin=YMin;
		PlotYMax=YMax;
	}
	else
	{
		PlotYMin=YMax;
		PlotYMax=YMin;
	}

	ScaleDisplay();
}

/*=============================================================================*/
/*!
  Set a new curve to the graphic but do not redraw.

  \date 2001-11-21
 */
/*=============================================================================*/
void GraphImage::SetGraphic(int Graphic,double *XPoints, double *YPoints,int NPts)
{
	//***** init variables *****
	if (Graphic<0 || Graphic>=MAX_CURVES) return;
	NPoints[Graphic]=NPts;
	XData[Graphic]=XPoints;
	YData[Graphic]=YPoints;
}

/*=============================================================================*/
/*!
  Unzoom the graphic.

  \date 2001-11-21
 */
/*=============================================================================*/
void GraphImage::Unzoom()
{
	int i,j;
	bool FirstVal;
	double XMin=0.,XMax=0.,YMin=0.,YMax=0.,Swap,*XPtr,*YPtr;

	FirstVal=true;
	for (i=0 ; i<MAX_CURVES ; i++)
	{
		if (!XData[i] || !YData[i]) continue;
		XPtr=XData[i];
		YPtr=YData[i];
		if (FirstVal)
		{
			XMax=XMin=*XPtr;
			YMax=YMin=*YPtr;
			FirstVal=false;
		}
		for (j=NPoints[i]-1 ; j>=0 ; j--)
		{
			if (XMin>*XPtr) XMin=*XPtr;
			else if (XMax<*XPtr) XMax=*XPtr;
			if (YMin>*YPtr) YMin=*YPtr;
			else if (YMax<*YPtr) YMax=*YPtr;
			XPtr++;
			YPtr++;
		}
	}
	if (!FirstVal)
	{
		if (XMin>XMax)
		{
			Swap=XMin;
			XMin=XMax;
			XMax=Swap;
		}
		if (YMin>YMax)
		{
			Swap=YMin;
			YMin=YMax;
			YMax=Swap;
		}
		if (ZoomEnabled)
		{
			if (XMin<PlotXMin) XMin=PlotXMin;
			if (XMax>PlotXMax) XMax=PlotXMax;
			if (YMin<PlotYMin) YMin=PlotYMin;
			if (YMax>PlotYMax) YMax=PlotYMax;
		}
		SetZoom(XMin,XMax,YMin,YMax);
		Redraw();
		update();
		DispXMin=XMin;
		DispXMax=XMax;
		DispYMin=YMin;
		DispYMax=YMax;
	}
	else
	{
		ZoomEnabled=false;
		DispXMin=0.;
		DispXMax=0.;
		DispYMin=0.;
		DispYMax=0.;
	}
}

/*=============================================================================*/
/*!
  Delete all curves on the graphic.

  \date 2001-11-21
 */
/*=============================================================================*/
void GraphImage::DeleteAllCurves()
{
	int i;

	for (i=0 ; i<MAX_CURVES ; i++)
	{
		NPoints[i]=0;
		XData[i]=NULL;
		YData[i]=NULL;
	}
	SetZoom(0.,10.,0.,10.);
	DispXMin=0.;
	DispXMax=0.;
	DispYMin=0.;
	DispYMax=0.;
	XName=tr("X axis");
	YName=tr("Y axis");
	update();
}

/*=============================================================================*/
/*!
  Function called each time a mouse button is pressed.

  \date 2001-11-22
 */
/*=============================================================================*/
void GraphImage::mousePressEvent(QMouseEvent *event)
{
	int X=event->x();
	int Y=event->y();
	if (X<GLeft || X>GRight || Y<GTop || Y>GBottom) return;
	Qt::MouseButton Button=event->button();
	if (Button==Qt::LeftButton) emit LeftMouseClick(event);
	if (Button==Qt::RightButton) emit RightMouseClick(event);
	if (Button==Qt::LeftButton && DispXMin!=DispXMax && DispYMin!=DispYMax && receivers(SIGNAL(LeftMouseClick(QMouseEvent*)))==0)
	{
		XZoom=X;
		YZoom=Y;
		XZoom2=XZoom;
		YZoom2=YZoom;
		Zooming=true;
		if (!Selection) Selection=new QRubberBand(QRubberBand::Rectangle,this);
		Selection->setGeometry(QRect(XZoom,YZoom,0,0));
		Selection->show();
	}
	if (Button==Qt::RightButton && receivers(SIGNAL(RightMouseClick(QMouseEvent*)))==0)
	{
		ZoomEnabled=false;
		Unzoom();
		update();
	}
}

/*=============================================================================*/
/*!
  Function called each time a mouse is moved on the graph.

  \date 2001-12-31
 */
/*=============================================================================*/
void GraphImage::mouseMoveEvent(QMouseEvent *event)
{
	int x0,y0,x1,y1;

	int X=event->x();
	int Y=event->y();
	if (receivers(SIGNAL(MouseMove(bool,double,double))))
	{
		bool InGraph=!(X<GLeft || X>GRight || Y<GTop || Y>GBottom);
		double XPos=((double)X-bx)/ax;
		double YPos=((double)Y-by)/ay;
		emit MouseMove(InGraph,XPos,YPos);
	}
	if (Zooming)
	{
		if (abs(X-XZoom)<MIN_ZOOM_AREA) return;
		if (abs(Y-YZoom)<MIN_ZOOM_AREA) return;
		if (X<GLeft) X=GLeft;
		if (X>GRight) X=GRight;
		if (Y<GTop) Y=GTop;
		if (Y>GBottom) Y=GBottom;
		/*if (abs(XZoom-XZoom2)>=MIN_ZOOM_AREA && abs(YZoom-YZoom2)>=MIN_ZOOM_AREA)
		{
			if (XZoom<XZoom2)
			{
				x0=XZoom;
				x1=XZoom2;
			}
			else
			{
				x0=XZoom2;
				x1=XZoom;
			}
			if (YZoom<YZoom2)
			{
				y0=YZoom;
				y1=YZoom2;
			}
			else
			{
				y0=YZoom2;
				y1=YZoom;
			}
			PCanvas.DrawFocusRect(TRect(x0,y0,x1,y1));
		}*/
		XZoom2=X;
		YZoom2=Y;
		if (XZoom<XZoom2)
		{
			x0=XZoom;
			x1=XZoom2;
		}
		else
		{
			x0=XZoom2;
			x1=XZoom;
		}
		if (YZoom<YZoom2)
		{
			y0=YZoom;
			y1=YZoom2;
		}
		else
		{
			y0=YZoom2;
			y1=YZoom;
		}
		//PCanvas->DrawFocusRect(TRect(x0,y0,x1,y1));
		Selection->setGeometry(QRect(x0,y0,x1-x0,y1-y0).normalized());
	}
}

/*=============================================================================*/
/*!
  Function called each time a mouse button is released.

  \date 2001-12-31
 */
/*=============================================================================*/
void GraphImage::mouseReleaseEvent(QMouseEvent *event)
{
	int x0,y0,x1,y1;
	double zx0,zy0,zx1,zy1;
	bool MousePointed;

	int X=event->x();
	int Y=event->y();
	if (Zooming)
	{
		Selection->hide();
		if (X<GLeft) X=GLeft;
		if (X>GRight) X=GRight;
		if (Y<GTop) Y=GTop;
		if (Y>GBottom) Y=GBottom;
		if (abs(X-XZoom)>=MIN_ZOOM_AREA) XZoom2=X;
		if (abs(Y-YZoom)>=MIN_ZOOM_AREA) YZoom2=Y;
		if (XZoom<XZoom2)
		{
			x0=XZoom;
			x1=XZoom2;
		}
		else
		{
			x0=XZoom2;
			x1=XZoom;
		}
		if (YZoom<YZoom2)
		{
			y0=YZoom;
			y1=YZoom2;
		}
		else
		{
			y0=YZoom2;
			y1=YZoom;
		}
		if (X<GLeft || X>GRight || Y<GTop || Y>GBottom) return;
		Zooming=false;
		zx0=((double)x0-bx)/ax;
		zy0=((double)y0-by)/ay;
		zx1=((double)x1-bx)/ax;
		zy1=((double)y1-by)/ay;
		if (fabs(zx1-zx0)<MIN_ZOOM_FACTOR*fabs(DispXMax-DispXMin) ||
				fabs(zy1-zy0)<MIN_ZOOM_FACTOR*fabs(DispYMax-DispYMin))
		{
			MousePointed=true;
		}
		else
		{
			MousePointed=false;
			SetZoom(zx0,zx1,zy0,zy1);
			ZoomEnabled=true;
			update();
		}
	}
	else
	{
		MousePointed=true;
	}
	/*if (MousePointed)
   {
   if (Button==mbLeft && LeftMouseClick) LeftMouseClick(ParentPtr,x,y,Shift);
   if (Button==mbRight && RightMouseClick) RightMouseClick(ParentPtr,x,y,Shift);
   }*/
}

/*=============================================================================*/
/*!
  Allow the wheel to zoom in or out of the graph.

  \date 2012-08-04
 */
/*=============================================================================*/
void GraphImage::wheelEvent(QWheelEvent *event)
{
	int X=event->x();
	int Y=event->y();
	if (X<GLeft || X>GRight || Y<GTop || Y>GBottom)
	{
		event->ignore();
		return;
	}
	double Scale=event->delta()/8.; //wheel rotation in degrees (15=1 wheel step)
	if (Scale>0.)
		Scale=Scale/15.*2./3.;
	else
		Scale=-Scale/15.*3./2.;

	double ZoomWidth=(PlotXMax-PlotXMin)*Scale;
	double ZoomHeight=(PlotYMax-PlotYMin)*Scale;
	if (ZoomWidth>fabs(DispXMax-DispXMin)) ZoomWidth=fabs(DispXMax-DispXMax);
	if (ZoomHeight>fabs(DispYMax-DispYMin)) ZoomHeight=fabs(DispYMax-DispYMax);
	double zx=((double)X-bx)/ax;
	double zy=((double)Y-by)/ay;
	double zx0=(double)zx-ZoomWidth/2.;
	double zx1=(double)zx+ZoomWidth/2.;
	double zy0=(double)zy-ZoomHeight/2.;
	double zy1=(double)zy+ZoomHeight/2.;
	if (zx0<DispXMin)
	{
		zx0=DispXMin;
		zx1=zx0+ZoomWidth;
	}
	else if (zx1>DispXMax)
	{
		zx1=DispXMax;
		zx0=zx1-ZoomWidth;
	}
	if (zy0<DispYMin)
	{
		zy0=DispYMin;
		zy1=zy0+ZoomHeight;
	}
	else if (zy1>DispYMax)
	{
		zy1=DispYMax;
		zy0=zy1-ZoomHeight;
	}
	if (fabs(zx1-zx0)<MIN_ZOOM_FACTOR*fabs(DispXMax-DispXMin) ||
			fabs(zy1-zy0)<MIN_ZOOM_FACTOR*fabs(DispYMax-DispYMin))
	{
		event->ignore();
		return;
	}
	SetZoom(zx0,zx1,zy0,zy1);
	update();
}

/*=============================================================================*/
/*!
  Delete one curve of the graphic.

  \date 2001-12-01
 */
/*=============================================================================*/
void GraphImage::DeleteCurve(int Curve)
{
	if (Curve<0 || Curve>=MAX_CURVES) return;
	NPoints[Curve]=0;
	XData[Curve]=NULL;
	YData[Curve]=NULL;
	Unzoom();
	update();
}

/*=============================================================================*/
/*!
  Calculate the coordinates of the mouse cursor.

  \param X The X position of the mouse.
  \param Y The Y position of the mouse.
  \param XPos A pointer to store the X coordinate.
  \param YPos A pointer to store the Y coordinate.

  \retrun True if the mouse is inside the graphics or false if it is outside.

  \date 2004-02-03
 */
/*=============================================================================*/
bool GraphImage::GetMousePos(int X, int Y,double *XPos,double *YPos)
{
	if (X<GLeft || X>GRight || Y<GTop || Y>GBottom) return(false);
	if (XPos) *XPos=((double)X-bx)/ax;
	if (YPos) *YPos=((double)Y-by)/ay;
	return(true);
}
