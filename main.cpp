#include <QtGui/QApplication>
#include "mainscreen.h"
#include "config.h"

//! Configuration file for this program.
ConfigObject *ConfigFile=NULL;

void Configure()
{
	QString Name=QCoreApplication::applicationFilePath();
	QFileInfo FName(Name);
	QFileInfo ConfigName(FName.dir(),FName.completeBaseName()+".ini");
	ConfigFile=new ConfigObject(ConfigName.filePath());
}

int Run(QApplication &a)
{
	MainScreen w;
	w.show();

	int RetCode=a.exec();
	return(RetCode);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// locale must be C to parse numbers with decimal dots
	setlocale(LC_NUMERIC,"C");

	Configure();

	int RetCode=Run(a);

	delete ConfigFile;
	return(RetCode);
}
