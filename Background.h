//---------------------------------------------------------------------------
#ifndef BackgroundH
#define BackgroundH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

#define MAX_BKGRPOINTS 20   //maximum number of points for the background

class TBackgroundForm : public TForm
{
__published:	// Composants gérés par l'EDI
    TPageControl *BackgroundMode;
    TTabSheet *ManualSheet;
    TListView *BkgrPointsList;
    TButton *DeleteSelectionButton;
    TButton *DeleteAllButton;
    TTabSheet *AutomaticSheet;
    TRadioButton *LinearRegButton;
    TEdit *LRFirst;
    TLabel *Label1;
    TEdit *LRLast;
    TButton *CalcAutoButton;
    TButton *DeleteAutoButton;
    TRadioButton *AverageButton;
    TEdit *AverageNPoints;
    TLabel *Label2;
    TEdit *AverageStart;
    void __fastcall CalcAutoButtonClick(TObject *Sender);
    void __fastcall DeleteAutoButtonClick(TObject *Sender);
    void __fastcall LRFirstKeyPress(TObject *Sender, char &Key);
    void __fastcall DeleteAllButtonClick(TObject *Sender);
    void __fastcall AverageKeyPress(TObject *Sender, char &Key);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall BackgroundModeChange(TObject *Sender);
    void __fastcall FormHide(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
private:
    double XData[MAX_BKGRPOINTS];   //points of the background
    double YData[MAX_BKGRPOINTS];   //points of the background
    int NPoints;                    //number of points in the background
    double Slope,Offset;            //slope and offset of the background in automatic mode
    bool AutoDefined;               //Set when slope and offset are valid and have been calculated
public:		// Déclarations de l'utilisateur
    double XBkgr[MAX_BKGRPOINTS+2];   //points of the background to be drawn
    double YBkgr[MAX_BKGRPOINTS+2];   //points of the background to be drawn
    int NBgPoints;
    bool BackgroundOk;               //set if the background has been prepared

    __fastcall TBackgroundForm(TComponent* Owner);
    void AddPoint(double x, double y);	// Déclarations de l'utilisateur
    void RemovePoint(double x, double y);
    bool PrepareBkgr(double Time0, double XFreq,int NPts);
    double GetNextSlope(double * x);
    void GetBackground(double XFreq,double *x,double *Slope,double *Offset);
    bool CalculateAutoBackground(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TBackgroundForm *BackgroundForm;
//---------------------------------------------------------------------------
#endif
