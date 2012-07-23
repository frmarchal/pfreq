//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SelOutFile.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOutputFile *OutputFile;

extern char ConfigFile[_MAX_PATH];  //configuration file for this program

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: TOutputFile
//
// Type: Function
//
// Applies To: TOutputFile
//
// Description: Constructor of the dialog box to select the output files.
//
// Usage: __fastcall TOutputFile::TOutputFile(TComponent* Owner)
//    : TForm(Owner)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
__fastcall TOutputFile::TOutputFile(TComponent* Owner)
    : TForm(Owner)
{
  GetPrivateProfileString("Output","Smooth","",SmoothFile,_MAX_PATH,ConfigFile);
  SmoothName->Text=SmoothFile;
  GetPrivateProfileString("Output","Derivative","",DeriveFile,_MAX_PATH,ConfigFile);
  DeriveName->Text=DeriveFile;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: BrowseDervClick
//
// Type: Function
//
// Applies To: TOutputFile
//
// Description: The user want to select the output file for the derivative.
//
// Usage: void __fastcall TOutputFile::BrowseDervClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TOutputFile::BrowseDervClick(TObject *Sender)
{
  SaveOutputFiles->FileName=DeriveFile;
  if (SaveOutputFiles->Execute())
   {
   strcpy(DeriveFile,SaveOutputFiles->FileName.c_str());
   DeriveName->Text=DeriveFile;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: BrowseSmoothClick
//
// Type: Function
//
// Applies To: TOutputFile
//
// Description: The user want to select the output file for the smoothed curve.
//
// Usage: void __fastcall TOutputFile::BrowseSmoothClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TOutputFile::BrowseSmoothClick(TObject *Sender)
{
  SaveOutputFiles->FileName=SmoothFile;
  if (SaveOutputFiles->Execute())
   {
   strcpy(SmoothFile,SaveOutputFiles->FileName.c_str());
   SmoothName->Text=SmoothFile;
   }
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: OkButtonClick
//
// Type: Function
//
// Applies To: TOutputFile
//
// Description: The user wants to validate its selection.
//
// Usage: void __fastcall TOutputFile::OkButtonClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TOutputFile::OkButtonClick(TObject *Sender)
{
  WritePrivateProfileString("Output","Smooth",SmoothFile,ConfigFile);
  WritePrivateProfileString("Output","Derivative",DeriveFile,ConfigFile);
  ModalResult=2;
}

////////////////////////// FUNCTION DOCUMENTATION ////////////////////////////
// Name: CancelButtonClick
//
// Type: Function
//
// Applies To: TOutputFile
//
// Description: The user wants to cancel its selection.
//
// Usage: void __fastcall TOutputFile::CancelButtonClick(TObject *Sender)
//
// Returns:
//
// Remarks:
//
// System: Borland C++ Builder 4 - Win95
// Author: Marchal F
//
// Date: 1/12/2001
//
// Revision:
//
//////////////////////////////////// EOD /////////////////////////////////////
void __fastcall TOutputFile::CancelButtonClick(TObject *Sender)
{
  ModalResult=1;
}

