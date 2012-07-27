#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <qstring.h>

void WriteMsg(const char *FuncName,int LineNum,const QString &Message);
void Purge(double *&Ptr);
bool StrToDouble(const char *String,double *Value);

#endif //UTILS_HEADER
