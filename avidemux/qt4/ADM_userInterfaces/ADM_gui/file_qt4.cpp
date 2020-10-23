
// C++ Interface: 
//
// Description: 
//
//
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include <QFileDialog>

#include "ADM_default.h"
#include "ADM_toolkitQt.h"

#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"

#include "prefs.h"
#include "ADM_last.h"
#include "ADM_script2/include/ADM_script.h"

extern QWidget *QuiMainWindows;
static IScriptEngine *tempEngine;

static QWidget *fileSelGetParent(void)
{
    QWidget *parent=qtLastRegisteredDialog();
    if(!parent)
        parent=QuiMainWindows;
    return parent;
}

/**
 * \fn fileSelWriteInternal
 */
static int fileSelWriteInternal(const char *label, char *target, uint32_t max, const char *location, const char *ext)
{
    QString str,outputPath,outputExt=QString("");
    QString fileName,dot=QString(".");
    QString separator = QString("/");
    QString filterFile=QString::fromUtf8(QT_TRANSLATE_NOOP("qfile","All files (*.*)"));
    QFileDialog::Options opts = 0;
    bool doFilter = !!(ext && strlen(ext));
    bool isProject=false;
    int extSize=1;

    if(doFilter)
    {
        outputExt = dot+QString(ext);
        extSize+=strlen(ext);
        for(int i=0; i < getScriptEngines().size(); i++)
        {
            tempEngine = getScriptEngines()[i];
            std::string dext = tempEngine->defaultFileExtension();
            if(!dext.empty() && dext == ext)
            {
                isProject=true;
                break;
            }
        }
    }

    if(location)
        outputPath = QFileInfo(location).path();

    if(!location || !QDir(outputPath).exists())
    {
        bool lastReadAsTarget=false;
        prefs->get(FEATURES_USE_LAST_READ_DIR_AS_TARGET,&lastReadAsTarget);
        std::string lastFolder;
        if(!lastReadAsTarget)
        {
            if(!isProject)
            {
                admCoreUtils::getLastWriteFolder(lastFolder);
            }else
            {
                admCoreUtils::getLastProjectWriteFolder(lastFolder);
            }
            if(!lastFolder.size())
            {
                if(!isProject)
                {
                    admCoreUtils::getLastReadFolder(lastFolder);
                }else
                {
                    admCoreUtils::getLastProjectReadFolder(lastFolder);
                }
            }
        }else
        {
            admCoreUtils::getLastReadFolder(lastFolder);
        }

        if(lastFolder.size())
        {
            outputPath = QFileInfo(QString::fromUtf8(lastFolder.c_str())).path();
        }
    }

    /* LASTDIR may have gone; then use the user's homedir instead */
    if(outputPath.isEmpty() || !QDir(outputPath).exists())
        outputPath = QDir::homePath();

    QString inputBaseName = QString("");
    std::string lastRead;
    admCoreUtils::getLastReadFile(lastRead);
    if(lastRead.size())
    {
        inputBaseName = QFileInfo(QString::fromUtf8(lastRead.c_str())).completeBaseName();
        str = outputPath+separator+inputBaseName+outputExt;

        if(str==QString::fromUtf8(lastRead.c_str()))
        { // try to avoid name collision when saving in the same directory as the currently loaded video
            str = outputPath+separator+inputBaseName+QString("_edit")+outputExt;
        }
    }else
    {
        str = QDir::homePath()+separator+QString("out")+dot+outputExt;
    }

    if(doFilter)
    {
        filterFile=QString(ext)+QString::fromUtf8(QT_TRANSLATE_NOOP("qfile"," files (*."))+QString(ext)+QString(");;")+filterFile;
    }

#ifndef __APPLE__
    opts = QFileDialog::DontConfirmOverwrite; // doesn't work on macOS, wtf?
#endif

    fileName = QFileDialog::getSaveFileName(fileSelGetParent(),
                    QString::fromUtf8(label),  // caption
                    str,    // folder
                    filterFile,   // filter
                    NULL, // selected filter
                    opts);

    int len = strlen(fileName.toUtf8().constData());
    if(!len || len >= max) return 0;

    // Check if we need to add an extension....
    if(doFilter)
    {                     
        if(!strstr(fileName.toUtf8().constData(),".")) //FIXME
        {
            fileName=fileName+QString(".")+QString(ext);
            len+=extSize;
        }
    }

    if(opts & QFileDialog::DontConfirmOverwrite)
    {
        QFile newFile(fileName);
        if(newFile.exists())
        {
            QFileInfo fileInfo(newFile);
            QString q=QString::fromUtf8(QT_TRANSLATE_NOOP("qfile","Overwrite file "))+fileInfo.fileName()+QString("?");
            // Show the dialog even in silent mode or if the user has disabled alerts.
            if(!GUI_Question(q.toUtf8().constData(),true))
            {
                return 0;
            }
        }
    }

    if(len >= max)
    {
        ADM_warning("Path length %d exceeds max %d\n",len,max-1);
        return 0;
    }

    strncpy(target,fileName.toUtf8().constData(),len);
    target[len]='\0';

    if(!isProject)
    {
        admCoreUtils::setLastWriteFolder( std::string(fileName.toUtf8().constData()));
    }else
    {
        admCoreUtils::setLastProjectWriteFolder( std::string(fileName.toUtf8().constData()));
    }
    return len;
}


/**
 * \fn fileSelReadInternal
 */
static int fileSelReadInternal(const char *label, char *target, uint32_t max, const char *location, const char *ext)
{
    QString str;
    QString fileName,dot=QString(".");
    QString filterFile=QString::fromUtf8(QT_TRANSLATE_NOOP("qfile","All files (*.*)"));
    bool doFilter = !!(ext && strlen(ext));
    bool isProject=false;
    QFileDialog::Options opts = 0;

    if(doFilter)
    {
        for(int i=0; i < getScriptEngines().size(); i++)
        {
            tempEngine = getScriptEngines()[i];
            std::string dext = tempEngine->defaultFileExtension();
            if(!dext.empty() && dext == ext)
            {
                isProject=true;
                break;
            }
        }
    }

    if(location)
        str = QFileInfo(location).path();

    if(!location || !QDir(str).exists())
    {
        std::string lastFolder;
        if(!isProject)
        {
            admCoreUtils::getLastReadFolder(lastFolder);
        }else
        {
            admCoreUtils::getLastProjectReadFolder(lastFolder);
        }

        if (lastFolder.size())
        {
            str = QFileInfo(QString::fromUtf8(lastFolder.c_str())).path();
            /* LASTDIR may have gone; then use the user's homedir instead */
            if (!QDir(str).exists())
                str = QDir::homePath();
        }else
        {
            str = QDir::homePath();
        }
    }

    if(doFilter)
    {
        filterFile=QString(ext)+QString::fromUtf8(QT_TRANSLATE_NOOP("qfile"," files (*."))+QString(ext)+QString(");;")+filterFile;
    }
    fileName = QFileDialog::getOpenFileName(fileSelGetParent(),
                                QString::fromUtf8(label),  // caption
                                str,    // folder
                                filterFile,   // filter
                                NULL,   // selected filter
                                opts);

    int len = strlen(fileName.toUtf8().constData());
    if(!len || len >= max) return 0;

    strncpy(target,fileName.toUtf8().constData(),len);
    target[len]='\0';

    if(!isProject)
    {
        admCoreUtils::setLastReadFolder(std::string(fileName.toUtf8().constData()));
    }else
    {
        admCoreUtils::setLastProjectReadFolder(std::string(fileName.toUtf8().constData()));
    }
    return len;
}        

/*****************************************************/

namespace ADM_QT4_fileSel
{

#if defined(__APPLE__)
 #define MAX_LEN 1024
#else
 #define MAX_LEN 4096
#endif
void GUI_FileSelRead(const char *label, char **name)
{
    char *fn=(char *)ADM_alloc(MAX_LEN);
    if(fileSelReadInternal(label,fn,MAX_LEN,NULL,NULL))
        *name=ADM_strdup(fn);
    else
        *name=NULL;
    ADM_dealloc(fn);
}

void GUI_FileSelWrite(const char *label, char **name)
{
    char *fn=(char *)ADM_alloc(MAX_LEN);
    if(fileSelWriteInternal(label,fn,MAX_LEN,NULL,NULL))
        *name=ADM_strdup(fn);
    else
        *name=NULL;
    ADM_dealloc(fn);
}

void GUI_FileSelRead(const char *label, SELFILE_CB cb)
{
        char *name;

        GUI_FileSelRead(label, &name);

        if (name)
        {
                cb(name); 
                ADM_dealloc(name);
        }
}        
/**
 * \fn GUI_FileSelWrite
 * @param label
 * @param cb
 */
void GUI_FileSelWrite(const char *label, SELFILE_CB cb)
{
        char *name;

        GUI_FileSelWrite(label, &name);

        if (name)
        {
                cb(name); 
                ADM_dealloc(name);
        }
}

/**
          \fn FileSel_SelectWrite
          \brief select file, write mode
          @param title window title
          @param target where to store result
          @param max Max buffer size in bytes
          @param source Original value
          @param extension Filter for the view
          @return 1 on success, 0 on failure
*/
uint8_t FileSel_SelectWrite(const char *title, char *target, uint32_t max, const char *source, const char *extension)
{
    if(fileSelWriteInternal(title,target,max,source,extension))
        return 1;
    return 0;
}

/**
          \fn FileSel_SelectRead
          \brief select file, read mode
          @param title window title
          @param target where to store result
          @param max Max buffer size in bytes
          @param source Original value
          @param extension Filter for the view
          @return 1 on success, 0 on failure
*/
uint8_t FileSel_SelectRead(const char *title, char *target, uint32_t max, const char *source, const char *extension)
{
    if(fileSelReadInternal(title,target,max,source,extension))
        return 1;
    return 0;
}

/**
          \fn FileSel_SelectDir
          \brief select directory
          @param title window title
          @param target where to store result
          @param max Max buffer size in bytes
          @param source Original value
          @return 1 on success, 0 on failure
*/
uint8_t FileSel_SelectDir(const char *title, char *target, uint32_t max, const char *source, const char *extension)
{
        QString fileName;
        QFileDialog::Options options = QFileDialog::ShowDirsOnly;

        fileName = QFileDialog::getExistingDirectory(fileSelGetParent(), title, source, options);

        if (!fileName.isNull())
        {
                const char *s = fileName.toUtf8().constData();
                strncpy(target, s, max);

                return 1;
        }

        return 0;
}
void GUI_FileSelWriteExtension(const char *label, const char *extension,SELFILE_CB cb)
{
    char *name=NULL;
    char *fn=(char *)ADM_alloc(MAX_LEN);
    if(fileSelWriteInternal(label,fn,MAX_LEN,NULL,extension))
        name=ADM_strdup(fn);
    ADM_dealloc(fn);
    if(name)
    {
        cb(name);
        ADM_dealloc(name);
    }
}
void GUI_FileSelReadExtension(const char *label, const char *extension,SELFILE_CB cb)
{
    char *name=NULL;
    char *fn=(char *)ADM_alloc(MAX_LEN);
    if(fileSelReadInternal(label,fn,MAX_LEN,NULL,extension))
        name=ADM_strdup(fn);
    ADM_dealloc(fn);
    if(name)
    {
        cb(name);
        ADM_dealloc(name);
    }
}        
/**
         * 
*/
void init(void)
{
        // Nothing special to do for QT4 fileselector
}
} // End of nameSpace

static DIA_FILESEL_DESC_T Qt4FileSelDesc =
{
        ADM_QT4_fileSel::init,
        ADM_QT4_fileSel::GUI_FileSelRead,
        ADM_QT4_fileSel::GUI_FileSelWrite,
        ADM_QT4_fileSel::GUI_FileSelRead,
        ADM_QT4_fileSel::GUI_FileSelWrite,
        ADM_QT4_fileSel::FileSel_SelectRead,
        ADM_QT4_fileSel::FileSel_SelectWrite,
        ADM_QT4_fileSel::FileSel_SelectDir,
        ADM_QT4_fileSel::GUI_FileSelWriteExtension,
        ADM_QT4_fileSel::GUI_FileSelReadExtension
};

// Hook our functions
void initFileSelector(void)
{
        DIA_fileSelInit(&Qt4FileSelDesc);
}

