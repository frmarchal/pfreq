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

protected:
	void closeEvent(QCloseEvent * event);

private slots:
	void on_ExitMenu_triggered();
	void on_LoadMenu_triggered();
	void on_SaveMenu_aboutToShow();
	void on_SaveDerivativeMenu_triggered();
	void on_SaveSmoothMenu_triggered();
	void on_SaveDataMenu_triggered();
	void on_BackgroundMenu_triggered();
	void on_SelectOutputFile_triggered();
	void on_SelectColors_triggered();
	void on_Settings_triggered();
	void on_CopyData_triggered();
	void on_CopyBkgrMenu_triggered();
	void on_CopySmoothMenu_triggered();
	void on_CopyDeriveMenu_triggered();
	void on_CopyMenu_aboutToShow();
	void on_TrackData_clicked();
	void on_TrackSmooth_clicked();
	void on_TrackDerv_clicked();
	void on_TrackMouse_clicked();
	void on_RawSmoothButton_clicked();
	void on_SavGolButton_clicked();
	void on_CutMenu_triggered();
	void on_AboutMenu_triggered();

	void on_XFrequency_editingFinished();
	void on_XTime0_editingFinished();
	void on_GaussWidthCtrl_editingFinished();
	void on_GaussNeighCtrl_editingFinished();
	void on_SavGolSPolyCtrl_editingFinished();
	void on_SavGolSNeighCtrl_editingFinished();
	void on_SmoothTab_currentChanged(int Index);
	void on_YGainCtrl_editingFinished();
	void on_YOffsetCtrl_editingFinished();
	void on_SavGolPolyCtrl_editingFinished();
	void on_SavGolNeighCtrl_editingFinished();
	void on_XTracker_editingFinished();
	void on_YTracker_editingFinished();

	void BkgrLeftClick(QMouseEvent *event);
	void BkgrRightClick(QMouseEvent *event);

public slots:
	void UpdateGraphics();
	void TrackMouseMove(bool InGraph,double x,double y);

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
	//! The degree of the polynomial for the Savitsky-Golay smoohting.
	int SavGolSmoothPoly;
	//! The number of neighbour for the Savitsky-Golay smoothing.
	int SavGolSmoothNeigh;
	int LastSGSPoly;
	int LastSGSNeigh;
	int SavGolPoly;
	int SavGolNeigh;
	double *Derive;
	int LastSGPoly;
	int LastSGNeigh;
	bool LastTrackSrc;
	bool HasBeenCut;
	bool ExitProgram;
	bool NumbersWithDot;

	bool GetColumns(int NColumn,int &XColumn,int &YColumn);
	bool LoadCsvFile(char *Buffer,unsigned int FSize);
	bool LoadTirFile(char *Buffer,unsigned int FSize);
	void TrackPosition(bool XSource);
	bool AddDataPoint(char **ColumnsTxt,double **XData,double **YData,int &NPoints,
		 int &NAllocated,bool &InData,int XColumn,int YColumn,int Line);
	void WriteXFreq(double XFreq);
	void WriteXTime0(double XTime);
	void SetGraphSize(GraphImage *Control,int Width,int Height);
	void ConfigureColors();
	void CopyDataPoints(double *Data);

public:		// Déclarations de l'utilisateur
	int NPoints;
	double *XPlot,*YPlot;  //buffer to store the calculated curves
	double *YSmooth,*YDerv;

	void RecalculateGraphics();

	void SetBgMouseClick(bool Active);
	static void TrackMouseMove(void *Parent,bool InGraph,double x,double y);
	bool WriteToClipboard(const QString *Text);

	QColor GetBackgroundColor();
	void SetBackgroundColor(const QColor &Color);
	QColor GetDataColor();
	void SetDataColor(const QColor &Color);
	QColor GetSmoothColor();
	void SetSmoothColor(const QColor &Color);
	QColor GetDervColor();
	void SetDervColor(const QColor &Color);
	QColor GetBkgrColor();
	void SetBkgrColor(const QColor &Color);
};

#endif // MAINSCREEN_H
