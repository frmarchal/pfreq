#ifndef SELECTCOLORS_H
#define SELECTCOLORS_H

#include <QDialog>
#include <qlistwidget.h>

namespace Ui {
class SelectColors;
}

class SelectColors : public QDialog
{
	Q_OBJECT

public:
	explicit SelectColors(QWidget *parent = 0);
	~SelectColors();

private slots:
	void on_ColorList_itemClicked(QListWidgetItem *Item);

private:
	Ui::SelectColors *ui;

	enum ColorItemEnum
	{
		DataBackground,
		DataCurve,
		SmoothCurve,
		BkgrCurve,
		DervCurve
	};


	void InsertItem(const QString &Label,const QColor &Color,ColorItemEnum Type);
};

#endif // SELECTCOLORS_H
