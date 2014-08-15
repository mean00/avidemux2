
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
#include <QtGui/qfiledialog.h>

#include "ADM_default.h"


#include "DIA_fileSel.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"

#include "prefs.h"

namespace ADM_QT4_fileSel
{
	/**
	  \fn GUI_FileSelWrite (const char *label, char **name, uint32_t access) 
	  \brief Ask for a file for reading (access=0) or writing (access=1)
	*/
	void GUI_FileSelSelect(const char *label, char **name, uint32_t access)
	{
		char *tmpname = NULL;
		QString str ;
		options pref_entry = LASTFILES_LASTDIR_READ;

		*name = NULL;

		if (access)
			pref_entry = LASTFILES_LASTDIR_WRITE;

		if (prefs->get(pref_entry,&tmpname))
		{
                        str = QFileInfo(QString::fromUtf8(tmpname)).path();


			/* LASTDIR may have gone; then do nothing and use current dir instead (implied) */
			if (!QDir(str).exists())
                                str.clear();
		}

		QString fileName;
		QFileDialog::Options options = 0;


		if (access)
			fileName = QFileDialog::getSaveFileName(NULL, label, str, NULL, NULL, options);
		else
			fileName = QFileDialog::getOpenFileName(NULL, label, str, NULL, NULL, options);

		if (!fileName.isNull() )
		{
			*name = ADM_strdup(fileName.toUtf8().constData());
			prefs->set(pref_entry, (char *)*name);
		}

	}

	
/**
	  \fn GUI_FileSelWrite (const char *label, char **name, uint32_t access) 
	  \brief Ask for a file for reading (access=0) or writing (access=1)
	*/
	void GUI_FileSelSelectWriteInternal(const char *label, const char *ext, char **name)
	{
		char *tmpname = NULL;
                *name = NULL;		
                QString str ;
                QString fileName,dot=QString(".");
                QString filterFile=QString("*");
                bool doFilter = !!(ext && strlen(ext));
                QFileDialog::Options opts;
                
                //printf("Do filer=%d\n",(int)doFilter);
                if (prefs->get(LASTFILES_LASTDIR_WRITE,&tmpname))
		{
                        
                        str = QFileInfo(QString::fromUtf8(tmpname)).path();
                	/* LASTDIR may have gone; then do nothing and use current dir instead (implied) */
			if (!QDir(str).exists())
                                str.clear();
		}
		
                if(doFilter)
                {
                    std::string f=std::string("*.")+std::string(ext);
                    filterFile=QString(f.c_str());
                    //printf("Filter = %s\n",f.c_str());
                }
                fileName = QFileDialog::getSaveFileName(NULL, 
                                        label,  // caption
                                        str,    // folder
                                        NULL,   // filter
                                        NULL,   // selected filter
                                        opts);
                
		if (fileName.isNull() ) return;
                prefs->set(LASTFILES_LASTDIR_WRITE, fileName.toUtf8().constData());
                *name = ADM_strdup(fileName.toUtf8().constData());
                
                // Check if we need to add an extension....
                if(doFilter)
                {                     
                        if(!strstr(*name,"."))
                        {
                            char *newName=(char *)ADM_alloc(strlen(*name)+4+strlen(ext));
                            strcpy(newName,*name);
                            strcat(newName,".");
                            strcat(newName,ext);
                            ADM_dezalloc(*name);
                            *name=newName;
                            //printf("New name =<%s>\n",*name);
                            QString newFileName=fileName+QString(".")+QString(ext);
                            //printf("New filename =<%s>\n",newFileName.toUtf8().constData());
                            QFile newFile(newFileName);
                            if(newFile.exists())
                            {
                                std::string q=std::string("Overwrite file ")+std::string(newFile.name());
                                if(!GUI_Question(q.c_str()))
                                    return;
                            }
                        }
                }
	}
        void GUI_FileSelRead(const char *label, char **name)
	{
		GUI_FileSelSelect(label, name, 0);
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
			const char *s = fileName.toUtf8().constData();
			strncpy(target, s, max);

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
			const char *s = fileName.toUtf8().constData();
			strncpy(target, s, max);

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
            printf("Extension is : %s\n",extension);
            GUI_FileSelSelectWriteInternal(label, extension, &name);
            if(name)
                cb(name);
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
        ADM_QT4_fileSel::GUI_FileSelWriteExtension
};

// Hook our functions
void initFileSelector(void)
{
	DIA_fileSelInit(&Qt4FileSelDesc);
}

