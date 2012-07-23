//---------------------------------------------------------------------------
#ifndef SelOutFileH
#define SelOutFileH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <stdlib.h>
//---------------------------------------------------------------------------
class TOutputFile : public TForm
{
__published:	// Composants gérés par l'EDI
    TEdit *DeriveName;
    TLabel *Label1;
    TButton *BrowseDerv;
    TSaveDialog *SaveOutputFiles;
    TLabel *Label2;
    TEdit *SmoothName;
    TButton *BrowseSmooth;
    TButton *OkButton;
    TButton *CancelButton;
    void __fastcall BrowseDervClick(TObject *Sender);
    void __fastcall BrowseSmoothClick(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
private:	// Déclarations de l'utilisateur
    char SmoothFile[_MAX_PATH];
    char DeriveFile[_MAX_PATH];
public:		// Déclarations de l'utilisateur
    __fastcall TOutputFile(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TOutputFile *OutputFile;
//---------------------------------------------------------------------------
#endif
