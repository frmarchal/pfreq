#ifndef XRANGE_H
#define XRANGE_H

#include <QDialog>

namespace Ui {
class XRange;
}

class XRange : public QDialog
{
	Q_OBJECT
	
public:
	explicit XRange(QWidget *parent = 0);
	~XRange();

	void GetRange(double *XStart,double *XStop);

private:
	Ui::XRange *ui;

	double XStart;
    double XStop;
};

#endif // XRANGE_H
