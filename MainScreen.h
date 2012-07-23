//---------------------------------------------------------------------------
#ifndef MainScreenH
#define MainScreenH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include "GraphImage.h"
//---------------------------------------------------------------------------

#define MAIN_COLOR ((enum TColor)0xFFFFC0)  //color of the main curve
#define SMOOTH_COLOR ((enum TColor)0x00C000) //color of the smoothed curve

class TMainForm : public TForm
{
__published:	// Composants gérés par l'EDI
        TMainMenu *MainMenu;
        TMenuItem *FileMenu;
        TMenuItem *LoadMenu;
        TMenuItem *ExitMenu;
        TMenuItem *N1;
        TOpenDialog *OpenFileDlg;
        TGroupBox *GroupBox1;
        TEdit *GaussWidthCtrl;
        TLabel *Label5;
        TEdit *GaussNeighCtrl;
        TLabel *Label6;
        TMemo *Memo1;
        TGroupBox *GroupBox2;
        TLabel *Label7;
        TEdit *SavGolPolyCtrl;
        TEdit *SavGolNeighCtrl;
        TLabel *Label10;
        TMenuItem *SaveMenu;
        TMenuItem *SaveDerivativeMenu;
        TMenuItem *SaveSmoothMenu;
        TSaveDialog *SaveDialog;
        TMenuItem *SaveDataMenu;
    TImage *MainGraphCtrl;
    TImage *DervGraphCtrl;
    TMenuItem *Data1;
    TMenuItem *BackgroundMenu;
    TMenuItem *Options1;
    TMenuItem *SelectOutputFileMenu;
    TRadioButton *RawSmoothButton;
    TRadioButton *SavGolButton;
    TGroupBox *GroupBox3;
    TEdit *YOffsetCtrl;
    TLabel *Label9;
    TEdit *YGainCtrl;
    TLabel *Label8;
    TGroupBox *XAxisGroup;
    TEdit *XTime0;
    TLabel *Label4;
    TLabel *Label3;
    TLabel *Label2;
    TEdit *XFrequency;
    TLabel *Label1;
    TMenuItem *N2;
    TMenuItem *CopyMenu;
    TMenuItem *CopySmoothMenu;
    TMenuItem *CopyDeriveMenu;
    TMenuItem *CopyData;
    TEdit *SmoothMaxCtrl;
    TLabel *Label11;
    TMenuItem *CopyBkgrMenu;
    TEdit *DervMaxCtrl;
    TLabel *Label12;
    TGroupBox *GroupBox5;
    TLabel *Label13;
    TLabel *Label14;
    TEdit *YTracker;
    TEdit *XTracker;
    TRadioButton *TrackData;
    TRadioButton *TrackSmooth;
    TRadioButton *TrackDerv;
    TMenuItem *CutMenu;
    TRadioButton *TrackMouse;
    TLabel *ProgVersionLabel;
    void __fastcall ExitMenuClick(TObject *Sender);
    void __fastcall LoadMenuClick(TObject *Sender);
    void __fastcall XFrequencyKeyPress(TObject *Sender, char &Key);
    void __fastcall XFrequencyExit(TObject *Sender);
    void __fastcall XTime0KeyPress(TObject *Sender, char &Key);
    void __fastcall XTime0Exit(TObject *Sender);
    void __fastcall GaussWidthCtrlKeyPress(TObject *Sender, char &Key);
    void __fastcall GaussWidthCtrlExit(TObject *Sender);
    void __fastcall GaussNeighCtrlKeyPress(TObject *Sender, char &Key);
    void __fastcall GaussNeighCtrlExit(TObject *Sender);
    void __fastcall YGainCtrlKeyPress(TObject *Sender, char &Key);
    void __fastcall YGainCtrlExit(TObject *Sender);
    void __fastcall YOffsetCtrlKeyPress(TObject *Sender, char &Key);
    void __fastcall YOffsetCtrlExit(TObject *Sender);
    void __fastcall SavGolPolyCtrlKeyPress(TObject *Sender, char &Key);
    void __fastcall SavGolPolyCtrlExit(TObject *Sender);
    void __fastcall SavGolNeighCtrlKeyPress(TObject *Sender, char &Key);
    void __fastcall SavGolNeighCtrlExit(TObject *Sender);
        void __fastcall SaveMenuClick(TObject *Sender);
        void __fastcall SaveDerivativeMenuClick(TObject *Sender);
        void __fastcall SaveSmoothMenuClick(TObject *Sender);
        void __fastcall SaveDataMenuClick(TObject *Sender);
    void __fastcall BackgroundMenuClick(TObject *Sender);
    void __fastcall SelectOutputFileMenuClick(TObject *Sender);
    void __fastcall CopyDataClick(TObject *Sender);
    void __fastcall CopyBkgrClick(TObject *Sender);
    void __fastcall CopySmoothMenuClick(TObject *Sender);
    void __fastcall CopyDeriveMenuClick(TObject *Sender);
    void __fastcall CopyMenuClick(TObject *Sender);
    void __fastcall RawSmoothButtonClick(TObject *Sender);
    void __fastcall SavGolButtonClick(TObject *Sender);
    void __fastcall XTrackerExit(TObject *Sender);
    void __fastcall XTrackerKeyPress(TObject *Sender, char &Key);
    void __fastcall YTrackerExit(TObject *Sender);
    void __fastcall YTrackerKeyPress(TObject *Sender, char &Key);
    void __fastcall TrackDataClick(TObject *Sender);
    void __fastcall TrackSmoothClick(TObject *Sender);
    void __fastcall TrackDervClick(TObject *Sender);
    void __fastcall TrackMouseClick(TObject *Sender);
    void __fastcall CutMenuClick(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

private:	// Déclarations de l'utilisateur
    AnsiString DefaultFileName;
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

    bool LoadCsvFile(char *Buffer,unsigned int FSize);
    bool LoadTirFile(char *Buffer,unsigned int FSize);
    void TrackPosition(bool XSource);
    bool AddDataPoint(char **ColumnsTxt,double **XData,double **YData,int &NPoints,
         int &NAllocated,bool &InData,int XColumn,int YColumn,int Line);
    void WriteXFreq(double XFreq);
    void WriteXTime0(double XTime);
    void SetGraphSize(TImage *Control,int Width,int Height);

public:		// Déclarations de l'utilisateur
    int NPoints;
    double *XPlot,*YPlot;  //buffer to store the calculated curves
    double *YSmooth,*YDerv;
    ImageGraph *MainGraphic;
    ImageGraph *DervGraphic;

    __fastcall TMainForm(TComponent* Owner);
    __fastcall  virtual ~TMainForm(void);
    void RecalculateGraphics(void);
    void UpdateGraphics(void);

    static void BkgrLeftClick(void *Parent,double x,double y,Classes::TShiftState Shift);
    static void BkgrRightClick(void *Parent,double x,double y,Classes::TShiftState Shift);
    static void TrackMouseMove(void *Parent,bool InGraph,double x,double y);
    bool WriteToClipboard(char * Text);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
