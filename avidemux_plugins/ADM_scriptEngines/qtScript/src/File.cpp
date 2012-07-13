#include "File.h"

namespace ADM_qtScript
{
    static const QFile::Permission qtPermissions[] =
    {
        QFile::ReadOwner, QFile::WriteOwner, QFile::ExeOwner, QFile::ReadUser, QFile::WriteUser, QFile::ExeUser,
        QFile::ReadGroup, QFile::WriteGroup, QFile::ExeGroup, QFile::ReadOther, QFile::WriteOther, QFile::ExeOther
    };

    static const File::Permission myPermissions[] =
    {
        File::ReadOwner, File::WriteOwner, File::ExeOwner, File::ReadUser, File::WriteUser, File::ExeUser,
        File::ReadGroup, File::WriteGroup, File::ExeGroup, File::ReadOther, File::WriteOther, File::ExeOther
    };

    static const QFile::OpenModeFlag qtOpenModes[] =
    {
        QIODevice::ReadOnly, QIODevice::WriteOnly, QIODevice::ReadWrite, QIODevice::Append, QIODevice::Truncate,
        QIODevice::Text, QIODevice::Unbuffered
    };

    static const File::OpenMode myOpenModes[] =
    {
        File::ReadOnly, File::WriteOnly, File::ReadWrite, File::Append, File::Truncate, File::Text, File::Unbuffered
    };

    File::File(const QString& name)
    {
        _file.setFileName(name);
    }

    QScriptValue File::constructor(QScriptContext * context, QScriptEngine * engine)
    {
        if (context->isCalledAsConstructor() && context->argumentCount() > 0)
        {
            return engine->newQObject(new File(context->argument(0).toString()), QScriptEngine::ScriptOwnership);
        }

        return engine->undefinedValue();
    }

    bool File::atEnd()
    {
        return _file.atEnd();
    }

    bool File::canReadLine()
    {
        return _file.canReadLine();
    }

    void File::close()
    {
        _file.close();
    }

    qint64 File::getBytesAvailable()
    {
        return _file.bytesAvailable();
    }

    qint64 File::getBytesToWrite()
    {
        return _file.bytesToWrite();
    }

    bool File::copy(const QString & newName)
    {
        return _file.copy(newName);
    }

    File::FileError File::getError()
    {
        QFile::FileError fileError = _file.error();

        switch (fileError)
        {
            case QFile::NoError:
                return File::NoError;

            case QFile::ReadError:
                return File::ReadError;

            case QFile::WriteError:
                return File::WriteError;

            case QFile::FatalError:
                return File::FatalError;

            case QFile::ResourceError:
                return File::ResourceError;

            case QFile::OpenError:
                return File::OpenError;

            case QFile::AbortError:
                return File::AbortError;

            case QFile::TimeOutError:
                return File::TimeOutError;

            case QFile::RemoveError:
                return File::RemoveError;

            case QFile::RenameError:
                return File::RenameError;

            case QFile::PositionError:
                return File::PositionError;

            case QFile::ResizeError:
                return File::ResizeError;

            case QFile::PermissionsError:
                return File::PermissionsError;

            case QFile::CopyError:
                return File::CopyError;

            default:
                return File::UnspecifiedError;
        }
    }

    QString File::getErrorString()
    {
        return _file.errorString();
    }

    bool File::exists()
    {
        return _file.exists();
    }

    bool File::flush()
    {
        return _file.flush();
    }

    QString File::getFileName()
    {
        return _file.fileName();
    }

    File::OpenMode File::getOpenMode()
    {
        QFile::OpenMode qtMode = _file.openMode();
        File::OpenModeFlags myMode = File::NotOpen;

        for (unsigned int i = 0; i < (sizeof(qtOpenModes) / sizeof(QFile::OpenModeFlag)); i++)
        {
            if (qtMode.testFlag(qtOpenModes[i]))
            {
                myMode |= myOpenModes[i];
            }
        }

        return static_cast<File::OpenMode>(static_cast<int>(myMode));
    }

    File::Permissions File::getPermissions()
    {
        return File::getPermissions(_file.permissions());
    }

    qint64 File::getPosition()
    {
        return _file.pos();
    }

    qint64 File::getSize()
    {
        return _file.size();
    }

    bool File::isOpen()
    {
        return _file.isOpen();
    }

    bool File::isReadable()
    {
        return _file.isReadable();
    }

    bool File::isSequential()
    {
        return _file.isSequential();
    }

    bool File::isTextModeEnabled()
    {
        return _file.isTextModeEnabled();
    }

    bool File::isWritable()
    {
        return _file.isWritable();
    }

    bool File::link(const QString& linkName)
    {
        return _file.link(linkName);
    }

    bool File::open(File::OpenMode mode)
    {
        QFile::OpenMode qtMode = QFile::NotOpen;
        File::OpenModeFlags myMode = mode;

        for (unsigned int i = 0; i < (sizeof(myOpenModes) / sizeof(File::OpenMode)); i++)
        {
            if (myMode.testFlag(myOpenModes[i]))
            {
                qtMode |= qtOpenModes[i];
            }
        }

        return _file.open(qtMode);
    }

    QString File::peek(qint64 maxSize)
    {
        return _file.peek(maxSize).constData();
    }

    QString File::read(qint64 maxSize)
    {
        return _file.read(maxSize).constData();
    }

    QString File::readAll()
    {
        return _file.readAll().constData();
    }

    QString File::readLine(qint64 maxSize)
    {
        return _file.readLine(maxSize).constData();
    }

    bool File::reset()
    {
        return _file.reset();
    }

    bool File::remove()
    {
        return _file.remove();
    }

    bool File::rename(const QString& newName)
    {
        return _file.rename(newName);
    }

    bool File::resize(qint64 sz)
    {
        return _file.resize(sz);
    }

    bool File::seek(qint64 pos)
    {
        return _file.seek(pos);
    }

    bool File::setPermissions(File::Permissions permissions)
    {
        return _file.setPermissions(File::getQPermissions(permissions));
    }

    void File::setTextModeEnabled(bool enabled)
    {
        _file.setTextModeEnabled(enabled);
    }

    QString File::symLinkTarget()
    {
        return _file.symLinkTarget();
    }

    void File::unsetError()
    {
        _file.unsetError();
    }

    qint64 File::write(const QString& data)
    {
        QByteArray array;

        array.append(data);

        return _file.write(array);
    }

    File::Permissions File::getPermissions(QFile::Permissions permissions)
    {
        File::Permissions myPerms;

        for (unsigned int i = 0; i < (sizeof(qtPermissions) / sizeof(QFile::Permission)); i++)
        {
            if (permissions.testFlag(qtPermissions[i]))
            {
                myPerms |= myPermissions[i];
            }
        }

        return myPerms;
    }

    QFile::Permissions File::getQPermissions(File::Permissions permissions)
    {
        QFile::Permissions qtPerms;

        for (unsigned int i = 0; i < (sizeof(qtPermissions) / sizeof(QFile::Permission)); i++)
        {
            if (permissions.testFlag(myPermissions[i]))
            {
                qtPerms |= qtPermissions[i];
            }
        }

        return qtPerms;
    }
}
