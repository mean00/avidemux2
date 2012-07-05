#ifndef ADM_qtScript_FileInformation
#define ADM_qtScript_FileInformation

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtScript/QScriptable>
#include <QtScript/QScriptValue>

#include "Directory.h"
#include "File.h"

namespace ADM_qtScript
{
    /** \brief The FileInformation %class provides system-independent file information.
     */
    class FileInformation : public QObject, protected QScriptable
    {
        Q_OBJECT

    private:
        QFileInfo _fileInfo;

        QScriptValue getAbsoluteDirectory();
        QString getAbsoluteFilePath();
        QString getAbsolutePath();
        QString getBaseName();
        QString getBundleName();
        bool getCaching();
        QString getCanonicalFilePath();
        QString getCanonicalPath();
        QString getCompleteBaseName();
        QString getCompleteSuffix();
        QDateTime getCreated();
        QScriptValue getDirectory();
        QString getFileName();
        QString getFilePath();
        QString getGroup();
        unsigned int getGroupId();
        QDateTime getLastModified();
        QDateTime getLastRead();
        QString getOwner();
        unsigned int getOwnerId();
        QString getPath();
        File::Permissions getPermissions();
        qint64 getSize();
        QString getSuffix();
        QString getSymLinkTarget();
        void setCaching(bool caching);

    public:
        /** \brief Constructs a new FileInformation that gives information about the given file.
         *
         * The file can also include an absolute or relative path.
		 */
        FileInformation(const QString& /*% String %*/ file);

        /** \brief Constructs a new FileInformation that gives information about file.
         *
         * If the file has a relative path, the FileInformation will also have a relative path.
         */
        FileInformation(const File& /*% File %*/ file);

        /** \brief Constructs a new FileInformation that gives information about the given file in the specified directory.
         *
         * If directory has a relative path, the FileInformation will also have a relative path.
         * If file is an absolute path, then the directory specified will be disregarded.
         */
        FileInformation(const Directory& /*% Directory %*/ directory, const QString& /*% String %*/ file);

        /** \cond */
		FileInformation(const QFileInfo& fileInfo);

        static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);
        /** \endcond */

        /** \brief Returns the file's absolute path as a Directory object.
         */
        Q_PROPERTY(QScriptValue /*% Directory %*/ absoluteDirectory READ getAbsoluteDirectory);

        /** \brief Returns an absolute path including the file name.
         *
         * The absolute path name consists of the full path and the file name.
         * On Unix this will always begin with the root, '/', directory.
         * On Windows this will always begin 'D:/' where D is a drive letter, except for network shares that are not mapped
         * to a drive letter, in which case the path will begin '//sharename/'.
         * FileInformation will uppercase drive letters. Note that Directory does not do this.
         *
         * This function returns the same as FileInformation::filePath, unless FileInformation::isRelative is true. In contrast to
         * FileInformation::canonicalFilePath, symbolic links or redundant "." or ".." elements are not necessarily removed.
         */
        Q_PROPERTY(QString /*% String %*/ absoluteFilePath READ getAbsoluteFilePath);

        /** \brief Returns a file's path absolute path. This doesn't include the file name.
         *
         * On Unix the absolute path will always begin with the root, '/', directory.
         * On Windows this will always begin 'D:/' where D is a drive letter, except for network shares that are not mapped
         * to a drive letter, in which case the path will begin '//sharename/'.
         *
         * In contrast to FileInformation::canonicalPath symbolic links or redundant "." or ".." elements are not necessarily removed.
         */
        Q_PROPERTY(QString /*% String %*/ absolutePath READ getAbsolutePath);

        /** \brief Returns the base name of the file without the path.
         *
         * The base name consists of all characters in the file up to (but not including) the first '.' character.
         */
        Q_PROPERTY(QString /*% String %*/ baseName READ getBaseName);

        /** \brief Returns the name of the bundle.
         *
         * On Mac OS X this returns the proper localized name for a bundle if the path FileInformation::isBundle.
         * On all other platforms an empty String is returned.
         */
        Q_PROPERTY(QString /*% String %*/ bundleName READ getBundleName);

        /** \brief Returns true if caching is enabled; otherwise returns false.
         */
        Q_PROPERTY(bool /*% Boolean %*/ caching READ getCaching WRITE setCaching);

        /** \brief Returns the canonical path including the file name, i.e. an absolute path without symbolic links or
         * redundant "." or ".." elements.
         *
         * An empty string is returned if the file does not exist.
         */
        Q_PROPERTY(QString /*% String %*/ canonicalFilePath READ getCanonicalFilePath);

        /** \brief Returns the file's path canonical path (excluding the file name), i.e. an absolute path without symbolic
         * links or redundant "." or ".." elements.
         *
         * An empty string is returned if the file does not exist.
         */
        Q_PROPERTY(QString /*% String %*/ canonicalPath READ getCanonicalPath);

        /** \brief Returns the complete base name of the file without the path.
         *
         * The complete base name consists of all characters in the file up to (but not including) the last '.' character.
         */
        Q_PROPERTY(QString /*% String %*/ completeBaseName READ getCompleteBaseName);

        /** \brief Returns the complete suffix of the file.
         *
         * The complete suffix consists of all characters in the file after (but not including) the first '.'.
         */
        Q_PROPERTY(QString /*% String %*/ completeSuffix READ getCompleteSuffix);

        /** \brief Returns the date and time when the file was created.
         *
         * On most Unix systems, this function returns the time of the last status change. A status change occurs when the file is
         * created, but it also occurs whenever the user writes or sets inode information (for example, changing the file permissions).
         *
         * If neither creation time nor "last status change" time are available, returns the same as FileInformation::lastModified.
         */
        Q_PROPERTY(QDateTime /*% Date %*/ created READ getCreated);

        /** \brief Returns the path of the object's parent directory as a Directory object.
         *
         * Note: The Directory returned always corresponds to the object's parent directory, even if the FileInformation
         * represents a directory.
         */
        Q_PROPERTY(QScriptValue /*% Directory %*/ directory READ getDirectory);

        /** \brief Returns the name of the file, excluding the path.
         */
        Q_PROPERTY(QString /*% String %*/ fileName READ getFileName);

        /** \brief Returns the file name, including the path (which may be absolute or relative).
         */
        Q_PROPERTY(QString /*% String %*/ filePath READ getFilePath);

        /** \brief Returns the group of the file.
         *
         * On Windows, on systems where files do not have groups, or if an error occurs, an empty string is returned.
         * This function can be time consuming under Unix (in the order of milliseconds).
         */
        Q_PROPERTY(QString /*% String %*/ group READ getGroup);

        /** \brief Returns the id of the group the file belongs to.
         *
         * On Windows and on systems where files do not have groups this function always returns -2.
         */
        Q_PROPERTY(uint /*% Number %*/ groupId READ getGroupId);

        /** \brief Returns the date and time when the file was last modified.
         */
        Q_PROPERTY(QDateTime /*% Date %*/ lastModified READ getLastModified);

        /** \brief Returns the date and time when the file was last read (accessed).
         *
         * On platforms where this information is not available, returns the same as FileInformation::lastModified.
         */
        Q_PROPERTY(QDateTime /*% Date %*/ lastRead READ getLastRead);

        /** \brief Returns the owner of the file.
         *
         * On systems where files do not have owners, or if an error occurs, an empty string is returned.
         * This function can be time consuming under Unix (in the order of milliseconds).
         */
        Q_PROPERTY(QString /*% String %*/ owner READ getOwner);

        /** \brief Returns the id of the owner of the file.
         *
         * On Windows and on systems where files do not have owners this function returns -2.
         */
        Q_PROPERTY(uint /*% Number %*/ ownerId READ getOwnerId);

        /** \brief Returns the complete OR-ed together combination of File::Permission for the file.
         */
        Q_PROPERTY(ADM_qtScript::File::Permissions /*% File::Permission %*/ permissions READ getPermissions);

        /** \brief Returns the file's path. This doesn't include the file name.
         *
         * Note that, if this FileInformation object is given a path ending in a slash, the name of the file is considered empty and
         * this function will return the entire path.
         */
        Q_PROPERTY(QString /*% String %*/ path READ getPath);

        /** \brief Returns the file size in bytes. If the file does not exist or cannot be fetched, 0 is returned.
         */
        Q_PROPERTY(qint64 /*% Number %*/ size READ getSize);

        /** \brief Returns the suffix of the file.
         *
         * The suffix consists of all characters in the file after (but not including) the last '.'.
         */
        Q_PROPERTY(QString /*% String %*/ suffix READ getSuffix);

        /** \brief Returns the absolute path to the file or directory a symlink (or shortcut on Windows) points to, or a an empty
         * string if the object isn't a symbolic link.
         *
         * This name may not represent an existing file; it is only a string. FileInformation::exists returns true if the symlink
         * points to an existing file.
         */
        Q_PROPERTY(QString /*% String %*/ symLinkTarget READ getSymLinkTarget);

        /** \brief Returns true if the file exists; otherwise returns false.
         *
         * Note: If the file is a symlink that points to a non existing file, false is returned.
         */
        Q_INVOKABLE bool /*% Boolean %*/ exists();

        /** \brief Returns true if the file path name is absolute, otherwise returns false if the path is relative.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isAbsolute();

        /** \brief Returns true if this object points to a bundle or to a symbolic link to a bundle on Mac OS X; otherwise returns false.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isBundle();

        /** \brief Returns true if this object points to a directory or to a symbolic link to a directory; otherwise returns false.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isDirectory();

        /** \brief Returns true if the file is executable; otherwise returns false.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isExecutable();

        /** \brief Returns true if this object points to a file or to a symbolic link to a file.
         *
         * Returns false if the object points to something which isn't a file, such as a directory.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isFile();

        /** \brief Returns true if this is a `hidden' file; otherwise returns false.
         *
         * Note: This function returns true for the special entries "." and ".." on Unix, even though
         * Directory::entryList treats them as shown.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isHidden();

        /** \brief Returns true if the user can read the file; otherwise returns false.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isReadable();

        /** \brief Returns true if the file path name is relative, otherwise returns false if the path is absolute
         * (e.g. under Unix a path is absolute if it begins with a "/").
         */
        Q_INVOKABLE bool /*% Boolean %*/ isRelative();

        /** \brief Returns true if the object points to a directory or to a symbolic link to a directory, and that
         * directory is the root directory; otherwise returns false.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isRoot();

        /** \brief Returns true if this object points to a symbolic link (or to a shortcut on Windows); otherwise returns false.
         *
         * On Unix (including Mac OS X), opening a symlink effectively opens the link's target. On Windows, it opens the .lnk file itself.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isSymLink();

        /** \brief Returns true if the user can write to the file; otherwise returns false.
         */
        Q_INVOKABLE bool /*% Boolean %*/ isWritable();

        /** \brief Converts the file's path to an absolute path if it is not already in that form.
         * \returns Returns true to indicate that the path was converted; otherwise returns false to indicate that the path was already absolute.
         */
        Q_INVOKABLE bool /*% Boolean %*/ makeAbsolute();

        /** \brief Tests for file permissions.
         *
         * The permissions argument can be several flags of type File::Permission OR-ed together to check for permission combinations.
         */
        Q_INVOKABLE bool /*% Boolean %*/ permission(File::Permissions /*% File::Permission %*/ permissions);

        /** \brief Refreshes the information about the file, i.e. reads in information from the file system the next time
         * a cached property is fetched.
         */
        Q_INVOKABLE void refresh();
    };
}
#endif
