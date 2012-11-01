#include "selectcolumn.h"
#include "ui_selectcolumn.h"
#include "Utils.h"

SelectColumn::SelectColumn(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectColumn)
{
	ui->setupUi(this);

	bool Success=connect(ui->ColumnList->header(),SIGNAL(sectionClicked(int)),this,SLOT(HeaderClicked(int)));
	if (Success)
		ui->ColumnList->header()->setClickable(true);
	else
		WriteMsg(__FILE__,__LINE__,tr("Cannot detect clicks on headers"));

	XColumn=-1;
	YColumn=-1;
}

SelectColumn::~SelectColumn()
{
	delete ui;
}

void SelectColumn::SetColumn(int XColumn, int YColumn)
{
	this->XColumn=XColumn;
	this->YColumn=YColumn;
	SetXColumn(XColumn);
	SetYColumn(YColumn);
}

void SelectColumn::GetColumn(int &XColumn, int &YColumn)
{
	XColumn=this->XColumn;
	YColumn=this->YColumn;
}

void SelectColumn::PrepareList(int NColumns)
{
	ui->ColumnList->clear();
	QStringList items;
	for (int i=0 ; i<NColumns ; i++)
		items.push_back("");
	ui->ColumnList->setHeaderLabels(items);

	/*ColList=ColumnList->Columns;
	for (i=0 ; i<NColumns ; i++)
	{
		NewColumn=ColList->Add();
		NewColumn->AutoSize=false; //true;
		NewColumn->Width=100;
		NewColumn->Caption="";
	}*/
	XDefine=true;
}

void SelectColumn::AddLine(char **Columns,int NColumns)
{
	int i;
	QTreeWidgetItem *ListItem=new QTreeWidgetItem();

	for (i=0 ; i<NColumns ; i++)
	{
		ListItem->setText(i,Columns[i]);
	}
	ui->ColumnList->addTopLevelItem(ListItem);
}

void SelectColumn::ColumnClicked(int Column)
{
	if (XDefine)
	{
		SetXColumn(Column);
	}
	else
	{
		SetYColumn(Column);
	}
	XDefine=!XDefine;
}

void SelectColumn::SetXColumn(int Column)
{
	QTreeWidgetItem *Header=ui->ColumnList->headerItem();
	if (XColumn>=0) Header->setText(XColumn,"");
	XColumn=Column;
	if (XColumn==YColumn) YColumn=-1;
	Header->setText(XColumn,"X");
}

void SelectColumn::SetYColumn(int Column)
{
	QTreeWidgetItem *Header=ui->ColumnList->headerItem();
	if (YColumn>=0) Header->setText(YColumn,"");
	YColumn=Column;
	if (XColumn==YColumn) XColumn=-1;
	Header->setText(YColumn,"Y");
}

void SelectColumn::on_ColumnList_itemClicked(QTreeWidgetItem *item,int column)
{
	item=item;//compiler pacifier
	ColumnClicked(column);
}

void SelectColumn::HeaderClicked(int column)
{
	ColumnClicked(column);
}
