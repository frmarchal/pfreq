#ifndef QEXCEPTION_H
#define QEXCEPTION_H

#include <qstring.h>
#include <exception>

/*!
\brief A generic exception to derive any exception thrown by the program.
*/
class QException : public std::exception
{
public:
	QException(const QString &Msg) throw();
	~QException() throw();
	const char *what() const throw();
	QString qwhat() const throw();
private:
	QString Message;
};

#endif // QEXCEPTION_H
