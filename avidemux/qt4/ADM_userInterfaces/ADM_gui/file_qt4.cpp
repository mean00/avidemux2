
// C++ Interface: 
//
// Description: 
//
//
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <dirent.h>

#include <QtCore/QVariant>
#include <qfiledialog.h>

#include "ADM_default.h"


#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"

#include "prefs.h"
#include "ADM_last.h"

namespace ADM_QT4_fileSel
{


/**
          \fn GUI_FileSelWrite (const char *label, char **name, uint32_t access) 
          \brief Ask for a file for reading (access=0) or writing (access=1)
        */
static	void GUI_FileSelSelectWriteInternal(const char *label, const char *ext, char **name)
{
    *name = NULL;		
    QString str ;
    QString fileName,dot=QString(".");
    QString filterFile=QString(QT_TRANSLATE_NOOP("qfile","All files (*.*)"));
    bool doFilter = !!(ext && strlen(ext));
    QFileDialog::Options opts;
    int extSize=1;

    if(doFilter)
    {
        extSize+=strlen(ext);
    }
    //printf("Do filer=%d\n",(int)doFilter);
    std::string lastFolder;
    admCoreUtils::getLastWriteFolder(lastFolder);
    if (lastFolder.size())
    {
        QString outputPath = QFileInfo(QString::fromUtf8(lastFolder.c_str())).path();

        char *tmpinputname = NULL;
        QString inputBaseName = QString("");
        std::string lastRead;
        admCoreUtils::getLastReadFolder(lastRead);
        if (lastRead.size())
        {
            inputBaseName = QFileInfo(QString::fromUtf8(lastRead.c_str())).completeBaseName();
        }

        QString outputExt = QString("");
        if (doFilter)
        {
            outputExt = dot+QString(ext);
        }
        QString separator = QString("/");
        str = outputPath+separator+inputBaseName+outputExt;

        /* LASTDIR may have gone; then do nothing and use current dir instead (implied) */
        if (!QDir(outputPath).exists())
                str.clear();
    }

    if(doFilter)
    {
        filterFile=QString(ext)+QString(QT_TRANSLATE_NOOP("qfile"," files (*."))+QString(ext)+QString(");;")+filterFile;
    }
    fileName = QFileDialog::getSaveFileName(NULL, 
                    label,  // caption
                    str,    // folder
                    filterFile,   // filter
                    NULL,QFileDialog::DontConfirmOverwrite);   // selected filter


    if (fileName.isNull() ) return;

    *name=(char *)ADM_alloc(  strlen(fileName.toUtf8().constData())+extSize);
    strcpy(*name,fileName.toUtf8().constData());

    // Check if we need to add an extension....
    if(doFilter)
    {                     
        if(!strstr(*name,"."))
        {
            strcat(*name,"."); strcat(*name,ext);

            fileName=fileName+QString(".")+QString(ext);
        }
    }
    QFile newFile(fileName);
    if(newFile.exists())
    {
        QFileInfo fileInfo(newFile);
        QString q=QString(QT_TRANSLATE_NOOP("qfile","Overwrite file "))+fileInfo.fileName()+QString("?");
        if(!GUI_Question(q.toUtf8().constData()))
        {
            ADM_dezalloc(*name);
            *name=NULL;
            return;
        }
    }
    admCoreUtils::setLastWriteFolder( std::string(fileName.toUtf8().constData()));
}


/**
          \fn GUI_FileSelWrite (const char *label, char **name, uint32_t access) 
          \brief Ask for a file for reading (access=0) or writing (access=1)
        */
static	void GUI_FileSelSelectReadInternal(const char *label, const char *ext, char **name)
{
        *name = NULL;		
        QString str ;
        QString fileName,dot=QString(".");
        QString filterFile=QString(QT_TRANSLATE_NOOP("qfile","All files (*.*)"));
        bool doFilter = !!(ext && strlen(ext));
        QFileDialog::Options opts;

        //printf("Do filer=%d\n",(int)doFilter);
        std::string lastFolder;
        admCoreUtils::getLastReadFolder(lastFolder);

        if (lastFolder.size())
        {
                str = QFileInfo(QString::fromUtf8(lastFolder.c_str())).path();
                /* LASTDIR may have gone; then do nothing and use current dir instead (implied) */
                if (!QDir(str).exists())
                        str.clear();
        }

        if(doFilter)
        {
            filterFile=QString(ext)+QString(QT_TRANSLATE_NOOP("qfile"," files (*."))+QString(ext)+QString(");;")+filterFile;
        }
        fileName = QFileDialog::getOpenFileName(NULL, 
                                label,  // caption
                                str,    // folder
                                filterFile,   // filter
                                NULL,   // selected filter
                                opts);

        if (fileName.isNull() ) return;

        *name=ADM_strdup(fileName.toUtf8().constData());
        admCoreUtils::setLastReadFolder(std::string(fileName.toUtf8().constData()));
}        

void GUI_FileSelRead(const char *label, char **name)
{
        GUI_FileSelSelectReadInternal(label, "",name);
}

void GUI_FileSelWrite(const char *label, char **name)
{
        GUI_FileSelSelectWriteInternal(label,"",name);
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
          @return 1 on success, 0 on failure
*/
uint8_t FileSel_SelectWrite(const char *title, char *target, uint32_t max, const char *source)
{
        QString fileName;
        QFileDialog::Options options = 0;

        fileName=QFileDialog::getSaveFileName(NULL, title, source, NULL, NULL, options);

        if (!fileName.isNull())
        {
            strncpy(target, fileName.toUtf8().constData(), max);
            target[max-1]=0;
            return 1;
        }
        return 0;
}

/**
          \fn FileSel_SelectRead
          \brief select file, read mode
          @param title window title
          @param target where to store result
          @param max Max buffer size in bytes
          @param source Original value
          @return 1 on success, 0 on failure
*/
uint8_t FileSel_SelectRead(const char *title, char *target, uint32_t max, const char *source)
{
        QString fileName;
        QFileDialog::Options options = 0;

        fileName = QFileDialog::getOpenFileName(NULL, title, source, NULL, NULL, options);

        if (!fileName.isNull())
        {
                strncpy(target, fileName.toUtf8().constData(), max);
                target[max-1]=0;
                return 1;
        }
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
uint8_t FileSel_SelectDir(const char *title, char *target, uint32_t max, const char *source)
{
        QString fileName;
        QFileDialog::Options options = QFileDialog::ShowDirsOnly;


        fileName = QFileDialog::getExistingDirectory(NULL, title, source, options);

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
    char *name;
    //printf("Extension is : %s\n",extension);
    GUI_FileSelSelectWriteInternal(label, extension, &name);
    if(name)
    {
        cb(name);
        ADM_dealloc(name);
    }
}
void GUI_FileSelReadExtension(const char *label, const char *extension,SELFILE_CB cb)
{
    char *name;
    //printf("Extension is : %s\n",extension);
    GUI_FileSelSelectReadInternal(label, extension, &name);
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

