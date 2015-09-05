#include "qexception.h"

/*=============================================================================*/
/*!
  Constructor of the exception.

  \param Msg The message to describe the exception.
 */
/*=============================================================================*/
QException::QException(const QString &Msg) throw() :
    Message(Msg)
{
}

/*=============================================================================*/
/*!
  Destructor of the exception.
 */
/*=============================================================================*/
QException::~QException() throw()
{

}

/*=============================================================================*/
/*!
  Return an ASCII text explaining the error.
 */
/*=============================================================================*/
const char *QException::what() const throw()
{
	return(Message.toLocal8Bit().constData());
}

/*=============================================================================*/
/*!
  Return a qstring explaining the error.
 */
/*=============================================================================*/
QString QException::qwhat() const throw()
{
	return(Message);
}
