#include <QtScript/QScriptEngine>

#include "Directory.h"
#include "FileInformation.h"

namespace ADM_qtScript
{
    FileInformation::FileInformation(const QString& file)
    {
        _fileInfo = QFileInfo(file);
    }

    FileInformation::FileInformation(const File& file)
    {
        _fileInfo = QFileInfo(file._file);
    }

    FileInformation::FileInformation(const Directory& directory, const QString& file)
    {
        _fileInfo = QFileInfo(directory._dir, file);
    }

    QScriptValue FileInformation::constructor(QScriptContext *context, QScriptEngine *engine)
    {
        if (context->isCalledAsConstructor())
        {
            File *file = NULL;
            Directory *directory = NULL;

            if (context->argumentCount() == 1)
            {
                file = qobject_cast<File*>(context->argument(0).toQObject());
            }
            else if (context->argumentCount() == 2)
            {
                directory = qobject_cast<Directory*>(context->argument(0).toQObject());
            }

            if (context->argumentCount() == 1 && context->argument(0).isString())
            {
                return engine->newQObject(
                           new FileInformation(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
            }

            if (context->argumentCount() == 1 && file != NULL)
            {
                return engine->newQObject(
                           new FileInformation(*file), QScriptEngine::ScriptOwnership);
            }
            else if (context->argumentCount() == 2 && directory != NULL && context->argument(1).isString())
            {
                return engine->newQObject(
                           new FileInformation(*directory, context->argument(1).toString()), QScriptEngine::ScriptOwnership);
            }
            else
            {
                return context->throwError("Invalid arguments passed to constructor");
            }
        }

        return engine->undefinedValue();
    }

    bool FileInformation::exists()
    {
        return _fileInfo.exists();
    }

    bool FileInformation::isAbsolute()
    {
        return _fileInfo.isAbsolute();
    }

    bool FileInformation::isBundle()
    {
        return _fileInfo.isBundle();
    }

    bool FileInformation::isDirectory()
    {
        return _fileInfo.isDir();
    }

    bool FileInformation::isExecutable()
    {
        return _fileInfo.isExecutable();
    }

    bool FileInformation::isFile()
    {
        return _fileInfo.isFile();
    }

    bool FileInformation::isHidden()
    {
        return _fileInfo.isHidden();
    }

    bool FileInformation::isReadable()
    {
        return _fileInfo.isReadable();
    }

    bool FileInformation::isRelative()
    {
        return _fileInfo.isRelative();
    }

    bool FileInformation::isRoot()
    {
        return _fileInfo.isRoot();
    }

    bool FileInformation::isSymLink()
    {
        return _fileInfo.isSymLink();
    }

    bool FileInformation::isWritable()
    {
        return _fileInfo.isWritable();
    }

    bool FileInformation::makeAbsolute()
    {
        return _fileInfo.makeAbsolute();
    }

    void FileInformation::refresh()
    {
        _fileInfo.refresh();
    }

    QScriptValue FileInformation::getAbsoluteDirectory()
    {
        return this->engine()->newQObject(new Directory(_fileInfo.absolutePath()), QScriptEngine::ScriptOwnership);
    }

    QString FileInformation::getAbsoluteFilePath()
    {
        return _fileInfo.absoluteFilePath();
    }

    QString FileInformation::getAbsolutePath()
    {
        return _fileInfo.absolutePath();
    }

    QString FileInformation::getBaseName()
    {
        return _fileInfo.baseName();
    }

    QString FileInformation::getBundleName()
    {
        return _fileInfo.bundleName();
    }

    bool FileInformation::getCaching()
    {
        return _fileInfo.caching();
    }

    QString FileInformation::getCanonicalFilePath()
    {
        return _fileInfo.canonicalFilePath();
    }

    QString FileInformation::getCanonicalPath()
    {
        return _fileInfo.canonicalPath();
    }

    QString FileInformation::getCompleteBaseName()
    {
        return _fileInfo.completeBaseName();
    }

    QString FileInformation::getCompleteSuffix()
    {
        return _fileInfo.completeSuffix();
    }

    QDateTime FileInformation::getCreated()
    {
        return _fileInfo.created();
    }

    QScriptValue FileInformation::getDirectory()
    {
        return this->engine()->newQObject(new Directory(_fileInfo.path()), QScriptEngine::ScriptOwnership);
    }

    QString FileInformation::getFileName()
    {
        return _fileInfo.fileName();
    }

    QString FileInformation::getFilePath()
    {
        return _fileInfo.filePath();
    }

    QString FileInformation::getGroup()
    {
        return _fileInfo.group();
    }

    unsigned FileInformation::getGroupId()
    {
        return _fileInfo.groupId();
    }

    QDateTime FileInformation::getLastModified()
    {
        return _fileInfo.lastModified();
    }

    QDateTime FileInformation::getLastRead()
    {
        return _fileInfo.lastRead();
    }

    QString FileInformation::getOwner()
    {
        return _fileInfo.owner();
    }

    unsigned int FileInformation::getOwnerId()
    {
        return _fileInfo.ownerId();
    }

    QString FileInformation::getPath()
    {
        return _fileInfo.path();
    }

    File::Permissions FileInformation::getPermissions()
    {
        return File::getPermissions(_fileInfo.permissions());
    }

    qint64 FileInformation::getSize()
    {
        return _fileInfo.size();
    }

    QString FileInformation::getSuffix()
    {
        return _fileInfo.suffix();
    }

    QString FileInformation::getSymLinkTarget()
    {
        return _fileInfo.symLinkTarget();
    }

    bool FileInformation::permission(File::Permissions permissions)
    {
        return _fileInfo.permission(File::getQPermissions(permissions));
    }

    void FileInformation::setCaching(bool caching)
    {
        return _fileInfo.setCaching(caching);
    }
}
