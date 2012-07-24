#include <qfileinfo.h>
#include <qmessagebox.h>
#include "Utils.h"

/*=============================================================================*/
/*!
  Display a message and print the calling function name and line
  number in the title.

  \param FuncName Name of the calling file (usually __FILE__).
  \param LineNum Line number from where this function is called (usually __LINE__).
  \param Message Message to display.

  \author F. Marchal
  \date
	 \arg 1999-10-18: Fma: Creation
 */
/*=============================================================================*/
void WriteMsg(const char *FuncName,int LineNum,const QString &Message)
{
	QMessageBox MsgBox;
	QFileInfo FullName(FuncName);
	MsgBox.setWindowTitle(QString(FullName.fileName())+" ("+QString::number(LineNum)+")");
	MsgBox.setText(Message);
	MsgBox.exec();
}

/*=============================================================================*/
/*!
  Free a pointer to a double and reset the pointer to NULL.

  \param Ptr Pointer to purge.

  \author F. Marchal
  \date 2001-11-21
 */
/*=============================================================================*/
void Purge(double *&Ptr)
{
	if (!Ptr) return;
	free(Ptr);
	Ptr=NULL;
}
