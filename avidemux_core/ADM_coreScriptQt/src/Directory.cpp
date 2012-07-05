#include <QtScript/QScriptEngine>

#include "Directory.h"
#include "FileInformation.h"

namespace ADM_qtScript
{
	static const QDir::Filter qtFilter[] =
	{
		QDir::Dirs, QDir::Files, QDir::Drives, QDir::NoSymLinks, QDir::Readable, QDir::Writable,
		QDir::Executable, QDir::Modified, QDir::Hidden, QDir::System, QDir::CaseSensitive,
		QDir::NoDotAndDotDot, QDir::NoFilter
	};

	static const Directory::Filter myFilter[] =
	{
		Directory::Directories, Directory::Files, Directory::Drives, Directory::NoSymLinks,
		Directory::Readable, Directory::Writable, Directory::Executable, Directory::Modified,
		Directory::Hidden, Directory::System, Directory::CaseSensitive, Directory::NoDotAndDotDot,
		Directory::NoFilter
	};

	static const QDir::SortFlag qtSort[] =
	{
		QDir::Name, QDir::Time, QDir::Size, QDir::Unsorted, QDir::DirsFirst, QDir::Reversed,
		QDir::IgnoreCase, QDir::DirsLast, QDir::LocaleAware, QDir::Type, QDir::NoSort
	};

	static const Directory::Sort mySort[] =
	{
		Directory::Name, Directory::Time, Directory::Size, Directory::Unsorted, Directory::DirectoriesFirst,
		Directory::Reversed, Directory::IgnoreCase, Directory::DirectoriesLast, Directory::LocaleAware,
		Directory::Type, Directory::NoSort
	};

	Directory::Directory(QString path)
	{
		_dir = QDir(path);
	}

	Directory::Directory(
		QString path, QString nameFilter, Sort sort, Filter filter)
	{
		_dir = QDir(path, nameFilter, this->getQtSortFlags(sort), this->getQtFilters(filter));
	}

	QScriptValue Directory::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			if (context->argumentCount() == 0)
			{
				return engine->newQObject(new Directory(), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 1)
			{
				return engine->newQObject(
						   new Directory(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 2)
			{
				return engine->newQObject(
						   new Directory(
							   context->argument(0).toString(), context->argument(1).toString()),
						   QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 3 && context->argument(2).isNumber())
			{
				return engine->newQObject(
						   new Directory(
							   context->argument(0).toString(), context->argument(1).toString(),
							   (Directory::Sort)context->argument(2).toNumber()),
						   QScriptEngine::ScriptOwnership);
			}
			else if (context->argumentCount() == 4 && context->argument(2).isNumber() &&
					 context->argument(3).isNumber())
			{
				return engine->newQObject(
						   new Directory(
							   context->argument(0).toString(), context->argument(1).toString(),
							   (Directory::Sort)context->argument(2).toNumber(),
							   (Directory::Filter)context->argument(3).toNumber()),
						   QScriptEngine::ScriptOwnership);
			}
			else
			{
				return context->throwError("Invalid arguments passed to constructor");
			}
		}

		return engine->undefinedValue();
	}

	Directory::Filters Directory::getMyFilters(QDir::Filters qtFilters)
	{
		Directory::Filters myFilters;

		for (unsigned int i = 0; i < (sizeof(qtFilter) / sizeof(QDir::Filter)); i++)
		{
			if (qtFilters.testFlag(qtFilter[i]))
			{
				myFilters |= myFilter[i];
			}
		}

		return myFilters;
	}

	Directory::SortFlags Directory::getMySortFlags(QDir::SortFlags qtSortFlags)
	{
		Directory::SortFlags mySortFlags;

		for (unsigned int i = 0; i < (sizeof(qtSort) / sizeof(QDir::SortFlag)); i++)
		{
			if (qtSortFlags.testFlag(qtSort[i]))
			{
				mySortFlags |= mySort[i];
			}
		}

		return mySortFlags;
	}

	QDir::Filters Directory::getQtFilters(Directory::Filters myFilters)
	{
		QDir::Filters qtFilters;

		for (unsigned int i = 0; i < (sizeof(myFilter) / sizeof(Directory::Filter)); i++)
		{
			if (myFilters.testFlag(myFilter[i]))
			{
				qtFilters |= qtFilter[i];
			}
		}

		return qtFilters;
	}

	QDir::SortFlags Directory::getQtSortFlags(Directory::SortFlags mySortFlags)
	{
		QDir::SortFlags qtSortFlags;

		for (unsigned int i = 0; i < (sizeof(mySort) / sizeof(Directory::Sort)); i++)
		{
			if (mySortFlags.testFlag(mySort[i]))
			{
				qtSortFlags |= qtSort[i];
			}
		}

		return qtSortFlags;
	}

	QString Directory::getPath()
	{
		return _dir.path();
	}

	void Directory::setPath(const QString &path)
	{
		_dir.setPath(path);
	}

	QString Directory::getAbsolutePath()
	{
		return _dir.absolutePath();
	}

	QString Directory::getCanonicalPath()
	{
		return _dir.canonicalPath();
	}

	QString Directory::getDirectoryName()
	{
		return _dir.dirName();
	}

	QString Directory::filePath(QString fileName)
	{
		return _dir.filePath(fileName);
	}

	QString Directory::absoluteFilePath(QString fileName)
	{
		return _dir.absoluteFilePath(fileName);
	}

	QString Directory::relativeFilePath(QString fileName)
	{
		return _dir.relativeFilePath(fileName);
	}

	QString Directory::toNativeSeparators(QString pathName)
	{
		return QDir::toNativeSeparators(pathName);
	}

	QString Directory::fromNativeSeparators(QString pathName)
	{
		return QDir::fromNativeSeparators(pathName);
	}

	bool Directory::changeDirectory(QString dirName)
	{
		return _dir.cd(dirName);
	}

	bool Directory::changeDirectoryUp()
	{
		return _dir.cdUp();
	}

	QScriptValue Directory::getNameFilters()
	{
		QStringList filters = _dir.nameFilters();
		QScriptValue array = this->engine()->newArray(filters.length());

		for (int filterIndex = 0; filterIndex < filters.length(); filterIndex++)
		{
			array.setProperty(filterIndex, filters[filterIndex]);
		}

		return array;
	}

	void Directory::setNameFilters(QScriptValue nameFilters)
	{
		if (nameFilters.isArray())
		{
			QStringList list;

			qScriptValueToSequence(nameFilters, list);
			_dir.setNameFilters(list);
		}
	}

	Directory::Filter Directory::getFilter()
	{
		return (Filter)(int)this->getMyFilters(_dir.filter());
	}

	void Directory::setFilter(Filter filter)
	{
		_dir.setFilter(this->getQtFilters(filter));
	}

	Directory::Sort Directory::getSorting()
	{
		return (Sort)(int)this->getMySortFlags(_dir.sorting());
	}

	void Directory::setSorting(Sort sort)
	{
		_dir.setSorting(this->getQtSortFlags(sort));
	}

	uint Directory::getCount()
	{
		return _dir.count();
	}

	QScriptValue Directory::entryList(Filter filters, Sort sort)
	{
		QStringList list = _dir.entryList(this->getQtFilters(filters), this->getQtSortFlags(sort));
		QScriptValue array = this->engine()->newArray(list.length());

		for (int listIndex = 0; listIndex < list.length(); listIndex++)
		{
			array.setProperty(listIndex, list[listIndex]);
		}

		return array;
	}

	QScriptValue Directory::entryList(QScriptValue nameFilters, Filter filters, Sort sort)
	{
		QStringList filterList;

		if (nameFilters.isArray())
		{
			qScriptValueToSequence(nameFilters, filterList);
		}
		else
		{
			return this->context()->throwError("nameFilters is an invalid type");
		}

		QStringList list = _dir.entryList(
							   filterList, this->getQtFilters(filters), this->getQtSortFlags(sort));
		QScriptValue array = this->engine()->newArray(list.length());

		for (int listIndex = 0; listIndex < list.length(); listIndex++)
		{
			array.setProperty(listIndex, list[listIndex]);
		}

		return array;
	}

	QScriptValue Directory::entryInfoList(Filter filters, Sort sort)
	{
		QFileInfoList list = _dir.entryInfoList(this->getQtFilters(filters), this->getQtSortFlags(sort));
		QScriptValue array = this->engine()->newArray(list.length());

		for (int listIndex = 0; listIndex < list.length(); listIndex++)
		{
			array.setProperty(listIndex, this->engine()->newQObject(
				new FileInformation(list[listIndex]), QScriptEngine::ScriptOwnership));
		}

		return array;
	}

	QScriptValue Directory::entryInfoList(QScriptValue nameFilters, Filter filters, Sort sort)
	{
		QStringList filterList;

		if (nameFilters.isArray())
		{
			qScriptValueToSequence(nameFilters, filterList);
		}
		else
		{
			return this->context()->throwError("nameFilters is an invalid type");
		}

		QFileInfoList list = _dir.entryInfoList(filterList, this->getQtFilters(filters), this->getQtSortFlags(sort));
		QScriptValue array = this->engine()->newArray(list.length());

		for (int listIndex = 0; listIndex < list.length(); listIndex++)
		{
			array.setProperty(listIndex, this->engine()->newQObject(
				new FileInformation(list[listIndex]), QScriptEngine::ScriptOwnership));
		}

		return array;
	}

	bool Directory::makeDirectory(QString dirName)
	{
		return _dir.mkdir(dirName);
	}

	bool Directory::removeDirectory(QString dirName)
	{
		return _dir.rmdir(dirName);
	}

	bool Directory::makePath(QString dirPath)
	{
		return _dir.mkpath(dirPath);
	}

	bool Directory::removePath(QString dirPath)
	{
		return _dir.rmpath(dirPath);
	}

	bool Directory::isReadable()
	{
		return _dir.isReadable();
	}

	bool Directory::exists()
	{
		return _dir.exists();
	}

	bool Directory::isRoot()
	{
		return _dir.isRoot();
	}

	bool Directory::isRelative()
	{
		return _dir.isRelative();
	}

	bool Directory::isAbsolute()
	{
		return _dir.isAbsolute();
	}

	bool Directory::makeAbsolute()
	{
		return _dir.makeAbsolute();
	}

	bool Directory::remove(QString fileName)
	{
		return _dir.remove(fileName);
	}

	bool Directory::rename(QString oldName, QString newName)
	{
		return _dir.rename(oldName, newName);
	}

	bool Directory::exists(QString name)
	{
		return _dir.exists(name);
	}

	QString Directory::getSeparator()
	{
		return _dir.separator();
	}

	QString Directory::getCurrentPath()
	{
		return QDir::currentPath();
	}

	bool Directory::setCurrentPath(QString path)
	{
		return QDir::setCurrent(path);
	}

	QString Directory::getHomePath()
	{
		return QDir::homePath();
	}

	QString Directory::getRootPath()
	{
		return QDir::rootPath();
	}

	QString Directory::getTempPath()
	{
		return QDir::tempPath();
	}

	QScriptValue Directory::getDrives()
	{
		QFileInfoList list = _dir.drives();
		QScriptValue array = this->engine()->newArray(list.length());

		for (int listIndex = 0; listIndex < list.length(); listIndex++)
		{
			array.setProperty(listIndex, this->engine()->newQObject(
				new FileInformation(list[listIndex]), QScriptEngine::ScriptOwnership));
		}

		return array;
	}

	bool Directory::match(QScriptValue filters, QString fileName)
	{
		QStringList filterList;

		if (filters.isArray())
		{
			qScriptValueToSequence(filters, filterList);
		}
		else
		{
			this->context()->throwError("filters is an invalid type");
			return false;
		}

		return QDir::match(filterList, fileName);
	}

	bool Directory::match(QString filter, QString fileName)
	{
		return QDir::match(filter, fileName);
	}

	QString Directory::cleanPath(QString path)
	{
		return _dir.cleanPath(path);
	}

	void Directory::refresh()
	{
		_dir.refresh();
	}
}
