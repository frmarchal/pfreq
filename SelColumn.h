//---------------------------------------------------------------------------
#ifndef SelColumnH
#define SelColumnH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TSelectColumn : public TForm
{
__published:	// Composants gérés par l'EDI
    TListView *ColumnList;
    TButton *OkButton;
    TButton *CancelButton;
    void __fastcall ColumnListColumnClick(TObject *Sender,
          TListColumn *Column);
private:
    int XColumn;
    int YColumn;	// Déclarations de l'utilisateur
    bool XDefine;
public:		// Déclarations de l'utilisateur
    __fastcall TSelectColumn(TComponent* Owner);
    void GetColumn(int &XColumn, int &YColumn);
    void PrepareList(int NColumns);
    void AddLine(char **Columns,int NColumns);
};
//---------------------------------------------------------------------------
extern PACKAGE TSelectColumn *SelectColumn;
//---------------------------------------------------------------------------
#endif
