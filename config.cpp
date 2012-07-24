/*! \file
\brief Configuration helper functions.
*/
#include <qdir.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qcoreapplication.h>
#include <qvariant.h>
#include <QtDebug>
#include <qobject.h>
#include <qtextstream.h>
#include "config.h"
//#include "LogFile.h"

//! The time to check if the file was modified and must be reloaded (in seconds).
#define CHECK_FOR_RELOAD_TIME 20

/*=============================================================================*/
/*!
  Set the configuration file name. This function must be called before any other
  function in this module.
  
  The real configuration file is found in a file named "preconfig.ini" in the application
  directory. Its sole purpose is to provide the name of the real configuration file.
  This way, it is possible to use another configuration setting simply by changing
  the preconfiguration file or to store the configuration file in another directory
  simply by stating the full path in the preconfiguration file.
  
  If the preconfiguration file doesn't exist, it is created. If it is empty, the
  default file name passed to the function is used to initialize it.
  
  It is recommanded to use differents configuration files name for differents settings
  to prevent accidental file overwritting between files with same names.
  
	The name of the real configuration file is found in a section of the preconfig.ini
	file. The entry we look for is Config but we may look in three different section.

	First, we may look in a section whose name is passed to the function in \a PreConfigSection.
	If it is NULL, we don't use it. You may use argv[1] as this parameter to allow
	the user to change the behaviour of the program simply by adding a name after the
	program's name in the shortcut used ot launch the program. If a string is passed
	in \a PreConfigSection, it will be added to the preconfig.ini if it doesn't exists but
	with an empty name. An empty name is ignored by the program.

	\code
	const char *PreConfigSection=NULL;
	
	if (argc>1) PreConfigSection=argv[1];
  Config_SetFile("Config.ini",PreConfigSection);
	\endcode
	
	The second section we look at is the one named after the name of the program (without
	path and extension). With this section, it is possible to change the behaviour of the
	program by changing the name of the application. No section is created if none is
	found in order not to clutter the preconfig file if this section is not needed.
	
	The third section is the default section and it is named [Default]. It is added to
	the preconfig file if it is not found and the config file is set to the default
	passed to the function.
  
  \param DefaultFile The name of the configuration file to use if none is found in
         the preconfiguration file. If it is NULL, "config.ini" is used instead.
  \param PreConfigSection If not NULL, that section is looked into the preconfig
         file to find the name of the real configuration file.
  
  \retval 0 No error.
  \retval -1 Cannot get the preconfiguration file.
  \retval -2 Invalid configuration file in preconfiguration file.
  \retval -3 No configuration file found.

  \date 2007-05-18
 */
/*=============================================================================*/
ConfigObject::ConfigObject(const QString &DefaultFile,const QString &PreConfigSection) :
	ConfigFile(this),
	SaveTimer(0)
{
	QString ConfFile;
	const char *EntryName="Config";
	QString EmptyStatusCode=tr("Configuration file not found. To initialize it with default values, create an empty file");

	BasePath=QDir::currentPath();
	PreConfigFile.setFile(BasePath,"preconfig.ini");
	FileObject PreConfigObj(NULL);
	PreConfigObj.File=PreConfigFile;
	PreConfigObj.LoadFile();
	
	// try to get the section passed to the function
	if (!PreConfigSection.isEmpty())
	{
		EntryObject *ConfFileEntry=PreConfigObj.GetOrCreate(PreConfigSection,EntryName,"");
		if (ConfFileEntry->New)
		{
			ConfFileEntry->Comment=FormatComment("Please adjust the path to the real configuration file.");
			if (ConfFileEntry->Section->New) ConfFileEntry->Section->Comment=FormatComment("\nPrefered configuration section requested by the program.\n");
		}
		ConfFile=ConfFileEntry->Value;
		EmptyStatusCode=tr("Configuration file not found. To initialize it with default values, create an empty"
				" file with the name found in the preconfig section");
	}
	
	// if it was not found, try to find a section with the name of the application
	if (ConfFile.isEmpty())
	{
		QString ProgName=Config_GetProgName();
		EntryObject *ConfFileEntry=PreConfigObj.GetNoCreate(ProgName,EntryName);
		ConfFile=(ConfFileEntry!=NULL) ? ConfFileEntry->Value : "";
		EmptyStatusCode=tr("Configuration file not found. To initialize it with default values, create an empty"
						   " file with the name found in the section named after the program");
	}
	
	// if still no config file, try the default location
	if (ConfFile.isEmpty())
	{
		EntryObject *ConfFileEntry=PreConfigObj.GetOrCreate("Default",EntryName,(DefaultFile.isEmpty()) ? "config.ini" : DefaultFile);
		if (ConfFileEntry->New)
		{
			ConfFileEntry->Comment+=FormatComment("Real configuration file to read.\n"
												  "You may want to give it a unique file name to prevent any accidental overwriting with "
												  "an identicaly named file.\n"
												  "Don't forget to create an empty file with that name. It will be initialized by the "
												  "program with some default values you'll have to review.");
			if (ConfFileEntry->Section->New) ConfFileEntry->Section->Comment=FormatComment("\nDefault configuration section if no more specific sections exist.\n");
		}
		ConfFile=ConfFileEntry->Value;
		EmptyStatusCode=tr("Configuration file not found. To initialize it with default values, create an empty"
						   " file with the name found in the default section");
	}
	if (PreConfigObj.Unsaved)
		PreConfigObj.SaveFile();

	ConfigFile.File.setFile(BasePath,ConfFile);
#ifdef LOGFILE_HEADER
	ProdLog_Write(__FILE__,__LINE__,LOGL_ProgDebug,QString("Using configuration file %1").arg(ConfigFile.File.filePath()));
#endif
	qDebug() << "Using configuration file" << ConfigFile.File.filePath();

	// Test if the file exists
	if (!ConfigFile.File.exists() || !ConfigFile.File.isReadable())
	{
		throw ConfigError(EmptyStatusCode);
	}

	// update the base path to be the path of the config file making all files relative to the configuration file actually used
	if (!ConfigFile.File.canonicalPath().isEmpty() && ConfigFile.File.canonicalPath()!=BasePath.canonicalPath())
		BasePath.setPath(ConfigFile.File.canonicalPath());
}

/*=============================================================================*/
/*!
  Set the default configuration file without resorting to a preconfiguration file.
  
  \param DefaultFile The name of the configuration. If it is NULL, "config.ini" 
         is used instead.
  
  \retval 0 No error.
  \retval -1 Cannot get the configuration file.

  \date 2007-07-11
 */
/*=============================================================================*/
ConfigObject::ConfigObject(const QString &DefaultFile) :
	ConfigFile(this),
	SaveTimer(0)
{
	BasePath=QDir::currentPath();
	if (DefaultFile.isEmpty())
		ConfigFile.File.setFile(BasePath,"config.ini");
	else
	{
		ConfigFile.File.setFile(BasePath,DefaultFile);
		// update the base path to be the path of the config file making all files relative to the configuration file actually used
		if (!ConfigFile.File.canonicalPath().isEmpty() && ConfigFile.File.canonicalPath()!=BasePath.canonicalPath())
			BasePath.setPath(ConfigFile.File.canonicalPath());
	}
}

/*=============================================================================*/
/*!
  Destroy the object and save it if necessary.
 */
/*=============================================================================*/
ConfigObject::~ConfigObject()
{
	if (ConfigFile.Unsaved)
		ConfigFile.SaveFile();
}

/*=============================================================================*/
/*!
  Load the file.

  \param FileData The buffer to store the file.

  \return \c True on success or \c false on error.
 */
/*=============================================================================*/
bool ConfigObject::FileObject::LoadFile()
{
	LastLoadTime.start();
	if (!File.exists())
	{
#ifdef LOGFILE_HEADER
		ProdLog_Write(__FILE__,__LINE__,LOGL_ProgFailure,QString("[all,,disk error]Unknown configuration file %1").arg(File.filePath()));
#endif
		return(false);
	}
	QDateTime ModTime=File.lastModified();
	if (this->ModTime==ModTime)  //is it the same file
	{
		return(true);
	}
	this->ModTime=ModTime;

	// read the file
#ifdef LOGFILE_HEADER
	ProdLog_Write(__FILE__,__LINE__,LOGL_ProgDebug,QString("Reading configuration file %1").arg(File.filePath()));
#endif
	QFile hFile(File.filePath());
	if (!hFile.open(QIODevice::ReadOnly))
	{
#ifdef LOGFILE_HEADER
		ProdLog_Write(__FILE__,__LINE__,LOGL_ProgFailure,QString("[all,,disk error]Cannot open configuration file %1 (error %2)")
					  .arg(File.filePath()).arg(hFile.errorString()));
#endif
		return(false);
	}
	Sections.clear();
	qint64 FSize=hFile.size();
	if (FSize>0)
	{
		QTextStream Stream(&hFile);
		QStringList Comment;
		SectionObject *Section=NULL;
		while (!Stream.atEnd())
		{
			QString Line=Stream.readLine(512);
			if (Line.isEmpty() || Line.at(0)=='#' || Line.at(0)==' ' || Line.at(0)=='\t')
			{
				Comment.append(Line);
				continue;
			}
			if (Line.at(0)=='[' || (Line.at(0)==';' && Line.at(1)=='['))
			{
				int Start=Line.indexOf('[');
				int EndOfSection=Line.indexOf(']',Start+1);
				if (EndOfSection<0)
				{
					Comment.append(Line);
					continue;
				}
				while (Comment.size()>0 && Comment.at(0).isEmpty()) Comment.pop_front();
				SectionObject *NewSection=new SectionObject;
				NewSection->Inactive=(Start!=0);
				NewSection->New=false;
				NewSection->Comment=Comment;
				NewSection->Name=Line.mid(Start+1,EndOfSection-Start-1);
				NewSection->EndOfLineComment=Line.mid(EndOfSection+1);
				Sections.push_back(NewSection);
				Section=NewSection;
				Comment.clear();
				continue;
			}
			if (!Section)//first section without header
			{
				while (Comment.size()>0 && Comment.at(0).isEmpty()) Comment.pop_front();
				SectionObject *NewSection=new SectionObject;
				NewSection->Inactive=false;
				NewSection->New=false;
				NewSection->Comment.clear();
				NewSection->Name="";
				NewSection->EndOfLineComment="";
				Sections.push_back(NewSection);
				Section=NewSection;
			}
			int SepIdx=Line.indexOf('=');
			if (SepIdx<0)
			{
				Comment.append(Line);
				continue;
			}
			{
				int Start=(Line.at(0)==';') ? 1 : 0;
				EntryObject *Entry=new EntryObject;
				Entry->Section=Section;
				Entry->Comment=Comment;
				Entry->New=false;
				Entry->Inactive=(Start!=0);
				Entry->Name=Line.mid(Start,SepIdx-Start);
				Entry->Value=Line.mid(SepIdx+1);
				Section->Entries.push_back(Entry);
				Comment.clear();
			}
		}
	}
	hFile.close();

	return(true);
}

/*=============================================================================*/
/*!
  Load the file if necessary.
 */
/*=============================================================================*/
bool ConfigObject::Load()
{
	AccessLock.lockForWrite();
	if (ConfigFile.LastLoadTime.isValid() && ConfigFile.LastLoadTime.elapsed()<CHECK_FOR_RELOAD_TIME*1000)
	{
		AccessLock.unlock();
		return(true);
	}

	bool Result=ConfigFile.LoadFile();
	AccessLock.unlock();
	if (!Result)
	{
		qWarning() << "Failed to load configuration file" << ConfigFile.File.fileName();
		return(false);
	}
	return(true);
}

/*=============================================================================*/
/*!
  Save the content of the file.
 */
/*=============================================================================*/
void ConfigObject::FileObject::SaveFile()
{
	Unsaved=false;
	if (Parent) Parent->StopSaveTimer();
#ifdef LOGFILE_HEADER
	ProdLog_Write(__FILE__,__LINE__,LOGL_ProgDebug,QString("Saving configuration file %1").arg(File.filePath()));
#endif

	QFile hFile(File.filePath());
	if (!hFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
#ifdef LOGFILE_HEADER
		ProdLog_Write(__FILE__,__LINE__,LOGL_ProgFailure,QString("[all,,disk error]Cannot create configuration file %1 (error %2)")
					  .arg(File.filePath()).arg(hFile.errorString()));
#endif
		return;
	}
	{
		QTextStream Stream(&hFile);
		bool FirstSection=true;
		for (SectionListType::iterator s=Sections.begin() ; s!=Sections.end() ; ++s)
		{
			SectionObject *SObj=(*s);

			if (FirstSection)
				FirstSection=false;
			else
				Stream << '\n';
			if (SObj->Comment.size()>0)
			{
				for (int i=0 ; i<SObj->Comment.size() ; i++)
					Stream << SObj->Comment.at(i) << '\n';
			}
			if (SObj->Inactive) Stream << ';';
			Stream << "[" << SObj->Name << "]";
			if (!SObj->EndOfLineComment.isEmpty()) Stream << SObj->EndOfLineComment;
			Stream << '\n';

			for (EntryListType::iterator e=SObj->Entries.begin() ; e!=SObj->Entries.end() ; ++e)
			{
				EntryObject *EObj=(*e);
				if (EObj->Comment.size()>0)
				{
					for (int i=0 ; i<EObj->Comment.size() ; i++)
						Stream << EObj->Comment.at(i) << '\n';
				}
				if (EObj->Inactive) Stream << ';';
				Stream << EObj->Name << "=" << EObj->Value << '\n';
				EObj->New=false;
			}

			SObj->New=false;
			Stream.flush();
		}
	}
	if (!hFile.flush())
	{
#ifdef LOGFILE_HEADER
		ProdLog_Write(__FILE__,__LINE__,LOGL_ProgFailure,QString("[all,,disk error]Write error in configuration file %1 (error %2)")
					  .arg(File.filePath()).arg(hFile.errorString()));
#endif
	}
	hFile.close();
}

/*=============================================================================*/
/*!
  Prepare the file for a later write on disk.
 */
/*=============================================================================*/
void ConfigObject::FileObject::Modified()
{
	if (!Unsaved)
	{
		Unsaved=true;
		if (Parent) Parent->StartSaveTimer();
	}
}

/*=============================================================================*/
/*!
  Timer called when it is time to save the file.
 */
/*=============================================================================*/
void ConfigObject::timerEvent(QTimerEvent *Event)
{
	Event=Event;//compiler pacifier
	StopSaveTimer();
	if (ConfigFile.Unsaved)
	{
		ConfigFile.SaveFile();
	}
}

/*=============================================================================*/
/*!
  Start the timer to save the file.
 */
/*=============================================================================*/
void ConfigObject::StartSaveTimer()
{
	QMutexLocker Lock(&SaveTimerCs);
	if (SaveTimer==0)
		SaveTimer=startTimer(5000);
}

/*=============================================================================*/
/*!
  Stop the timer to save the file.
 */
/*=============================================================================*/
void ConfigObject::StopSaveTimer()
{
	QMutexLocker Lock(&SaveTimerCs);
	if (SaveTimer)
	{
		killTimer(SaveTimer);
		SaveTimer=0;
	}
}

/*=============================================================================*/
/*!
  Initialize the object.

  \param Parent The configuration object holding this file.
 */
/*=============================================================================*/
ConfigObject::FileObject::FileObject(ConfigObject *Parent) :
	Parent(Parent),
	Unsaved(false)
{
	LastLoadTime.invalidate();
}

/*=============================================================================*/
/*!
  Delete the object including its sections.
 */
/*=============================================================================*/
ConfigObject::FileObject::~FileObject()
{
	while (Sections.size()>0)
	{
		SectionObject *s=Sections.front();
		Sections.pop_front();
		delete s;
	}
}

/*=============================================================================*/
/*!
  Find the first section of the file.

  \param Section The section.

  \return A pointer to the section or NULL if it was not found.
 */
/*=============================================================================*/
ConfigObject::SectionObject *ConfigObject::FileObject::FindSection(const QString &Section)
{
	for (SectionListType::iterator s=Sections.begin() ; s!=Sections.end() ; ++s)
	{
		SectionObject *SObj=*s;
		if (!SObj->Inactive && SObj->Name==Section)
		{
			return(SObj);
		}
	}
	return(NULL);
}

/*=============================================================================*/
/*!
  Destroy the content of the section.
 */
/*=============================================================================*/
ConfigObject::SectionObject::~SectionObject()
{
	while (Entries.size()>0)
	{
		EntryObject *e=Entries.front();
		Entries.pop_front();
		delete e;
	}
}

/*=============================================================================*/
/*!
  Get the entry of the file.

  \param Section The section.
  \param Entry

  \return A pointer to the entry or NULL if it was not found.
 */
/*=============================================================================*/
ConfigObject::EntryObject *ConfigObject::SectionObject::FindEntry(const QString &Entry)
{
	for (EntryListType::iterator e=Entries.begin() ; e!=Entries.end() ; ++e)
	{
		EntryObject *EObj=*e;
		if (!EObj->Inactive && EObj->Name==Entry)
		{
			return(EObj);
		}
	}
	return(NULL);
}

/*=============================================================================*/
/*!
  Append a section to the file.

  \param Section The name of the section.

  \return A pointer to the newly created section.
 */
/*=============================================================================*/
ConfigObject::SectionObject *ConfigObject::FileObject::AppendSection(const QString &Section)
{
	SectionObject *NewSection=new SectionObject;
	NewSection->Inactive=false;
	NewSection->New=true;
	NewSection->Name=Section;
	Sections.push_back(NewSection);
	return(NewSection);
}

/*=============================================================================*/
/*!
  Append an entry to a section.

  \param Section The name of the section.
  \param Entry The entry to add.

  \return A pointer to the newly created entry.
 */
/*=============================================================================*/
ConfigObject::EntryObject *ConfigObject::SectionObject::AppendEntry(const QString &Entry)
{
	EntryObject *NewEntry=new EntryObject;
	NewEntry->Section=this;
	NewEntry->New=true;
	NewEntry->Inactive=false;
	NewEntry->Name=Entry;
	Entries.push_back(NewEntry);
	return(NewEntry);
}

/*=============================================================================*/
/*!
  Append an entry to a section.

  \param Section The name of the section.
  \param Entry The entry to add.
  \param Default The default value to assign to the entry if none exists.
  \param Comment A comment to write before this parameter if it is created.

  \return A pointer to the newly created entry.
 */
/*=============================================================================*/
ConfigObject::EntryObject *ConfigObject::FileObject::GetOrCreate(const QString &Section, const QString &Entry,const QString &Default)
{
	SectionObject *LastSObj=NULL;
	EntryObject *EObj=NULL;
	for (SectionListType::iterator s=Sections.begin() ; s!=Sections.end() ; ++s)
	{
		SectionObject *SObj=*s;
		if (!SObj->Inactive && SObj->Name==Section)
		{
			LastSObj=SObj;
			EObj=SObj->FindEntry(Entry);
			if (EObj) return(EObj);
		}
	}

	if (!LastSObj)
	{
		LastSObj=AppendSection(Section);
		Modified();
	}
	EObj=LastSObj->AppendEntry(Entry);
	EObj->Value=Default;
	EObj->New=true;
	Modified();
	return(EObj);
}

/*=============================================================================*/
/*!
  Find an entry in the file.

  \param Section The name of the section.
  \param Entry The entry to find.

  \return A pointer to the existing entry or NULL if it doesn't exists.
 */
/*=============================================================================*/
ConfigObject::EntryObject *ConfigObject::FileObject::GetNoCreate(const QString &Section, const QString &Entry)
{
	for (SectionListType::iterator s=Sections.begin() ; s!=Sections.end() ; ++s)
	{
		SectionObject *SObj=*s;
		if (!SObj->Inactive && SObj->Name==Section)
		{
			EntryObject *EObj=SObj->FindEntry(Entry);
			if (EObj) return(EObj);
		}
	}

	return(NULL);
}

/*=============================================================================*/
/*!
  Store a value in the file.
 */
/*=============================================================================*/
void ConfigObject::FileObject::Store(const QString &Section, const QString &Entry, const QString &Value)
{
	SectionObject *LastSObj=NULL;
	EntryObject *EObj=NULL;
	for (SectionListType::iterator s=Sections.begin() ; s!=Sections.end() ; ++s)
	{
		SectionObject *SObj=*s;
		if (SObj->Name==Section)
		{
			LastSObj=SObj;
			EObj=SObj->FindEntry(Entry);
			if (EObj)
			{
				if (EObj->Value!=Value)
				{
					EObj->Value=Value;
					Modified();
				}
				return;
			}
		}
	}

	if (!LastSObj)
	{
		LastSObj=AppendSection(Section);
	}
	EObj=LastSObj->AppendEntry(Entry);
	EObj->Value=Value;
	Modified();
}

/*=============================================================================*/
/*!
  Format a text to be a comment of a section or entry.

  \param Text The text to format.
  \param Width The maximum width of each line.

  \return The comment.
 */
/*=============================================================================*/
QStringList ConfigObject::FormatComment(const QString &Text,int Width)
{
	QStringList Result;
	int Start=-1;
	while (Start<Text.length())
	{
		Start++;
		int Eol=Text.indexOf('\n',Start);
		if (Eol<Start) Eol=Text.length();
		if (Eol-Start<=Width)
		{
			Result.append("# "+Text.mid(Start,Eol-Start));
		}
		else
		{
			QString Line=Text.mid(Start,Eol-Start);
			while (Line.length()>Width)
			{
				int j=Width;
				while (j>0 && Line.at(j)!=' ') j--;
				if (j<=0) j=Width;
				Result.append("# "+Line.left(j));
				Line=Line.mid(j+1);
			}
			Result.append("# "+Line);
		}
		Start=Eol;
	}
	return(Result);
}

/*=============================================================================*/
/*!
  Get the name of the pre configuration file where we read the real name of the
  configuration file.
  
  \return A pointer to the pre configuration file or an empty string if the
          SetConfigFile() function was never used.
  
  \date 2009-02-09
 */
/*=============================================================================*/
QString ConfigObject::Config_GetPreFile()
{
	return(PreConfigFile.absoluteFilePath());
}

/*=============================================================================*/
/*!
  Get the name of the configuration file where we read our working parameters.
  
  \return A pointer to the default configuration file or an empty string if the
          SetConfigFile() function was never used.
  
  \date 2007-05-18
 */
/*=============================================================================*/
QString ConfigObject::Config_GetFile()
{
	return(ConfigFile.File.canonicalFilePath());
}

/*=============================================================================*/
/*!
  Get the path to the configuration file. It may be used to find some files
  provided without path.
  
  \return A pointer to the base path of the files or NULL if the SetConfigFile()
          function was never used.
  
  \date 2007-12-14
 */
/*=============================================================================*/
QString ConfigObject::Config_GetBasePath()
{
	return(BasePath.absolutePath());
}

/*=============================================================================*/
/*!
  Read one item in the configuration file but do not write the default value to
  the configuration file if it is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default string to write in the configuration file and to return to the caller
         if the item could not be found. If the pointer is NULL, the value in 
         \a Value is unchanged.
  \param Value Buffer to store the string from the item.
  \param ValueLen Length of the buffer Value.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \retval 1 The item was read.
  \retval 0 It has been added to the configuration file.
  \retval -1 No configuration file name or requested value too small.
 
  \author F. Marchal
  \date 2000-05-11
 */
/*=============================================================================*/
QString ConfigObject::Config_GetStringNoWrite(const QString &Section,const QString &Item,const QString Default)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetNoCreate(Section,Item);
	QString Val=(EObj) ? EObj->Value : Default;
	AccessLock.unlock();

	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file but do not write the default value to
  the configuration file if it is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2007-06-01
 */
/*=============================================================================*/
int ConfigObject::Config_GetIntNoWrite(const QString &Section,const QString &Item,int Default)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetNoCreate(Section,Item);
	if (!EObj)
	{
		AccessLock.unlock();
		return(Default);
	}

	bool Ok=false;
	int Val=EObj->Value.toInt(&Ok);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file but do not write the default value to
  the configuration file if it is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2007-07-18
 */
/*=============================================================================*/
unsigned int ConfigObject::Config_GetHexNoWrite(const QString &Section,const QString &Item,unsigned int Default)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetNoCreate(Section,Item);
	if (!EObj)
	{
		AccessLock.unlock();
		return(Default);
	}

	bool Ok=false;
	unsigned int Val=EObj->Value.toUInt(&Ok,16);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file but do not write the default value to
  the configuration file if it is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2007-06-01
 */
/*=============================================================================*/
unsigned int ConfigObject::Config_GetUIntNoWrite(const QString &Section,const QString &Item,unsigned int Default)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetNoCreate(Section,Item);
	if (!EObj)
	{
		AccessLock.unlock();
		return(Default);
	}

	bool Ok=false;
	unsigned int Val=EObj->Value.toUInt(&Ok);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file but do not write the default value to
  the configuration file if it is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2007-06-01
 */
/*=============================================================================*/
double ConfigObject::Config_GetDoubleNoWrite(const QString &Section,const QString &Item,double Default)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetNoCreate(Section,Item);
	if (!EObj)
	{
		AccessLock.unlock();
		return(Default);
	}

	bool Ok=false;
	double Val=EObj->Value.toDouble(&Ok);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Get a whole section from the configuration file.
  
  \param Section The section to read.
  \param Default The default value to return if none is found in the file. Don't forget
         to end each line of the section with a \\0 including the last line of the section.
         In fact, your default string must always end with a \\0 !
  \param Value The buffer to store the value.
  \param ValueLen The size of the buffer.
  \param ConfigFile The name of the config file. If it is NULL, the default file
         name is used.

  \retval 3 No error and the section was read entirely.
  \retval 2 No error but only part of the section was read.
  \retval 1 Section is empty but the default value was copied entirely.
  \retval 0 Section is empty and default value is truncated.
  \retval -1 Buffer too small.

  \date 2008-11-28
 */
/*=============================================================================*/
QStringList ConfigObject::Config_GetSectionNoWrite(const QString &Section)
{
	Load();

	AccessLock.lockForRead();
	QStringList List;
	for (SectionListType::const_iterator s=ConfigFile.Sections.begin() ; s!=ConfigFile.Sections.end() ; ++s)
	{
		const SectionObject *SObj=*s;
		if (SObj->Name==Section)
		{
			for (EntryListType::const_iterator e=SObj->Entries.begin() ; e!=SObj->Entries.end() ; ++e)
				List.append((*e)->Name);
		}
	}
	AccessLock.unlock();

	return(List);
}






/*=============================================================================*/
/*!
  Read one item in the configuration file and add a default value to the configuration
  file if the item is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default string to write in the configuration file and to return to the caller
         if the item could not be found.
  \param Value Buffer to store the string from the item.
  \param ValueLen Length of the buffer Value.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \retval 1 The item was read.
  \retval 0 It has been added to the configuration file if \a Default is not NULL.
  \retval -1 No configuration file name or requested valud too small.
 
  \author F. Marchal
  \date 2000-05-11
 */
/*=============================================================================*/
QString ConfigObject::Config_GetString(const QString &Section,const QString &Item,const QString &Default,const QString &Comment)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetOrCreate(Section,Item,Default);
	if (EObj->New && !Comment.isEmpty()) EObj->Comment=FormatComment(Comment);
	QString Val=EObj->Value;
	AccessLock.unlock();
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file and add a default value to the configuration
  file if the item is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2004-05-19
 */
/*=============================================================================*/
int ConfigObject::Config_GetInt(const QString &Section,const QString &Item,int Default, const QString &Comment)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetOrCreate(Section,Item,QString::number(Default));
	if (EObj->New && !Comment.isEmpty()) EObj->Comment=FormatComment(Comment);

	bool Ok=false;
	int Val=EObj->Value.toInt(&Ok);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file and add a default value to the configuration
  file if the item is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2007-07-18
 */
/*=============================================================================*/
unsigned int ConfigObject::Config_GetHex(const QString &Section,const QString &Item,unsigned int Default, const QString &Comment)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetOrCreate(Section,Item,QString::number(Default,16));
	if (EObj->New && !Comment.isEmpty()) EObj->Comment=FormatComment(Comment);

	bool Ok=false;
	unsigned int Val=EObj->Value.toUInt(&Ok,16);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file and add a default value to the configuration
  file if the item is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2004-06-01
 */
/*=============================================================================*/
unsigned int ConfigObject::Config_GetUInt(const QString &Section,const QString &Item,unsigned int Default, const QString &Comment)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetOrCreate(Section,Item,QString::number(Default));
	if (EObj->New && !Comment.isEmpty()) EObj->Comment=FormatComment(Comment);

	bool Ok=false;
	unsigned int Val=EObj->Value.toUInt(&Ok);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Read one item in the configuration file and add a default value to the configuration
  file if the item is missing.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param Default Default value to write in the configuration file and to return to the caller
         if the item could not be found.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return The value read in the configuration file or the default value if either the value
          in the configuration file doesn't exist or is not a number.
 
  \author F. Marchal
  \date 2004-05-19
 */
/*=============================================================================*/
double ConfigObject::Config_GetDouble(const QString &Section,const QString &Item,double Default, const QString &Comment)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetOrCreate(Section,Item,QString::number(Default));
	if (EObj->New && !Comment.isEmpty()) EObj->Comment=FormatComment(Comment);

	bool Ok=false;
	double Val=EObj->Value.toDouble(&Ok);
	AccessLock.unlock();
	if (!Ok)
	{
		qWarning() << "Invalid value for " << Section << "/" << Item << "in configuration file" << ConfigFile.File.fileName();
		return(Default);
	}
	return(Val);
}

/*=============================================================================*/
/*!
  Get a full file name including the absolute path from a configuration file. The
  configuration file may provide a relative path and it will be expanded relative
  to the application root directory.
  
  \param Section The section with the file name.
  \param Item The item to read.
  \param Default The default value if none exists in the config file. If it is
         NULL, no value is added to the configuration file
  \param Value A buffer to store the file name.
  \param ValueLen The length of the buffer.
  \param ConfigFile The name of the config file. If it is NULL, the default file
         name is used.

  \retval 1 Value read from the configuration file.
  \retval 0 Value not found but it may have been added to the configuration file if
            one was found and the default value is not NULL.
  \retval -1 Could not read the configuration file.
  \retval -2 No file name returned.
  
  \date 2007-05-18
 */
/*=============================================================================*/
QFileInfo ConfigObject::Config_GetFileName(const QString &Section,const QString &Item,const QString &Default, const QString &Comment)
{
	Load();

	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetOrCreate(Section,Item,Default);
	if (EObj->New && !Comment.isEmpty()) EObj->Comment=FormatComment(Comment);

	QFileInfo FileName;
	if (!EObj->Value.isEmpty())
		FileName.setFile(BasePath,EObj->Value);
	AccessLock.unlock();
	return(FileName);
}

/*=============================================================================*/
/*!
  Get a whole section from the configuration file. If the section is empty and
  a default value is provided, it is written to the configuration file.
  
  \param Section The section to read.
  \param Default The default value to return if none is found in the file. Don't forget
         to end each line of the section with a \\0 including the last line of the section.
         In fact, your default string must always end with a \\0 !
  \param Value The buffer to store the value.
  \param ValueLen The size of the buffer.
  \param ConfigFile The name of the config file. If it is NULL, the default file
         name is used.

  \retval 1 No error and the section was read entirely.
  \retval 0 No error but only part of the section was read.
  \retval -1 Buffer too small.

  \date 2008-11-28
 */
/*=============================================================================*/
QStringList ConfigObject::Config_GetSection(const QString Section)
{
	Load();

	AccessLock.lockForRead();
	QStringList List;
	for (SectionListType::const_iterator s=ConfigFile.Sections.begin() ; s!=ConfigFile.Sections.end() ; ++s)
	{
		const SectionObject *SObj=*s;
		if (SObj->Name==Section)
		{
			for (EntryListType::const_iterator e=SObj->Entries.begin() ; e!=SObj->Entries.end() ; ++e)
				List.append((*e)->Name);
		}
	}
	AccessLock.unlock();

	/*
	  This function used to write a default value if none was found but it is unwieldy.
	  Therefore, the function is now identical to Config_GetSectionNoWrite().
	*/
	return(List);
}

/*=============================================================================*/
/*!
  Check if the required item is defined in the configuration file and is not empty.
 
  \param Section Section of the configuration file to get the item.
  \param Item Item to fetch in the configuration file.
  \param ConfigFile Configuration file were the item is to be retrieved.
 
  \return \c True if a data exists in the ini file or \c false if it is empty.
 
  \author F. Marchal
  \date 2010-05-03
 */
/*=============================================================================*/
bool ConfigObject::Config_IsEmpty(const QString &Section,const QString &Item)
{
	Load();
	AccessLock.lockForRead();
	EntryObject *EObj=ConfigFile.GetNoCreate(Section,Item);
	AccessLock.unlock();
	return(EObj!=NULL);
}





/*=============================================================================*/
/*!
  Write a parameter to the configuration file.

  \param Section Section of the configuration file to set the item.
  \param Item Item to set in the configuration file.
  \param Value Value to write in the configuration.
  \param ConfigFile Configuration file were the item is to be written.

  \date 2007-05-18
 */
/*=============================================================================*/
void ConfigObject::Config_WriteString(const QString &Section,const QString &Item,const QString &Value)
{
	Load();
	AccessLock.lockForWrite();
	ConfigFile.Store(Section,Item,Value);
	AccessLock.unlock();
}

/*=============================================================================*/
/*!
  Write a parameter to the configuration file.

  \param Section Section of the configuration file to set the item.
  \param Item Item to set in the configuration file.
  \param Value Value to write in the configuration.
  \param ConfigFile Configuration file were the item is to be written.

  \date 2005-12-14
 */
/*=============================================================================*/
void ConfigObject::Config_WriteInt(const QString &Section,const QString &Item,int Value)
{
	Load();
	AccessLock.lockForWrite();
	ConfigFile.Store(Section,Item,QString::number(Value));
	AccessLock.unlock();
}

/*=============================================================================*/
/*!
  Write a parameter to the configuration file.
  
  \date 2007-07-18
 */
/*=============================================================================*/
void ConfigObject::Config_WriteHex(const QString &Section,const QString &Item,unsigned int Value)
{
	Load();
	AccessLock.lockForWrite();
	ConfigFile.Store(Section,Item,QString::number(Value,16));
	AccessLock.unlock();
}

/*=============================================================================*/
/*!
  Write a parameter to the configuration file.
  
  \date 2006-10-23
 */
/*=============================================================================*/
void ConfigObject::Config_WriteUInt(const QString &Section,const QString &Item, unsigned int Value)
{
	Load();
	AccessLock.lockForWrite();
	ConfigFile.Store(Section,Item,QString::number(Value));
	AccessLock.unlock();
}

/*=============================================================================*/
/*!
  Write a parameter to the configuration file.
  
  \date 2005-10-26
 */
/*=============================================================================*/
void ConfigObject::Config_WriteDouble(const QString &Section,const QString &Item, double Value)
{
	Load();
	AccessLock.lockForWrite();
	ConfigFile.Store(Section,Item,QString::number(Value));
	AccessLock.unlock();
}

/*=============================================================================*/
/*!
  Write a file name in the configuration file. If the file name can be expressed
  as a relative path from the main configuration file, it will be converted to 
  a relative path.
  
  \param Section The section with the file name.
  \param Item The item to write.
  \param Value The file name to store. It must be less than MAX_PATHNAME_LEN bytes
         long.
  \param ConfigFile The name of the config file. If it is NULL, the default file
         name is used.

  \date 2007-05-31
 */
/*=============================================================================*/
void ConfigObject::Config_WriteFileName(const QString &Section,const QString &Item, const QFileInfo &Value)
{
	Load();
	AccessLock.lockForWrite();
	ConfigFile.Store(Section,Item,BasePath.relativeFilePath(Value.filePath()));
	AccessLock.unlock();
}

/*==============================================================================*/
/*!
  Get the name of the program.
  
  \param ProgName The buffer to store the file name.
  \param Size The size of the buffer.
  
  \retval 0 No error.
  \retval -1 Cannot get the program name.
  
  \date 2007-07-25
 */
/*==============================================================================*/
QString ConfigObject::Config_GetProgName()
{
	QString Name=QCoreApplication::applicationFilePath();
	QFileInfo FName(Name);
	return(FName.completeBaseName());
}

#if 0
/*=============================================================================*/
/*!
  Get the string and the description of the status code returned by a function
  of this module.
  
  \param Status The status code.
  \param Description A pointer to store the long description of the status. The
         string is static and doesn't need to be freed and may not be modified.
         The pointer may be NULL if the long description is not needed.
         
  \return A string with the short name of the status.
  
  \date 2008-02-14
 */
/*=============================================================================*/
QString Config_GetStatusString(enum Config_StatusCode Status,QString &Description)
{
	#define STATUS_STRING(id) id,#id
	struct StatusStruct
	{
		enum Config_StatusCode Status;
		const char *Name;
		const QString Description;
	};
	static const struct StatusStruct StatusList[]=
		{
			{STATUS_STRING(ConfigStatus_NoError),QObject::tr("No error")},
			{STATUS_STRING(ConfigStatus_InvalidPreconfigFileName),QObject::tr("The name of the preconfiguration file is invalid")},
			{STATUS_STRING(ConfigStatus_InvalidConfigFileName),QObject::tr("The configuration file name cannot be determined")},
			{STATUS_STRING(ConfigStatus_InvalidConfigFileNamePreConfig),QObject::tr("The configuration file name cannot be determined (taken from the preconfig section)")},
			{STATUS_STRING(ConfigStatus_InvalidConfigFileNameProgram),QObject::tr("The configuration file name cannot be determined (taken from the program name section)")},
			{STATUS_STRING(ConfigStatus_InvalidConfigFileNameDefault),QObject::tr("The configuration file name cannot be determined (taken from the default section)")},
			{STATUS_STRING(ConfigStatus_NoConfigFile),QObject::tr("Configuration file not found. To initialize it with default values, create an empty file")},
			{STATUS_STRING(ConfigStatus_NoConfigFilePreConfig),QObject::tr("Configuration file not found. To initialize it with default values, create an empty"
			                                                      " file with the name found in the preconfig section")},
			{STATUS_STRING(ConfigStatus_NoConfigFileProgram),QObject::tr("Configuration file not found. To initialize it with default values, create an empty"
			                                                      " file with the name found in the section named after the program")},
			{STATUS_STRING(ConfigStatus_NoConfigFileDefault),QObject::tr("Configuration file not found. To initialize it with default values, create an empty"
			                                                      " file with the name found in the default section")},
		};
	
	for (unsigned int i=0 ; i<sizeof(StatusList)/sizeof(StatusList[0]) ; i++)
		if (StatusList[i].Status==Status)
		{
			Description=QString(StatusList[i].Description);
			return(StatusList[i].Name);
		}
	Description=QObject::tr("Unknown status code");
	return(QString(QObject::tr("???")));
}
#endif
