#ifndef SELECTCOLUMN_H
#define SELECTCOLUMN_H

#include <QDialog>
#include <qtreewidget.h>

namespace Ui {
class SelectColumn;
}

class SelectColumn : public QDialog
{
	Q_OBJECT
	
public:
	explicit SelectColumn(QWidget *parent = 0);
	~SelectColumn();
	
	void GetColumn(int &XColumn, int &YColumn);
	void PrepareList(int NColumns);
	void AddLine(char **Columns,int NColumns);

protected slots:
	void on_ColumnList_itemClicked(QTreeWidgetItem *item,int column);
	void HeaderClicked(int column);

private:
	Ui::SelectColumn *ui;

	int XColumn;
	int YColumn;
	bool XDefine;

	void ColumnClicked(int Column);
};

#endif // SELECTCOLUMN_H
