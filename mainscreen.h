#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include <QMainWindow>
#include <qfileinfo.h>
#include "GraphImage.h"

//! Color of the main curve
#define MAIN_COLOR ((enum TColor)0xFFFFC0)
//! Color of the smoothed curve
#define SMOOTH_COLOR ((enum TColor)0x00C000)

namespace Ui {
    class MainScreen;
}

class MainScreen : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainScreen(QWidget *parent = 0);
    ~MainScreen();

private:
    Ui::MainScreen *ui;

private slots:
	void on_ExitMenu_triggered();
	void on_LoadMenu_triggered();

private:
	QFileInfo DefaultFileName;
	double *XData;
	double *YData;
	double YGain;
	double YOffset;
	double *Smooth;
	double GaussWidth;
	double XFreq;
	double Time0;
	double StoredXFreq;
	double StoredTime0;
	int GaussNeigh;
	double LastGWidth;
	int LastGNeigh;
	int SavGolPoly;
	int SavGolNeigh;
	double *Derive;
	int LastSGPoly;
	int LastSGNeigh;
	bool LastTrackSrc;
	bool HasBeenCut;
	bool ExitProgram;

	bool GetColumns(int NColumn,int &XColumn,int &YColumn);
	bool LoadCsvFile(char *Buffer,unsigned int FSize);
	bool LoadTirFile(char *Buffer,unsigned int FSize);
	void TrackPosition(bool XSource);
	bool AddDataPoint(char **ColumnsTxt,double **XData,double **YData,int &NPoints,
		 int &NAllocated,bool &InData,int XColumn,int YColumn,int Line);
	void WriteXFreq(double XFreq);
	void WriteXTime0(double XTime);
	void SetGraphSize(GraphImage *Control,int Width,int Height);
	void AddMemoLine(const QString &Text);

public:		// Déclarations de l'utilisateur
	int NPoints;
	double *XPlot,*YPlot;  //buffer to store the calculated curves
	double *YSmooth,*YDerv;

	void RecalculateGraphics(void);
	void UpdateGraphics(void);

	static void BkgrLeftClick(void *Parent,double x,double y,Qt::MouseButtons Shift);
	static void BkgrRightClick(void *Parent,double x,double y,Qt::MouseButtons Shift);
	static void TrackMouseMove(void *Parent,bool InGraph,double x,double y);
	bool WriteToClipboard(char * Text);
};

#endif // MAINSCREEN_H
