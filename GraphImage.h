//---------------------------------------------------------------------------
#ifndef GraphImageH
#define GraphImageH
//---------------------------------------------------------------------------

#define MAX_CURVES 3     //maximum number of curves that can be drawn in the graphic
#define GRAD 5           //length of a tick mark
#define MAXXNAME 15      //maximum number of characters in the name of the X axis
#define MAXYNAME 15      //maximum number of characters in the name of the X axis

class ImageGraph
{
private:
    void *ParentPtr;     //pointer to the parent class
    TImage *MainImage;   //handle of the parent image
    TCanvas *PCanvas;    //Canvas of the parent image
    int ImageHeight;     //heigth of the parent control
    int ImageWidth;      //width of the parent control
    int GTop,GBottom,GRight,GLeft;  //position of the curves area
    int FontHeight,FontWidth;   //size of the font to draw the text
    enum TColor CTextBg;   //color of the text background
    enum TColor CFrame;    //color of the frame
    enum TColor CData[MAX_CURVES];     //color of the data curves
    enum TColor CDataBg;   //color of the data curve background
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

    void ImageGraph::SetXTicks(double min,double max);
    void ImageGraph::SetYTicks(double min,double max);
    int join(double x0, double y0, double x1, double y1);
    void __fastcall wts(double x, double y, int * ix, int* iy);
    void __fastcall MouseDown(TObject *Sender,TMouseButton Button,TShiftState Shift,int X,int Y);
    void __fastcall MouseMove(TObject *Sender,TShiftState Shift, int X, int Y);
    void __fastcall MouseUp(TObject *Sender,TMouseButton Button,TShiftState Shift,int X,int Y);
    void ScaleDisplay();

public:
    char XName[MAXXNAME];      //label of the X axis
    char YName[MAXYNAME];      //label of the Y axis

    void (*LeftMouseClick)(void *Parent,double x,double y,Classes::TShiftState Shift);
    void (*RightMouseClick)(void *Parent,double x,double y,Classes::TShiftState Shift);
    void (*MouseMoveCallback)(void *Parent,bool InGraph,double x,double y);

    ImageGraph(void *ParentClass,TImage * Parent);
    void Redraw(void);
    void SetZoom(double XMin, double XMax, double YMin, double YMax);
    void Unzoom(void);
    void SetGraphic(int Graphic,double *XPoints, double *YPoints,int NPts);
    void DeleteAllCurves(void);
    void DeleteCurve(int Curve);
    bool GetMousePos(int X, int Y,double *XPos,double *YPos);
    void ResizeGraph(void);
};
#endif
