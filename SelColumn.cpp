//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SelColumn.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSelectColumn *SelectColumn;
//---------------------------------------------------------------------------
__fastcall TSelectColumn::TSelectColumn(TComponent* Owner)
    : TForm(Owner)
{
  XColumn=-1;
  YColumn=-1;
}
//---------------------------------------------------------------------------

void TSelectColumn::GetColumn(int &XColumn, int &YColumn)
{
  XColumn=this->XColumn;
  YColumn=this->YColumn;
}

void TSelectColumn::PrepareList(int NColumns)
{
  int i;
  TListColumns *ColList;
  TListColumn *NewColumn;

  ColumnList->Items->Clear();
  ColList=ColumnList->Columns;
  for (i=0 ; i<NColumns ; i++)
   {
   NewColumn=ColList->Add();
   NewColumn->AutoSize=false; //true;
   NewColumn->Width=100;
   NewColumn->Caption="";
   }
  XDefine=true;
}

void TSelectColumn::AddLine(char **Columns,int NColumns)
{
  int i;
  TListItem *ListItem;

  ListItem=ColumnList->Items->Add();
  ListItem->Caption=Columns[0];
  for (i=1 ; i<NColumns ; i++)
   {
   ListItem->SubItems->Add(Columns[i]);
   }
}

void __fastcall TSelectColumn::ColumnListColumnClick(TObject *Sender,
      TListColumn *Column)
{
  if (XDefine)
   {
   if (XColumn>=0) ColumnList->Columns->Items[XColumn]->Caption="";
   XColumn=Column->Index;
   if (XColumn==YColumn) YColumn=-1;
   Column->Caption="X";
   }
  else
   {
   if (YColumn>=0) ColumnList->Columns->Items[YColumn]->Caption="";
   YColumn=Column->Index;
   if (XColumn==YColumn) XColumn=-1;
   Column->Caption="Y";
   }
  XDefine=!XDefine;
}
//---------------------------------------------------------------------------

