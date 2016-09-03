#ifndef ADM_qtScript_File
#define ADM_qtScript_File

#include <QtCore/QFile>
#include <QtScript/QScriptable>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

namespace ADM_qtScript
{
	/** \brief The File %class provides an interface for reading from and writing to files.
	 */
	class File : public QObject, protected QScriptable
	{
		Q_OBJECT
		Q_ENUMS(FileError OpenMode Permission)

	public:
		/** \brief Describes the errors that may be returned by the File.error property.
		 */
		enum FileError
		{
			NoError = 0, /**< No error occurred. */
			ReadError = 1, /**< An error occurred when reading from the file. */
			WriteError = 2, /**< An error occurred when writing to the file. */
			FatalError = 3, /**< A fatal error occurred. */
			ResourceError = 4, /**< A resource error occurred. */
			OpenError = 5, /**< The file could not be opened. */
			AbortError = 6, /**< The operation was aborted. */
			TimeOutError = 7, /**< A timeout occurred. */
			UnspecifiedError = 8, /**< An unspecified error occurred. */
			RemoveError = 9, /**< The file could not be removed. */
			RenameError = 10, /**< The file could not be renamed. */
			PositionError = 11, /**< The position in the file could not be changed. */
			ResizeError = 12, /**< The file could not be resized. */
			PermissionsError = 13, /**< The file could not be accessed. */
			CopyError = 14 /**< The file could not be copied. */
		};

		/** \brief Used by the File.permission property to report the permissions and ownership of a file.
		 *
		 * The values may be OR-ed together to test multiple permissions and ownership values.
		 */
		enum Permission
		{
			ReadOwner = 0x4000, /**< The file is readable by the owner of the file. */
			WriteOwner = 0x2000, /**< The file is writable by the owner of the file. */
			ExeOwner = 0x1000, /**< The file is executable by the owner of the file. */
			ReadUser  = 0x0400, /**< The file is readable by the user. */
			WriteUser  = 0x0200, /**< The file is writable by the user. */
			ExeUser  = 0x0100, /**< The file is executable by the user. */
			ReadGroup = 0x0040, /**< The file is readable by the group. */
			WriteGroup = 0x0020, /**< The file is writable by the group. */
			ExeGroup = 0x0010, /**< The file is executable by the group. */
			ReadOther = 0x0004, /**< The file is readable by anyone. */
			WriteOther = 0x0002, /**< The file is writable by anyone. */
			ExeOther = 0x0001 /**< The file is executable by anyone. */
		};
		Q_DECLARE_FLAGS(Permissions, Permission)

		/** \brief Used with open() to describe the mode in which a file is opened. It is also returned by the File.openMode property.
		 */
		enum OpenMode
		{
			NotOpen = 0x0000, /**< The file is not open. */
			ReadOnly = 0x0001, /**< The file is open for reading. */
			WriteOnly = 0x0002, /**< The file is open for writing. */
			ReadWrite = ReadOnly | WriteOnly, /**< The file is open for reading and writing. */
			Append = 0x0004, /**< The file is opened in append mode, so that all data is written to the end of the file. */
			Truncate = 0x0008, /**< If possible, the file is truncated before it is opened. All earlier contents of the file are lost. */
			Text = 0x0010, /**< When reading, the end-of-line terminators are translated to '\\n'. When writing, the end-of-line terminators are translated to the local encoding, for example '\\r\\n' for Microsoft Windows. */
			Unbuffered = 0x0020 /**< Any buffer in the device is bypassed. */
		};
		Q_DECLARE_FLAGS(OpenModeFlags, OpenMode)

	private:
		bool atEnd();
		qint64 getBytesAvailable();
		qint64 getBytesToWrite();
		File::FileError getError();
		QString getErrorString();
		QString getFileName();
		File::OpenMode getOpenMode();
		Permissions getPermissions();
		qint64 getPosition();
		qint64 getSize();
		bool isOpen();
		bool isReadable();
		bool isSequential();
		bool isTextModeEnabled();
		bool isWritable();

	public:
	    /** \cond */
	    QFile _file;

		static Permissions getPermissions(QFile::Permissions permissions);
		static QFile::Permissions getQPermissions(Permissions permissions);
	    /** \endcond */

		/** \brief Constructs a new file object to represent the file with the given name.
		 */
		File(const QString& /*% String %*/ name);

		/** \cond */
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);
		/** \endcond */

		/** \brief Returns true if the current read and write position is at the end of the file
		 * (i.e. there is no more data available for reading); otherwise returns false.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ atEnd READ atEnd);

		/** \brief Returns the number of bytes that are available for reading.
		 */
		Q_PROPERTY(qint64 /*% Number %*/ bytesAvailable READ getBytesAvailable);

		/** \brief For buffered devices, this function returns the number of bytes waiting to be
		 * written. For devices with no buffer, this function returns 0.
		 */
		Q_PROPERTY(qint64 /*% Number %*/ bytesToWrite READ getBytesToWrite)

		/** \brief Returns the file error status.
		 *
		 * The I/O device status returns an error code. For example, if open() returns false, or
		 * a read/write operation returns -1, this function can be called to find out the reason
		 * why the operation failed.
		 */
		Q_PROPERTY(FileError error READ getError);

		/** \brief Returns a human-readable description of the last file error that occurred.
		 */
		Q_PROPERTY(QString /*% String %*/ errorString READ getErrorString);

		/** \brief Returns the name set by the File constructor.
		 */
		Q_PROPERTY(QString /*% String %*/ fileName READ getFileName);

		/** \brief Returns true if the file is open; otherwise false.
		 *
		 * A file is open if it can be read from and/or written to. This property returns false if
		 * File.openMode returns File::NotOpen.
		 *
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isOpen READ isOpen);

		/** \brief Returns true if data can be read from the file; otherwise false.
		 *
		 * Use bytesAvailable() to determine how many bytes can be read.
		 * This is a convenience function which checks if the File.openMode of the file contains
		 * the File::ReadOnly flag.
		 *
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isReadable READ isReadable);

		/** \brief Returns true if this file is sequential; otherwise false.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isSequential READ isSequential);

		/** \brief Returns true if the File.Text flag is enabled; otherwise false.
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isTextModeEnabled READ isTextModeEnabled);

		/** \brief Returns true if data can be written to the file; otherwise false.
		 *
		 * This is a convenience function which checks if the File.openMode of the file contains the
		 * File::WriteOnly flag.
		 *
		 */
		Q_PROPERTY(bool /*% Boolean %*/ isWritable READ isWritable);

		/** \brief Returns the mode in which the file has been opened; i.e. File::ReadOnly or File::WriteOnly.
		 */
		Q_PROPERTY(OpenMode /*% OpenMode %*/ openMode READ getOpenMode);

		/** \brief Returns the OR-ed combination of File::Permission for the file.
		 */
		Q_PROPERTY(Permissions /*% Permission %*/ permissions READ getPermissions);

		/** \brief Returns the position that data is written to or read from.
		 */
		Q_PROPERTY(qint64 /*% Number %*/ position READ getPosition);

		/** \brief Returns the size of the file.
		 */
		Q_PROPERTY(qint64 /*% Number %*/ size READ getSize);

		/** \brief Returns true if a complete line of data can be read from the device; otherwise returns false.
		 *
		 * Note that unbuffered devices, which have no way of determining what can be read, always return false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ canReadLine();

		/** \brief Closes the file and sets its File.openMode to File::NotOpen. The error string is also reset.
		 */
		Q_INVOKABLE void close();

		/** \brief Copies the file currently specified by File.fileName to a file called newName.
		 *
		 * The source file is closed before it is copied.
		 *
		 * \return Returns true if successful; otherwise false. Note that if a file with the name newName
		 * already exists, copy() returns false (i.e. File will not overwrite it).
		 */
		Q_INVOKABLE bool /*% Boolean %*/ copy(const QString& /*% String %*/ newName);

		/** \brief Returns true if the file specified by File.fileName exists; otherwise false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ exists();

		/** \brief Flushes any buffered data to the file.
		 * \return Returns true if successful; otherwise false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ flush();

		/** \brief Creates a link named linkName that points to the file currently specified by File.fileName.
		 *
		 * What a link is depends on the underlying filesystem (be it a shortcut on Windows or a symbolic
		 * link on Unix).
		 * Note: To create a valid link on Windows, linkName must have a .lnk file extension.
		 *
		 * \return Returns true if successful; otherwise false. This function will not overwrite
		 * an already existing entity in the file system; in this case, link() will return false
		 * and set the File.error property to return File::RenameError.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ link(const QString& /*% String %*/ linkName);

		/** \brief Opens the file and sets its File.openMode to mode.
		 * \return Returns true if successful; otherwise false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ open(OpenMode /*% OpenMode %*/ mode);

		/** \brief Peeks at most maxSize bytes from the file, returning the data peeked.
		 *
		 * This function has no way of reporting errors; returning an empty String can mean either
		 * that no data was currently available for peeking, or that an error occurred.
		 *
		 */
		Q_INVOKABLE QString /*% String %*/ peek(qint64 /*% Number %*/ maxSize);

		/** \brief Reads and returns at most maxSize bytes from the file.
		 */
		Q_INVOKABLE QString /*% String %*/ read(qint64 /*% Number %*/ maxSize);

		/** \brief Reads and returns all available data from the file
		 */
		Q_INVOKABLE QString /*% String %*/ readAll();

		/** \brief Reads and returns a line from the file but no more than maxSize characters.
		 */
		Q_INVOKABLE QString /*% String %*/ readLine(qint64 /*% Number %*/ maxSize = 0);

		/** \brief Removes the file specified by File.fileName.
		 *
		 * The file is closed before it is removed.
		 *
		 * \return Returns true if successful; otherwise false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ remove();

		/** \brief Renames the file currently specified by File.fileName to newName.
		  *
		  * The file is closed before it is renamed.
		  *
		  * \return Returns true if successful; otherwise false. If a file with the name newName
		  * already exists, rename() returns false (i.e., File will not overwrite it).
		  */
		Q_INVOKABLE bool /*% Boolean %*/ rename(const QString& /*% String %*/ newName);

		/** \brief Seeks to the start of the file.
		 * \return Returns true on success; otherwise false (for example, if the file is closed).
		 */
		Q_INVOKABLE bool /*% Boolean %*/ reset();

		/** \brief Sets the file size (in bytes).
		 *
		 * Note: if the specified size is larger than the file's current size then the file is truncated to 0 bytes.
		 *
		 * \return Returns true if the resize succeeds; otherwise false.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ resize(qint64 /*% Number %*/ size);

		/** \brief Sets the current position to pos.
		 * \return Returns true on success, or false if an error occurred.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ seek(qint64 /*% Number %*/ pos);

		/** \brief Sets the permissions for the file to the permissions specified.
		 * \return Returns true if successful; otherwise false if the permissions cannot be modified.
		 */
		Q_INVOKABLE bool /*% Boolean %*/ setPermissions(Permissions /*% Permission %*/ permissions);

		/** \brief If enabled, this function sets the Text flag on the file; otherwise the Text flag is removed.
		 */
		Q_INVOKABLE void setTextModeEnabled (bool /*% Boolean %*/ enabled);

		/** \brief Returns the absolute path of the file or directory that a symlink (or shortcut on Windows)
		 * points to. An empty string is returned if the object isn't a symbolic link.
		 *
		 * This name may not represent an existing file; it is only a string. File::exists() returns true
		 * if the symlink points to an existing file.
		 */
		Q_INVOKABLE QString /*% String %*/ symLinkTarget();

		/** \brief Sets the file's error to File::NoError.
		 */
		Q_INVOKABLE void unsetError();

		/** \brief Writes the passed content to the file.
		 * \return Returns the number of bytes that were actually written or -1 if an error occurred.
		 */
		Q_INVOKABLE qint64 /*% Number %*/ write(const QString& /*% String %*/ data);
	};
}
#endif
