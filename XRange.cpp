//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdio.h>
#include <stdlib.h>

#include "XRange.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TXRangeForm *XRangeForm;
//---------------------------------------------------------------------------
__fastcall TXRangeForm::TXRangeForm(TComponent* Owner)
    : TForm(Owner)
{
  XStart=0.;
  XStop=0.;
}
//---------------------------------------------------------------------------
void __fastcall TXRangeForm::FormShow(TObject *Sender)
{
  char Text[50];

  sprintf(Text,"%.2f",XStart);
  XStartText->Text=Text;
  sprintf(Text,"%.2f",XStop);
  XStopText->Text=Text;
}
//---------------------------------------------------------------------------
void __fastcall TXRangeForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  XStart=atof(XStartText->Text.c_str());
  XStop=atof(XStopText->Text.c_str());
}
//---------------------------------------------------------------------------
