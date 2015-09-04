#include <qcolordialog.h>
#include "mainscreen.h"
#include "selectcolors.h"
#include "ui_selectcolors.h"

extern MainScreen *MainForm;

SelectColors::SelectColors(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectColors)
{
	ui->setupUi(this);

	InsertItem(tr("Data background"),MainForm->GetBackgroundColor(),DataBackground);
	InsertItem(tr("Data curve"),MainForm->GetDataColor(),DataCurve);
	InsertItem(tr("Smoothed curve"),MainForm->GetSmoothColor(),SmoothCurve);
	InsertItem(tr("Background curve"),MainForm->GetBkgrColor(),BkgrCurve);
	InsertItem(tr("Derivative curve"),MainForm->GetDervColor(),DervCurve);
}

SelectColors::~SelectColors()
{
	delete ui;
}

void SelectColors::InsertItem(const QString &Label,const QColor &Color,ColorItemEnum Type)
{
	QListWidgetItem *Item=new QListWidgetItem(Label,ui->ColorList);
	QBrush Brush(Color);
	Item->setBackground(Brush);
	int ColVal=Color.value();
	if (ColVal<80)//arbitrary value to detect a dark background.
		Item->setForeground(Qt::white);
	Item->setData(Qt::UserRole,Type);
}

void SelectColors::on_ColorList_itemClicked(QListWidgetItem *Item)
{
	QBrush Brush=Item->background();
	QColor Default=Brush.color();

	QColor Color=QColorDialog::getColor(Default,this);
	if (!Color.isValid()) return;

	Brush.setColor(Color);
	Item->setBackground(Brush);
	int ColVal=Color.value();
	if (ColVal<80)//arbitrary value to detect a dark background.
		Item->setForeground(Qt::white);
	else
		Item->setForeground(Qt::black);

	QVariant TypeVar=Item->data(Qt::UserRole);
	enum ColorItemEnum Type=static_cast<enum ColorItemEnum>(TypeVar.toInt());
	switch (Type)
	{
		case DataBackground:
			MainForm->SetBackgroundColor(Color);
			break;
		case DataCurve:
			MainForm->SetDataColor(Color);
			break;
		case SmoothCurve:
			MainForm->SetSmoothColor(Color);
			break;
		case BkgrCurve:
			MainForm->SetBkgrColor(Color);
			break;
		case DervCurve:
			MainForm->SetDervColor(Color);
			break;
	}
}
