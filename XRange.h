//---------------------------------------------------------------------------
#ifndef XRangeH
#define XRangeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TXRangeForm : public TForm
{
__published:	// Composants gérés par l'EDI
    TEdit *XStartText;
    TEdit *XStopText;
    TLabel *Label1;
    TLabel *Label2;
    TButton *OkButton;
    TButton *CancelButton;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// Déclarations de l'utilisateur
public:
    double XStart;
    double XStop;		// Déclarations de l'utilisateur
    __fastcall TXRangeForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TXRangeForm *XRangeForm;
//---------------------------------------------------------------------------
#endif
