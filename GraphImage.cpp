//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <values.h>
#include "GraphImage.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

#define MIN_ZOOM_AREA 5  //minimum size of the zoom area in pixels
#define MIN_ZOOM_FACTOR 1E-10  //zoom area must be bigger than MIN_ZOOM_FACTOR*(Full axis range of unzoomed data)

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: ImageGraph
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Create a new graphic using the specified image.
//
// Usage: ImageGraph::ImageGraph(void *ParentClass,TImage * Parent)
//
//    -@- void *ParentClass: Pointer to the parent class that can be passed
//              to the functions invoked when the user click on the graphic.
//    -@- TImage *Parent: Image control where we must draw the graphic.
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 6/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
ImageGraph::ImageGraph(void *ParentClass,TImage * Parent)
{
  TEXTMETRIC tm;
  int i;

  //***** init variables *****
  for (i=0 ; i<MAX_CURVES ; i++)
   {
   NPoints[i]=0;
   XData[i]=NULL;
   YData[i]=NULL;
   }
  LeftMouseClick=NULL;
  RightMouseClick=NULL;
  MouseMoveCallback=NULL;
  Zooming=false;
  ZoomEnabled=false;

  //***** get informations about the parent image *****
  ParentPtr=ParentClass;
  MainImage=Parent;
  PCanvas=MainImage->Canvas;
  ImageHeight=MainImage->Height;
  ImageWidth=MainImage->Width;
  MainImage->OnMouseDown=MouseDown;
  MainImage->OnMouseMove=MouseMove;
  MainImage->OnMouseUp=MouseUp;

  //***** get default colors *****
  CTextBg=(enum TColor)0x808080;
  CFrame=(enum TColor)0x0000FF;
  CData[0]=(enum TColor)0x0000FF;
  CData[1]=(enum TColor)0x00FF00;
  CData[2]=(enum TColor)0xFF0000;
  CDataBg=(enum TColor)0x000000;

  //***** get font size *****
  GetTextMetrics(MainImage->Canvas->Handle,&tm);
  FontHeight = tm.tmHeight;
  FontWidth = tm.tmMaxCharWidth;

  //***** calculate the size of the image *****
  GTop=0;
  GBottom=ImageHeight-2*FontHeight;
  GLeft=5*FontWidth+FontHeight;
  GRight=ImageWidth;

  //***** set default view region *****
  SetZoom(0.,10.,0.,10.);
  DispXMin=0.;
  DispXMax=0.;
  DispYMin=0.;
  DispYMax=0.;
  strcpy(XName,"X axis");
  strcpy(YName,"Y axis");

  Redraw();
}

/*=============================================================================*/
/*!
  Redraw both axis.

  \date 2001-11-11
 */
/*=============================================================================*/
void ImageGraph::Redraw(void)
{
  double *x,*y,x0,x1,y0,y1;
  int i,j,NPts;
  TPoint MultiPoint[100];
  double xx0, yy0, y00, xx1, yy1, y11, h,ox0,oy0;
  int ix0, iy0, ix1, iy1;

  ImageHeight=MainImage->Height;
  ImageWidth=MainImage->Width;
  GTop=0;
  GBottom=ImageHeight-2*FontHeight;
  GLeft=5*FontWidth+FontHeight;
  GRight=ImageWidth;

  //***** clear axis *****
  PCanvas->Pen->Style=psSolid;
  PCanvas->Pen->Color=CTextBg;
  PCanvas->Brush->Style=bsSolid;
  PCanvas->Brush->Color=CTextBg;
  PCanvas->FillRect(TRect(0, GTop, GLeft, GBottom));
  PCanvas->FillRect(TRect(0, GBottom, GRight, ImageHeight));
  PCanvas->Pen->Style=psSolid;
  PCanvas->Pen->Color=CFrame;
  PCanvas->FrameRect(TRect(GLeft,GTop,GRight,GBottom));
  PCanvas->Brush->Color=CDataBg;
  PCanvas->FillRect(TRect(GLeft+1, GTop+1, GRight-1, GBottom-1));

  //OX = GrY+1;
  //OY = GrX-1;
  //VMaxX = MaxX-OX-1;
  //VMaxY = OY-1-Limite;

  //***** dessiner axes *****
  SetXTicks(PlotXMin,PlotXMax);
  SetYTicks(PlotYMin,PlotYMax);

  //***** draw curve *****
  for (j=0 ; j<MAX_CURVES ; j++)
   {
   if (!XData[j] || !YData[j]) continue;
   x=XData[j];
   y=YData[j];
   PCanvas->Pen->Color=CData[j];
   ox0=MAXDOUBLE;
   oy0=MAXDOUBLE;
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
    //join(x[i-1],y[i-1],x[i],y[i]);

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
     if (NPts>0) PCanvas->Polyline(MultiPoint,NPts-1);
     NPts=0;
     wts(xx0, yy0, &ix0, &iy0);
     if (ix0>GRight) ix0=GRight;
     else if (ix0<GLeft) ix0=GLeft;
     if (iy0>GBottom) iy0=GBottom;
     else if (iy0<GTop) iy0=GTop;
     MultiPoint[NPts++]=TPoint(ix0,iy0);
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
     if (NPts>=sizeof(MultiPoint)/sizeof(TPoint))
      {
      PCanvas->Polyline(MultiPoint,NPts-1);
      NPts=0;
      }
     MultiPoint[NPts++]=TPoint(ix1,iy1);
     }
    }
   if (NPts>0) PCanvas->Polyline(MultiPoint,NPts-1);
   }

}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SetXTicks
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Set the Ticks on the X axis.
//
//
//
// Usage: void ImageGraph::SetXTicks(double ea,double eb)
//
//
// Returns:
//
// Remarks:
//
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 11/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void ImageGraph::SetXTicks(double min,double max)
{
  double ex, fen, del, i,c,start,y10;
  double y, xoffset, yoffset,ix;
  char val[20],Deci=0;
  TSize Size;

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
  PCanvas->Pen->Color=CFrame;
  PCanvas->Brush->Color=CTextBg;
  for (i = start ; i <= max ; i += del)
   {
   ix = xoffset + (i - min) * ex;
   PCanvas->MoveTo((int)ix,(int)yoffset-1);
   PCanvas->LineTo((int)ix,(int)yoffset-GRAD);
   PCanvas->MoveTo((int)ix,(int)GTop+1);
   PCanvas->LineTo((int)ix,(int)GTop+GRAD);
   if (Deci)
    sprintf(val,"%.2f",i);
   else
    sprintf(val,"%.0f",i);
   Size=PCanvas->TextExtent(val);
   if (ix+Size.cx/2<ImageWidth) PCanvas->TextOut((int)ix-Size.cx/2,(int)yoffset+1,val);
   }

  Size=PCanvas->TextExtent(XName);
  PCanvas->TextOut(GRight-Size.cx,ImageHeight-Size.cy,XName);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SetYTicks
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Set the Ticks on the Y axis.
//
//
//
// Usage: void ImageGraph::SetYTicks(double ea,double eb)
//
//
// Returns:
//
// Remarks:
//
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 11/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void ImageGraph::SetYTicks(double min,double max)
{
  double ex, fen, del, i, y10,c,start;
  double y, xoffset, yoffset, iy,Bottom=0.;
  char val[25];
  TSize Size,Size1;

  //if (min<0.) min=0.;
  //if (max<0.) max=10.;

  //***** calculer echelle *****
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
  /*if ((max/y10)>1. && log10(fabs(floor(max/y10)))>5.)
   {
   Bottom=start;
   SetTextAlign(DC,TA_LEFT|TA_TOP);
   sprintf(val,"Bottom at:%g",Bottom);
   PrintText(DC,0,scr_b,val,900);
   }*/

  //***** afficher echelle *****
  PCanvas->Pen->Color=CFrame;
  PCanvas->Brush->Color=CTextBg;
  //SetTextAlign(DC,TA_RIGHT|TA_BASELINE);
  for (i = start ; i <= max ; i += del)
   {
   iy = yoffset-((i - min) * ex);
   PCanvas->MoveTo((int)xoffset+1,(int)iy);
   PCanvas->LineTo((int)xoffset+GRAD,(int)iy);
   PCanvas->MoveTo((int)GRight-1,(int)iy);
   PCanvas->LineTo((int)GRight-GRAD,(int)iy);
   sprintf(val, "%5ld",(long int)((i-Bottom) / y10));
   Size=PCanvas->TextExtent(val);
   if (iy>GTop+3+3*FontHeight/2) PCanvas->TextOut((int)xoffset-Size.cx-GRAD,(int)iy-Size.cy/2,val);
   }

  //***** afficher facteur d'échelle *****
  /*SetTextAlign(DC,TA_LEFT|TA_TOP);
  PrintText(DC,0,scr_t+3+FontHeight/2,"x10",0);*/
  sprintf(val,"%+02d",(int)y);
  Size=PCanvas->TextExtent("x10");
  Size1=PCanvas->TextExtent(val);
  PCanvas->TextOut(0,GTop+Size1.cy/2,"x10");
  PCanvas->TextOut(Size.cx,GTop,val);

  //if (h->DivEner)
  /*SetTextAlign(DC,TA_RIGHT|TA_TOP);
  PrintText(DC,0,scr_t+3*FontHeight,Spect->YName,900);
  SetTextAlign(DC,TA_LEFT|TA_TOP);
  if (!HasDC) ReleaseDC(MainHWind,DC);*/
  /*Size=PCanvas->TextExtent(YName);
  PCanvas->TextOut(0,GTop+Size1.cy/2,YName);*/
  return;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: join
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Draw a line between two points.
//
//
//
// Usage: int ImageGraph::join(double x0, double y0, double x1, double y1)
//
//
// Returns:
//
// Remarks:
//
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 14/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
int ImageGraph::join(double x0, double y0, double x1, double y1)
{
  double xx0, yy0, y00, xx1, yy1, y11, h;
  int ix0, iy0, ix1, iy1;

  if (x0<=PlotXMin || x1<=PlotXMin || x0>=PlotXMax || x1>=PlotXMax ||
      y0<=PlotYMin || y1<=PlotYMin || y0>=PlotYMax || y1>=PlotYMax)
   {
   if ((y0<=PlotYMin) && (y1<=PlotYMin)) return(-1);
   if ((x0>=PlotXMax) && (x1>=PlotXMax)) return(-2);
   if ((y0>=PlotYMax) && (y1>=PlotYMax)) return(-3);
   if ((x0<=PlotXMin) && (x1<=PlotXMin)) return(-4);
   if (x0 != x1)
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
    if (y00 != y11)
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

  wts(xx0, yy0, &ix0, &iy0);
  if (ix0>GRight) ix0=GRight;
  else if (ix0<GLeft) ix0=GLeft;
  if (iy0>GBottom) iy0=GBottom;
  else if (iy0<GTop) iy0=GTop;
  wts(xx1, yy1, &ix1, &iy1);
  if (ix1>GRight) ix1=GRight;
  else if (ix1<GLeft) ix1=GLeft;
  if (iy1>GBottom) iy1=GBottom;
  else if (iy1<GTop) iy1=GTop;
  if (ix0!=ix1 || iy0!=iy1)
   {
   PCanvas->MoveTo(ix0,iy0);
   PCanvas->LineTo(ix1,iy1);
   }
  return(1);
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: wts
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Convert a point coordinate in a screen position.
//
// Usage: void __fastcall ImageGraph::wts(double x, double y, int * ix, int* iy)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 21/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall ImageGraph::wts(double x, double y, int * ix, int* iy)
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
void ImageGraph::ScaleDisplay()
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
void ImageGraph::SetZoom(double XMin, double XMax, double YMin, double YMax)
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

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: SetGraphic
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Set a new curve to the graphic but do not redraw.
//
// Usage: void ImageGraph::SetGraphic(int Graphic,double *XPoints, double *YPoints,int NPoints)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 21/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void ImageGraph::SetGraphic(int Graphic,double *XPoints, double *YPoints,int NPts)
{
  //***** init variables *****
  if (Graphic<0 || Graphic>=MAX_CURVES) return;
  NPoints[Graphic]=NPts;
  XData[Graphic]=XPoints;
  YData[Graphic]=YPoints;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: Unzoom
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Unzoom the graphic.
//
// Usage: void ImageGraph::Unzoom(void)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 21/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void ImageGraph::Unzoom(void)
{
  int i,j;
  bool FirstVal;
  double XMin,XMax,YMin,YMax,Swap,*XPtr,*YPtr;

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

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: DeleteAllCurves
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Delete all curves on the graphic.
//
// Usage: void ImageGraph::DeleteAllCurves(void)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 21/11/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void ImageGraph::DeleteAllCurves(void)
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
  strcpy(XName,"X axis");
  strcpy(YName,"Y axis");
  Redraw();
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: MouseDown
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Function called each time a mouse button is pressed.
//
// Usage: void __fastcall ImageGraph::MouseDown(TObject *Sender, TMouseButton Button,
//                        TShiftState Shift, int X, int Y)
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
void __fastcall ImageGraph::MouseDown(TObject *Sender, TMouseButton Button,
                        TShiftState Shift, int X, int Y)
{
  double x,y;

  if (X<GLeft || X>GRight || Y<GTop || Y>GBottom) return;
  x=((double)X-bx)/ax;
  y=((double)Y-by)/ay;
  if (Button==mbLeft && LeftMouseClick) LeftMouseClick(ParentPtr,x,y,Shift);
  if (Button==mbRight && RightMouseClick) RightMouseClick(ParentPtr,x,y,Shift);
  if (!LeftMouseClick && Button==mbLeft && DispXMin!=DispXMax && DispYMin!=DispYMax)
   {
   XZoom=X;
   YZoom=Y;
   XZoom2=XZoom;
   YZoom2=YZoom;
   Zooming=true;
   }
  if (!RightMouseClick && Button==mbRight)
   {
   ZoomEnabled=false;
   Unzoom();
   Redraw();
   }
}

/*=============================================================================*/
/*!
  Function called each time a mouse is moved on the graph.

  \date 2001-12-31
 */
/*=============================================================================*/
void __fastcall ImageGraph::MouseMove(TObject *Sender,TShiftState Shift, int X, int Y)
{
  int x0,y0,x1,y1;

  if (MouseMoveCallback)
   {
   bool InGraph=!(X<GLeft || X>GRight || Y<GTop || Y>GBottom);
   double XPos=((double)X-bx)/ax;
   double YPos=((double)Y-by)/ay;
   MouseMoveCallback(ParentPtr,InGraph,XPos,YPos);
   }
  if (Zooming)
   {
   if (abs(X-XZoom)<MIN_ZOOM_AREA) return;
   if (abs(Y-YZoom)<MIN_ZOOM_AREA) return;
   if (X<GLeft) X=GLeft;
   if (X>GRight) X=GRight;
   if (Y<GTop) Y=GTop;
   if (Y>GBottom) Y=GBottom;
   if (abs(XZoom-XZoom2)>=MIN_ZOOM_AREA && abs(YZoom-YZoom2)>=MIN_ZOOM_AREA)
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
    PCanvas->DrawFocusRect(TRect(x0,y0,x1,y1));
    }
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
   PCanvas->DrawFocusRect(TRect(x0,y0,x1,y1));
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: MouseUp
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Function called each time a mouse button is released.
//
// Usage: void __fastcall ImageGraph::MouseUp(TObject *Sender, TMouseButton Button,
//                        TShiftState Shift, int X, int Y)
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
void __fastcall ImageGraph::MouseUp(TObject *Sender, TMouseButton Button,
                        TShiftState Shift, int X, int Y)
{
  int x0,y0,x1,y1;
  double zx0,zy0,zx1,zy1;
  bool MousePointed;

  if (Zooming)
   {
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
   PCanvas->DrawFocusRect(TRect(x0,y0,x1,y1));
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
    Redraw();
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

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: DeleteCurve
//
// Type: Function
//
// Applies To: ImageGraph
//
// Description: Delete one curve of the graphic.
//
// Usage: void ImageGraph::DeleteCurve(int Curve)
//
// Returns:
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
void ImageGraph::DeleteCurve(int Curve)
{
  if (Curve<0 || Curve>=MAX_CURVES) return;
  NPoints[Curve]=0;
  XData[Curve]=NULL;
  YData[Curve]=NULL;
  Unzoom();
  Redraw();
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
bool ImageGraph::GetMousePos(int X, int Y,double *XPos,double *YPos)
{
  if (X<GLeft || X>GRight || Y<GTop || Y>GBottom) return(false);
  if (XPos) *XPos=((double)X-bx)/ax;
  if (YPos) *YPos=((double)Y-by)/ay;
  return(true);
}

/*=============================================================================*/
/*!
  Resize the graphics.

  \date 2005-01-17
 */
/*=============================================================================*/
void ImageGraph::ResizeGraph(void)
{
  PCanvas=MainImage->Canvas;
  ImageHeight=MainImage->Height;
  ImageWidth=MainImage->Width;

  //***** calculate the size of the image *****
  GTop=0;
  GBottom=ImageHeight-2*FontHeight;
  GLeft=5*FontWidth+FontHeight;
  GRight=ImageWidth;
  ScaleDisplay();
  //Unzoom();
  Redraw();
}

