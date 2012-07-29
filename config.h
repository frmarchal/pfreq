#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#include <qcoreapplication.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qreadwritelock.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qmutex.h>
#include <qelapsedtimer.h>
#include <list>
#include "qexception.h"

class ConfigObject : public QObject
{
	Q_OBJECT

public:
	ConfigObject(const QString &DefaultFile,const QString &PreConfigSection);
	ConfigObject(const QString &DefaultFile);
	~ConfigObject();

	QString Config_GetPreFile();
	QString Config_GetFile();
	QString Config_GetBasePath();

	QString Config_GetString(const QString &Section,const QString &Item,const QString &Default,const QString &Comment="");
	int Config_GetInt(const QString &Section,const QString &Item, int Default,const QString &Comment="");
	unsigned int Config_GetHex(const QString &Section,const QString &Item, unsigned int Default,const QString &Comment="");
	unsigned int Config_GetUInt(const QString &Section,const QString &Item, unsigned int Default,const QString &Comment="");
	double Config_GetDouble(const QString &Section,const QString &Item, double Default,const QString &Comment="");
	QFileInfo Config_GetFileName(const QString &Section,const QString &Item, const QString &Default,const QString &Comment="");
	QStringList Config_GetSection(const QString Section);

	bool Config_IsEmpty(const QString &Section, const QString &Item);

	QString Config_GetStringNoWrite(const QString &Section,const QString &Item, const QString Default);
	int Config_GetIntNoWrite(const QString &Section,const QString &Item, int Default);
	unsigned int Config_GetHexNoWrite(const QString &Section,const QString &Item,unsigned int Default);
	unsigned int Config_GetUIntNoWrite(const QString &Section,const QString &Item, unsigned int Default);
	double Config_GetDoubleNoWrite(const QString &Section,const QString &Item, double Default);
	QStringList Config_GetSectionNoWrite(const QString &Section);

	void Config_WriteString(const QString &Section,const QString &Item,const QString &Value);
	void Config_WriteInt(const QString &Section,const QString &Item,int Value);
	void Config_WriteHex(const QString &Section,const QString &Item,unsigned int Value);
	void Config_WriteUInt(const QString &Section,const QString &Item,unsigned int Value);
	void Config_WriteDouble(const QString &Section,const QString &Item,double Value);
	void Config_WriteFileName(const QString &Section,const QString &Item,const QFileInfo &Value);
	void Config_WriteFileName(const QString &Section,const QString &Item,const QString &Value);

	class ConfigError : public QException
	{
	public:
		ConfigError(const QString &Msg) : QException(Msg) {}
	};

public slots:
	void timerEvent(QTimerEvent *Event);

private:
	class SectionObject;//forward declaration
	class EntryObject
	{
	public:
		bool Inactive;
		bool New;
		QStringList Comment;
		QString Name;
		QString Value;
		SectionObject *Section;
	};
	typedef std::list<EntryObject *> EntryListType;

	class SectionObject
	{
	public:
		~SectionObject();
		bool Inactive;
		bool New;
		QStringList Comment;
		QString Name;
		QString EndOfLineComment;
		EntryListType Entries;

		EntryObject *FindEntry(const QString &Entry);
		EntryObject *AppendEntry(const QString &Entry);
	};
	typedef std::list<SectionObject *> SectionListType;

	class FileObject
	{
	public:
		//! The parent object.
		ConfigObject *Parent;
		//! The name of the file
		QFileInfo File;
		//! The last modification time of the file.
		QDateTime ModTime;
		//! The last time it was reloaded.
		QElapsedTimer LastLoadTime;
		//! \c True if the file needs to be saved.
		bool Unsaved;
		//! Sections contained in the file.
		SectionListType Sections;

		FileObject(ConfigObject *Parent);
		~FileObject();
		bool LoadFile();
		SectionObject *FindSection(const QString &Section);
		SectionObject *AppendSection(const QString &Section);
		EntryObject *GetOrCreate(const QString &Section,const QString &Entry,const QString &Default);
		EntryObject *GetNoCreate(const QString &Section,const QString &Entry);
		void Store(const QString &Section,const QString &Entry,const QString &Value);
		void SaveFile();
		void Modified();
	};

	//! The base path to compute all the filenames. It is the application root directory.
	QDir BasePath;
	//! The pre configuration file we used.
	QFileInfo PreConfigFile;
	//! The configuration file we are using.
	FileObject ConfigFile;
	//! Prevent any reading while the content is updated.
	QReadWriteLock AccessLock;
	//! The ID of the timer to save the file.
	int SaveTimer;
	//! A mutex to prevent concurrent accesses to the save timer.
	QMutex SaveTimerCs;

private:
	QString Config_GetProgName();
	void StartSaveTimer();
	void StopSaveTimer();
	bool Load();
	QStringList FormatComment(const QString &Text, int Width=78);
};

#endif //CONFIG_HEADER

