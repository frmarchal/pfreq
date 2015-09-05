#ifndef GRAPHIMAGE_HEADER
#define GRAPHIMAGE_HEADER

#include <qwidget.h>
#include <qstring.h>
#include <qcolor.h>
#include <qpainter.h>
#include <QMouseEvent>
#include <qrubberband.h>

#define MAX_CURVES 3     //maximum number of curves that can be drawn in the graphic
#define GRAD 5           //length of a tick mark
#define MAXXNAME 15      //maximum number of characters in the name of the X axis
#define MAXYNAME 15      //maximum number of characters in the name of the X axis

class GraphImage : public QWidget
{
	Q_OBJECT

private:
    int ImageHeight;     //heigth of the parent control
    int ImageWidth;      //width of the parent control
    int GTop,GBottom,GRight,GLeft;  //position of the curves area
    int FontHeight,FontWidth;   //size of the font to draw the text
	QColor CText;   //color of the text
	QColor CTextBg;   //color of the text background
	QColor CFrame;    //color of the frame
	QColor CData[MAX_CURVES];     //color of the data curves
	QColor CDataBg;   //color of the data curve background
    double PlotXMin,PlotXMax,PlotYMin,PlotYMax;   //visible area on the graphic
    double DispXMin,DispXMax,DispYMin,DispYMax;   //maximum area necessary to view the whole graphics
    int NPoints[MAX_CURVES];         //number of points in the curves
    double *XData[MAX_CURVES];       //data of the X axis
    double *YData[MAX_CURVES];       //data of the Y axis
    double ax,bx;              //gain and offset of X axis
    double ay,by;              //gain and offset of Y axis
    bool Zooming;              //set when the zoom area is being drawn
    int XZoom,YZoom;           //first point of the zoom area
    int XZoom2,YZoom2;         //second point of the zoom area
    bool ZoomEnabled;          //Set when the graphic is zoomed
	//! The area selected with the mouse,
	QRubberBand *Selection;

	void ComputeGLeft(QPainter &PCanvas,double min,double max);
	void SetXTicks(QPainter &PCanvas,double min,double max);
	void SetYTicks(QPainter &PCanvas,double min,double max);
	void wts(double x, double y, int * ix, int* iy);
	void ScaleDisplay();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);
	void wheelEvent(QWheelEvent *event);

signals:
	void LeftMouseClick(QMouseEvent *event);
	void RightMouseClick(QMouseEvent *event);
	void MouseMove(bool InGraph,double x,double y);

public:
	//! Label of the X axis.
	QString XName;
	//! Label of the Y axis.
	QString YName;

	GraphImage(QWidget *Parent=0);
	void Redraw();
    void SetZoom(double XMin, double XMax, double YMin, double YMax);
	void Unzoom();
    void SetGraphic(int Graphic,double *XPoints, double *YPoints,int NPts);
	void DeleteAllCurves();
    void DeleteCurve(int Curve);
    bool GetMousePos(int X, int Y,double *XPos,double *YPos);
	QColor GetBackgroundColor();
	void SetBackgroundColor(const QColor &Color);
	QColor GetCurveColor(int Curve);
	void SetCurveColor(int Curve,const QColor &Color);
};

#endif //GRAPHIMAGE_HEADER
