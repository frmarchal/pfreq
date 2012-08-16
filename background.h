#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QDialog>

namespace Ui {
class BackgroundForm;
}

//! Maximum number of points for the background.
#define MAX_BKGRPOINTS 20

class BackgroundForm : public QDialog
{
	Q_OBJECT
	
public:
	explicit BackgroundForm(QWidget *parent = 0);
	~BackgroundForm();
	
	double XBkgr[MAX_BKGRPOINTS+2];   //points of the background to be drawn
	double YBkgr[MAX_BKGRPOINTS+2];   //points of the background to be drawn
	int NBgPoints;
	bool BackgroundOk;               //set if the background has been prepared

	void AddPoint(double x, double y);	// Déclarations de l'utilisateur
	void RemovePoint(double x, double y);
	bool PrepareBkgr(double Time0, double XFreq,int NPts);
	double GetNextSlope(double * x);
	void GetBackground(double XFreq,double *x,double *Slope,double *Offset);
	bool CalculateAutoBackground(void);

private slots:
	void on_CalcAutoButton_clicked();
	void on_DeleteAutoButton_clicked();
	void on_DeleteAllButton_clicked();
	void on_BackgroundMode_currentChanged(int Index);
	void on_LRFirst_editingFinished();
	void on_AverageStart_editingFinished();
	void on_LinearRegButton_toggled(bool Checked);
	void on_AverageButton_toggled(bool Checked);

signals:
	void UpdateGraphics();

private:
	Ui::BackgroundForm *ui;

	double XData[MAX_BKGRPOINTS];   //points of the background
	double YData[MAX_BKGRPOINTS];   //points of the background
	int NPoints;                    //number of points in the background
	double Slope,Offset;            //slope and offset of the background in automatic mode
	bool AutoDefined;               //Set when slope and offset are valid and have been calculated

	void Configure();
};

#endif // BACKGROUND_H
