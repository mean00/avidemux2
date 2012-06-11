#ifndef ADM_qtScript_Directory
#define ADM_qtScript_Directory

#include <QtCore/QDir>
#include <QtScript/QScriptable>
#include <QtScript/QScriptValue>

namespace ADM_qtScript
{
	/** \brief The Directory %class provides access to directory structures and their contents.
	  *
	  * A Directory object is used to manipulate path names, access information regarding paths and files,
	  * and manipulate the underlying file system.
	  *
	  * The Directory class uses "/" as a universal directory separator in the same way that "/" is used
	  * as a path separator in URLs. If you always use "/" as a directory separator, the Directory class
	  * will translate your paths to conform to the underlying operating system.
	  *
	  * A Directory object can point to a file using either a relative or an absolute path. Absolute
	  * paths begin with the directory separator (optionally preceded by a drive specification under
	  * Windows). Relative file names begin with a directory name or a file name and specify a path
	  * relative to the current directory.
	  */
	class Directory : public QObject, protected QScriptable
	{
		Q_OBJECT
		Q_ENUMS(Filter Sort)

	public:
		/** \brief Describes the filtering options available to Directory; e.g. for entryList() and entryInfoList().
		 *
		 * The filter value is specified by combining values from the following list using the bitwise OR operator:
		 */
		enum Filter
		{
			Directories = 0x001, /**< List directories that match the filters. */
			Files       = 0x002, /**< List files. */
			Drives      = 0x004, /**< List disk drives (ignored under Unix). */
			NoSymLinks  = 0x008, /**< Do not list symbolic links (ignored by operating systems that don't support symbolic links). */
			AllEntries  = Directories | Files | Drives, /**< List directories, files, drives and symlinks (this does not list broken symlinks unless you specify System). */
			Readable    = 0x010, /**< List files for which the application has read access. The Readable value needs to be combined with Directories or Files. */
			Writable    = 0x020, /**< List files for which the application has write access. The Writable value needs to be combined with Directories or Files. */
			Executable  = 0x040, /**< List files for which the application has execute access. The Executable value needs to be combined with Directories or Files. */
			Modified    = 0x080, /**< Only list files that have been modified (ignored under Unix). */
			Hidden      = 0x100, /**< List hidden files (on Unix, files starting with a "."). */
			System      = 0x200, /**< List system files (on Unix, FIFOs, sockets and device files). */
			AllDirectories = 0x400, /**< List all directories; i.e. don't apply the filters to directory names. */
			CaseSensitive = 0x800, /**< The filter should be case sensitive. */
			NoDotAndDotDot = 0x1000, /**< Do not list the special entries "." and "..". */
			NoFilter = -1 /**< List everything. */
		};
		Q_DECLARE_FLAGS(Filters, Filter)

		/** \brief Describes the sort options available to Directory, e.g. for entryList() and entryInfoList().
		 *
		 * The sort value is specified by combining values from the following list using the bitwise OR operator.
		 * You can only specify one of the first four. If you specify both DirectoriesFirst and Reversed,
		 * directories are still put first, but in reverse order; the files will be listed after the
		 * directories, again in reverse order.
		 */
		enum Sort
		{
			Name        = 0x00, /**< Sort by name. */
			Time        = 0x01, /**< Sort by time (modification time). */
			Size        = 0x02, /**< Sort by file size. */
			Type        = 0x80, /**< Sort by file type (extension). */
			Unsorted    = 0x03, /**< Do not sort. */
			DirectoriesFirst = 0x04, /**< Put the directories first, then the files. */
			Reversed    = 0x08, /**< Reverse the sort order. */
			IgnoreCase  = 0x10, /**< Sort case-insensitively. */
			DirectoriesLast = 0x20, /**< Put the files first, then the directories. */
			LocaleAware = 0x40, /**< Sort items appropriately using the current locale settings. */
			NoSort = -1 /**< No sorting. */
		};
		Q_DECLARE_FLAGS(SortFlags, Sort)

	private:
		Filters getMyFilters(QDir::Filters qtFilters);
		SortFlags getMySortFlags(QDir::SortFlags qtSortFlags);
		QDir::Filters getQtFilters(Filters myFilters);
		QDir::SortFlags getQtSortFlags(SortFlags mySortFlags);

		QString getPath();
		void setPath(const QString &path);
		QString getAbsolutePath();
		QString getCanonicalPath();
		QString getDirectoryName();
		QScriptValue getNameFilters();
		void setNameFilters(QScriptValue nameFilters);
		Filter getFilter();
		void setFilter(Filter filter);
		Sort getSorting();
		void setSorting(Sort sort);
		uint getCount();
		bool isReadable();
		bool exists();
		bool isRoot();
		bool isRelative();
		bool isAbsolute();
		QString getSeparator();
		QString getCurrentPath();
		bool setCurrentPath(QString path);
		QString getHomePath();
		QString getRootPath();
		QString getTempPath();

	public:
	    /** \cond */
	    QDir _dir;
	    /** \endcond */

		/** \brief Constructs a Directory object pointing to the given directory path.
		 * If path is empty the program's working directory, ("."), is used.
		 */
		Directory(QString /*% String %*/ path = "");

		/** \brief Constructs a Directory object with specified path, that filters its entries by name
		 * using nameFilter and by attributes using filter. It also sorts the names using sort.
		 *
		 * The default nameFilter is an empty string, which excludes nothing; the default filters
		 * is AllEntries, which also means exclude nothing. The default sort is Name | IgnoreCase,
		 * i.e. sort by name case-insensitively.
		 *
		 * If path is an empty string, Directory uses "." (the current directory). If nameFilter
		 * is an empty string, Directory uses the name filter "*" (all files).
		 *
		 * Note that path need not exist.
		 */
		Directory(
			QString /*% String %*/ path, QString /*% String %*/ nameFilter,
			Sort sort = Sort(Name | IgnoreCase), Filter filter = AllEntries);

		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);
		/** \endcond */

		/** \brief Gets or sets the path of the Directory object.
		  *
		  * The path is cleaned of redundant ".", ".." and of multiple separators. No check is made
		  * to see whether a directory with this path actually exists; but you can check for yourself
		  * using exists().
		  *
		  * The path can be either absolute or relative. Absolute paths begin with the directory
		  * separator "/" (optionally preceded by a drive specification under Windows). Relative file
		  * names begin with a directory name or a file name and specify a path relative to the current
		  * directory. An example of an absolute path is the string "/tmp/quartz", a relative path
		  * might look like "src/fatlib".
		  */
		Q_PROPERTY(QString /*% String %*/ path READ getPath WRITE setPath);

		/** \brief Returns the absolute path (a path that starts with "/" or with a drive specification),
		 * which may contain symbolic links, but never contains redundant ".", ".." or multiple separators.
		 */
		Q_PROPERTY(QString /*% String %*/ absolutePath READ getAbsolutePath);

		/** \brief Returns the canonical path, i.e. a path without symbolic links or redundant "." or ".."
		 * elements.
		 *
		 * On systems that do not have symbolic links this function will always return the same string
		 * that absolutePath returns. If the canonical path does not exist (normally due to dangling
		 * symbolic links) canonicalPath returns an empty string.
		 */
		Q_PROPERTY(QString /*% String %*/ canonicalPath READ getCanonicalPath);

		/** \brief Returns the name of the directory.
		 *
		 * A directory name isn't the same as the path, e.g. a directory with the name "mail", might have
		 * the path "/var/spool/mail". If the directory has no name (e.g. it is the root directory) an
		 * empty string is returned.
		 *
		 * No check is made to ensure that a directory with this name actually exists; but see
		 * Directory.exists.
		 * \sa exists
		 */
		Q_PROPERTY(QString /*% String %*/ directoryName READ getDirectoryName);

		/** \brief Gets or sets the name filters used by entryList() and entryInfoList().
		 *
		 * Each name filter is a wildcard (globbing) filter that understands * and ? wildcards.
		 */
		Q_PROPERTY(QScriptValue /*% Array %*/ nameFilters READ getNameFilters WRITE setNameFilters);

		/** \brief Gets or sets the filter used by entryList() and entryInfoList().
		 *
		 * The filter is used to specify the kind of files that should be returned by entryList() and
		 * entryInfoList().
		 */
		Q_PROPERTY(Filter filter READ getFilter WRITE setFilter);

		/** \brief Gets or sets the sort order used by entryList() and entryInfoList().
		 */
		Q_PROPERTY(Sort sorting READ getSorting WRITE setSorting);

		/** \brief Returns the total number of directories and files in the directory.
		 *
		 * Equivalent to entryList().length.
		 */
		Q_PROPERTY(uint /*% Number %*/ count READ getCount);

		/** \brief Returns true if the directory is readable and we can open files by name; otherwise
		 * returns false.
		 *
		 * Warning: A false value from this function isn't a guarantee that files in the directory
		 * are not accessible.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isReadable READ isReadable);

		/** \brief Returns true if the directory exists; otherwise returns false.
		 *
		 * If a file with the same name is found this function will return false.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ exists READ exists);

		/** \brief Returns true if the directory is the root directory; otherwise returns false.
		 *
		 * Note: If the directory is a symbolic link to the root directory this function returns false.
		 * If you want to test for this use canonicalPath.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isRoot READ isRoot);

		/** \brief Returns true if the directory path is relative; otherwise returns false.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isRelative READ isRelative);

		/** \brief Returns true if the directory's path is absolute; otherwise returns false.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isAbsolute READ isAbsolute);

		/** \brief Returns the native directory separator: "/" under Unix (including Mac OS X) and
		 * "\" under Windows.
		 *
		 * You do not need to use this function to build file paths. If you always use "/", the
		 * Directory class will translate your paths to conform to the underlying operating system.
		 * If you want to display paths to the user using their operating system's separator use
		 * toNativeSeparators()
		 */
		Q_PROPERTY(QString /*% String %*/ separator READ getSeparator);

		/** \brief Gets or sets the current path.
		 */
		Q_PROPERTY(QString /*% String %*/ currentPath READ getCurrentPath WRITE setCurrentPath);

		/** \brief Returns the absolute path of the user's home directory.
		 *
		 * Under Windows this function will return the directory of the current user's profile.
		 * Typically, this is: C:/Documents and Settings/Username
		 *
		 * Use the toNativeSeparators() function to convert the separators to the ones that are
		 * appropriate for the underlying operating system.
		 *
		 * If the directory of the current user's profile does not exist or cannot be retrieved,
		 * the following alternatives will be checked (in the given order) until an existing and
		 * available path is found:
		 *
		 * \li The path specified by the USERPROFILE environment variable.
		 * \li The path formed by concatenating the HOMEDRIVE and HOMEPATH environment variables.
		 * \li The path specified by the HOME environment variable.
		 * \li The path returned by the rootPath property (which uses the SystemDrive environment variable).
		 * \li The C:/ directory.
		 *
		 * Under non-Windows operating systems the HOME environment variable is used if it exists,
		 * otherwise the path returned by the rootPath() function is used.
		 */
		Q_PROPERTY(QString /*% String %*/ homePath READ getHomePath);

		/** \brief Returns the absolute path of the root directory.
		 *
		 * For Unix operating systems this returns "/". For Windows file systems this normally
		 * returns "c:/".
		 */
		Q_PROPERTY(QString /*% String %*/ rootPath READ getRootPath);

		/** \brief Returns the absolute path of the system's temporary directory.
		 *
		 * On Unix/Linux systems this is usually /tmp; on Windows this is usually the path in the
		 * TEMP or TMP environment variable. Whether a directory separator is added to the end or not,
		 * depends on the operating system.
		 */
		Q_PROPERTY(QString /*% String %*/ tempPath READ getTempPath);

		/** \brief Returns the path name of a file in the directory.
		 *
		 * Does not check if the file actually exists in the directory; but see Directory.exists.
		 * If the Directory object is relative, the returned path name will also be relative.
		 * Redundant multiple separators or "." and ".." directories in fileName are not removed
		 * (see cleanPath()).
		 */
		Q_INVOKABLE QString /*% String %*/ filePath(QString /*% String %*/ fileName);

		/** \brief Returns the absolute path name of a file in the directory.
		 *
		 * Does not check if the file actually exists in the directory; but see Directory.exists.
		 * Redundant multiple separators or "." and ".." directories in fileName are not removed
		 * (see cleanPath()).
		 */
		Q_INVOKABLE QString /*% String %*/ absoluteFilePath(QString /*% String %*/ fileName);

		/** \brief Returns the path to fileName relative to the directory.
		 */
		Q_INVOKABLE QString /*% String %*/ relativeFilePath(QString /*% String %*/ fileName);

		/** \brief Returns pathName with the '/' separators converted to separators that are appropriate
		 * for the underlying operating system.
		 *
		 * On Windows, toNativeSeparators("c:/winnt/system32") returns "c:\winnt\system32". The returned
		 * string may be the same as the argument on some operating systems, for example on Unix.
		 */
		Q_INVOKABLE QString /*% String %*/ toNativeSeparators(QString /*% String %*/ pathName);

		/** \brief Returns pathName using '/' as file separator.
		 *
		 * On Windows, for instance, fromNativeSeparators("c:\\winnt\\system32") returns
		 * "c:/winnt/system32". The returned string may be the same as the argument on some operating
		 * systems, for example on Unix.
		 */
		Q_INVOKABLE QString /*% String %*/ fromNativeSeparators(QString /*% String %*/ pathName);

		/** \brief Changes the Directory object's directory to dirName.
		 *
		 * Note that the logical cd() operation is not performed if the new directory does not exist.
		 * Calling cd("..") is equivalent to calling cdUp().
		 *
		 * \return Returns true if the new directory exists and is readable; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ changeDirectory(QString /*% String %*/ dirName);

		/** \brief Changes directory by moving one directory up from the Directory object's current
		 * directory.
		 *
		 * Note that the logical changeDirectoryUp() operation is not performed if the new directory
		 * does not exist.
		 *
		 * \return Returns true if the new directory exists and is readable; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ changeDirectoryUp();

		/** \brief Returns a list of the names of all the files and directories in the directory.
		 *
		 * The list is ordered according to the name and attribute filters previously set with the
		 * nameFilters and filter properties.  The list is also sorted according to the flags set
		 * with the sorting property.
		 *
		 * The attribute filter and sorting specifications can be overridden using the filters and
		 * sort arguments.
		 *
		 * Note: To list symlinks that point to non existing files, System must be passed to the filter.
		 *
		 * \return Returns an empty list if the directory is unreadable, does not exist, or if nothing
		 * matches the specification.
		 */
		Q_INVOKABLE QScriptValue /*% Array %*/ entryList(Filter filters = NoFilter, Sort sort = NoSort);

		/** \brief Returns a list of the names of all the files and directories in the directory.
		 *
		 * The list is ordered according to the name and attribute filters previously set with
		 * the nameFilters and filter properties.  The list is also sorted according to the flags set
		 * with the sorting property.
		 *
		 * The name filter, file attribute filter and sorting specification can be overridden using
		 * the nameFilters, filters, and sort arguments.
		 *
		 * \return Returns an empty list if the directory is unreadable, does not exist, or if nothing
		 * matches the specification.
		 */
		Q_INVOKABLE QScriptValue /*% Array %*/ entryList(QScriptValue /*% Array %*/ nameFilters, Filter filters = NoFilter, Sort sort = NoSort);

		//QFileInfoList entryInfoList(Filters filters = NoFilter, SortFlags sort = NoSort);
		//QFileInfoList entryInfoList(const QStringList &nameFilters, Filters filters = NoFilter, SortFlags sort = NoSort);
		//QFileInfoList drives();

		/** \brief Creates a sub-directory called dirName.
		 * \return Returns true on success; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ makeDirectory(QString /*% String %*/ dirName);

		/** \brief Removes the directory specified by dirName.
		 *
		 * The directory must be empty for removeDirectory() to succeed.
		 *
		 * \return Returns true if successful; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ removeDirectory(QString /*% String %*/ dirName);

		/** \brief Creates the directory path dirPath.
		 *
		 * The function will create all parent directories necessary to create the directory.
		 *
		 * \return Returns true if successful; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ makePath(QString /*% String %*/ dirPath);

		/** \brief Removes the directory path dirPath.
		 *
		 * The function will remove all parent directories in dirPath, provided that they are empty.
		 * This is the opposite of makePath(dirPath).
		 *
		 * \return Returns true if successful; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ removePath(QString /*% String %*/ dirPath);

		/** \brief Converts the directory path to an absolute path.
		 *
		 * If it is already absolute nothing happens.
		 *
		 * \return Returns true if the conversion succeeded; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ makeAbsolute();

		/** \brief Removes the file, fileName.
		 *
		 * \return Returns true if the file is removed successfully; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ remove(QString /*% String %*/ fileName);

		/** \brief Renames a file or directory from oldName to newName
		 *
		 * On most file systems, rename() fails only if oldName does not exist, if newName and oldName
		 * are not on the same partition or if a file with the new name already exists. However, there
		 * are also other reasons why rename() can fail. For example, on at least one file system rename()
		 * fails if newName points to an open file.
		 *
		 * \return Returns true if successful; otherwise returns false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ rename(QString /*% String %*/ oldName, QString /*% String %*/ newName);

		/** \brief Returns true if the file called name exists; otherwise returns false.
		 *
		 * Unless name contains an absolute file path, the file name is assumed to be relative to the
		 * current directory.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ exists(QString /*% String %*/ name);

		/** \brief Returns true if the fileName matches any of the wildcard (glob) patterns in the list
		 * of filters; otherwise returns false.
		 *
		 * The matching is case insensitive.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ match(QScriptValue /*% Array %*/ filters, QString /*% String %*/ fileName);

		/** \brief Returns true if the fileName matches the wildcard (glob) pattern filter; otherwise
		 * returns false.
		 *
		 * The filter may contain multiple patterns separated by spaces or semicolons. The matching is
		 * case insensitive.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ match(QString /*% String %*/ filter, QString /*% String %*/ fileName);

		/** \brief Removes all multiple directory separators "/" and resolves any "." or ".." found in
		 * the path.
		 *
		 * Symbolic links are kept. This function does not return the canonical path, but rather the
		 * simplest version of the input. For example, "./local" becomes "local", "local/../bin" becomes
		 * "bin" and "/local/usr/../bin" becomes "/local/bin".
		 */
		Q_INVOKABLE QString /*% String %*/ cleanPath(QString /*% String %*/ path);

		/** \brief Refreshes the directory information.
		 */
		Q_INVOKABLE void refresh();
	};
}

#endif
