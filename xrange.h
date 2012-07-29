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
	
private:
	Ui::XRange *ui;
};

#endif // XRANGE_H
