#ifndef SELOUTFILE_H
#define SELOUTFILE_H

#include <QDialog>

namespace Ui {
class SelOutFile;
}

class SelOutFile : public QDialog
{
	Q_OBJECT
	
public:
	explicit SelOutFile(QWidget *parent = 0);
	~SelOutFile();

private slots:
	void on_BrowseDerv_clicked();
	void on_BrowseSmooth_clicked();

private:
	Ui::SelOutFile *ui;
};

#endif // SELOUTFILE_H
