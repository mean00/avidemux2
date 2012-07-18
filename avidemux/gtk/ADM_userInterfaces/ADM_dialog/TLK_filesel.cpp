/***************************************************************************
                          TLK_filesel.cpp  -  description
                             -------------------
	New version of file selector

    begin                : Fri Sep 20 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_toolkitGtk.h"
#include "DIA_coreToolkit.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "ADM_default.h"
//#include "avi_vars.h"

#include "DIA_fileSel.h"
#include "DIA_coreUI_internal.h"
#include "DIA_coreToolkit.h"
#include "prefs.h"

#define TH_READ 1
#define TH_WRITE 2

extern void FileSel_ReadWrite(SELFILE_CB *cb, int rw, const char *name, const char *actual_workbench_file);

namespace ADM_GTK_fileSel 
{
static void GUI_FileSel(const char *label, SELFILE_CB cb, int rw, char **name = NULL);
uint8_t initFileSelector(void);

static GtkFileFilter *filter_avi = NULL, *filter_mpeg = NULL, *filter_image = NULL, *filter_all = NULL;
static uint8_t setFilter(GtkWidget *dialog);

/**
\fn FileSel_SelectRead(const char *title,char *target,uint32_t max, const char *source)
\brief allow to select a file
@return 0 on failure, 1 on success
@param title : window title 
@param target : where to copy the result (must be allocated by caller)
@param max : Max # of bytes that target can hold
@param source : where we start from
*/
uint8_t FileSel_SelectRead(const char *title, char *target, uint32_t max, const char *source)
{
	GtkWidget *dialog;
	uint8_t ret = 0;
	gchar *selected_filename, last;
	char *dupe = NULL, *tmpname = NULL;
	DIR *dir = NULL;

	dialog = gtk_file_chooser_dialog_new("Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,	NULL);

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_ACCEPT,
										GTK_RESPONSE_CANCEL,
										-1);

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	initFileSelector();
	setFilter(dialog);
	gtk_register_dialog(dialog);

	if (source && *source)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), (gchar*)source);
	else	//use pref
		if (prefs->get(LASTFILES_LASTDIR_READ, (char**)&tmpname))
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), (gchar*)tmpname);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		selected_filename = (gchar *)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if (strlen(selected_filename))
		{
			last = selected_filename[strlen(selected_filename) - 1]; 

			if (last == '/' || last =='\\' )
			{
				GUI_Error_HIG(QT_TR_NOOP("Cannot open directory as a file"), NULL);
				return 0;
			}
			else
			{
				// Check we can read it ..
				FILE *fd;

				fd = fopen(selected_filename,"rb");

				if (fd)
				{
					fclose(fd);
					strncpy(target, (char*)selected_filename, max);
					// Finally we accept it :)
					ret = 1;
				}
			}
		}
	}

	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);

	return ret;
}
/**
\fn FileSel_SelectWrite(const char *title,char *target,uint32_t max, const char *source)
\brief allow to select a file
@return 0 on failure, 1 on success
@param title : window title 
@param target : where to copy the result (must be allocated by caller)
@param max : Max # of bytes that target can hold
@param source : where we start from
*/
uint8_t FileSel_SelectWrite(const char *title, char *target, uint32_t max, const char *source)
{
	GtkWidget *dialog;
	uint8_t ret = 0;
	gchar *selected_filename;
	gchar last;
	char *dupe = NULL, *tmpname = NULL;
	DIR *dir = NULL;

	dialog = gtk_file_chooser_dialog_new("Write to File", NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_ACCEPT,
										GTK_RESPONSE_CANCEL,
										-1);

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	initFileSelector();
	setFilter(dialog);
	gtk_register_dialog(dialog);

	if (source && *source)
	{
#if 0
		// well, this is what they say to do, but then you can't easily edit the
		// name...

		// the following sequence is per GTK docs for gtk_file_chooser_set_filename()
		if (access (source, W_OK) == 0) // if file exists
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),(gchar *)source);
		else // new file
#endif
		{
			dupe = ADM_PathCanonize(source);
			ADM_PathStripName(dupe);

			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), (gchar*)dupe);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
				(gchar*)(source + strlen(dupe)));

			delete [] dupe;
		}
	}
	else	//use pref
	{
		if (prefs->get(LASTFILES_LASTDIR_WRITE,(char **)&tmpname))
		{
			dupe = ADM_PathCanonize(tmpname);

			if (dir = opendir(dupe))
			{
				closedir(dir);
				gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), (gchar*)tmpname);
			}

			delete [] dupe;
		}
	}
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		selected_filename = (gchar*)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if (strlen(selected_filename))
		{
			last = selected_filename[strlen(selected_filename) - 1];

			if (last == '/' || last =='\\')
			{
				GUI_Error_HIG(QT_TR_NOOP("Cannot open directory as a file"), NULL);
				return 0;
			}
			else
			{
				strncpy(target, (char*)selected_filename, max);
				// Finally we accept it :)
				ret = 1;
			}
		}
	}

	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);

	return ret;
}

/**
\fn FileSel_SelectDir(const char *title,char *target,uint32_t max, const char *source)
\brief allow to select a directory
@return 0 on failure, 1 on success
@param title : window title 
@param target : where to copy the result (must be allocated by caller)
@param max : Max # of bytes that target can hold
@param source : where we start from
*/
uint8_t FileSel_SelectDir(const char *title, char *target, uint32_t max, const char *source)
{
	GtkWidget *dialog;
	uint8_t ret = 0;
	gchar *selected_filename;
	gchar last;
	char *dupe = NULL, *tmpname = NULL;
	DIR *dir = NULL;

	dialog = gtk_file_chooser_dialog_new("Open File", NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_ACCEPT,
										GTK_RESPONSE_CANCEL,
										-1);

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_register_dialog(dialog);

	/* Set default dir if provided ..*/
	if (source)
	{
		dupe = ADM_PathCanonize(source);
		ADM_PathStripName(dupe);

		if (dir = opendir(dupe))
		{
			closedir(dir);
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), (gchar*)source);
		}

		delete [] dupe;
	}
	else	//use pref
	{
		if (prefs->get(LASTFILES_LASTDIR_READ, (char **)&tmpname))
		{
			dupe = ADM_PathCanonize(tmpname);
			ADM_PathStripName(dupe);

			if (dir = opendir(dupe))
			{
				closedir(dir);
				gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), (gchar*)dupe);
			}

			delete [] dupe;
		}
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		selected_filename = (gchar*)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if (strlen(selected_filename))  /* Nothing selected */
		{
			/* Check it is a dir ...*/
			printf("<%s>\n", selected_filename);
			strncpy(target, selected_filename, max);
			target[max-1] = 0;
			ret = 1;
		}
	}

	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);

	return ret;
}

void GUI_FileSelWrite(const char *label, SELFILE_CB cb)
{
	/* Create the selector */
	GUI_FileSel(label, cb, 1);
}

void GUI_FileSelRead(const char *label, SELFILE_CB cb)
{
	/* Create the selector */
	GUI_FileSel(label, cb, 0);
}
void GUI_FileSelRead(const char *label, char * * name)
{
	/* Create the selector */
	GUI_FileSel(label, NULL, 0, name);
}
void GUI_FileSelWrite(const char *label, char * * name)
{
	/* Create the selector */
	GUI_FileSel(label, NULL, 1, name);
}

void GUI_FileSel(const char *label, SELFILE_CB cb, int rw,char **rname)
{
	/* Create the selector */
	GtkWidget *dialog;
	char *name = NULL;
	char *tmpname;
	gchar *selected_filename;
	uint8_t res;

	if (rname)
		*rname = NULL;

	if (rw)
		dialog = gtk_file_chooser_dialog_new ("Save", NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	else
		dialog = gtk_file_chooser_dialog_new ("Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
											GTK_RESPONSE_ACCEPT,
											GTK_RESPONSE_CANCEL,
											-1);

	initFileSelector();
	setFilter(dialog);

	gtk_window_set_title (GTK_WINDOW(dialog), label);
	gtk_register_dialog(dialog);

	if (rw)
		res = prefs->get(LASTFILES_LASTDIR_WRITE,(char **)&tmpname);
	else
		res = prefs->get(LASTFILES_LASTDIR_READ,(char **)&tmpname);

	if (res)
	{
		DIR *dir;
		char *str = ADM_PathCanonize(tmpname);

		ADM_PathStripName(str);

		/* LASTDIR may have gone; then do nothing and use current dir instead (implied) */
		if (dir = opendir(str))
		{
			closedir(dir);
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),(gchar *)str);
		}

		delete [] str;
	}

	ADM_dealloc(tmpname);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		selected_filename = (gchar *)gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

#ifdef _WIN32
		if (*(selected_filename + strlen(selected_filename) - 1) == '\\'){
#else
		if (*(selected_filename + strlen(selected_filename) - 1) == '/'){
#endif
			GUI_Error_HIG(QT_TR_NOOP("Cannot open directory as a file"), NULL);
		}
		else
		{
			name = ADM_strdup(selected_filename);

			char *str = ADM_PathCanonize(name);

			ADM_PathStripName(str);

			if (rw)
				prefs->set(LASTFILES_LASTDIR_WRITE, (char*)str);
			else
				prefs->set(LASTFILES_LASTDIR_READ, (char*)str);

			delete [] str;
		}
	}

	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);

	// CYB 2005.02.23
	if (cb)
	{
#warning fixme
        const char *leak=NULL;
		FileSel_ReadWrite(cb, rw, name, leak);
		ADM_dealloc(name);
	}
	else
		*rname = name;
}


/* Mean:It seems it is attached to the dialog & destroyed with it
As it leads to crash if we don't recreate them each time....*/
uint8_t initFileSelector(void)
{
#define ADD_PAT(x,y) gtk_file_filter_add_pattern(x,"*."#y);

	filter_avi=gtk_file_filter_new();
	gtk_file_filter_set_name(filter_avi, "AVI (*.avi)");
	ADD_PAT(filter_avi, avi);
	ADD_PAT(filter_avi, AVI);

	filter_mpeg=gtk_file_filter_new();
	gtk_file_filter_set_name(filter_mpeg, "MPEG (*.m*,*.vob)");
	ADD_PAT(filter_mpeg, [mM][12][Vv]);
	ADD_PAT(filter_mpeg, [Mm][pP][gG]);
	ADD_PAT(filter_mpeg, [Vv][Oo][Bb]);
	ADD_PAT(filter_mpeg, ts);
	ADD_PAT(filter_mpeg, TS);

	filter_image = gtk_file_filter_new();

	gtk_file_filter_set_name(filter_image, QT_TR_NOOP("Images"));
	ADD_PAT(filter_image, png);
	ADD_PAT(filter_image, bmp);
	ADD_PAT(filter_image, jpg);

	ADD_PAT(filter_image, PNG);
	ADD_PAT(filter_image, BMP);
	ADD_PAT(filter_image, JPG);

	filter_all = gtk_file_filter_new();

	gtk_file_filter_set_name(filter_all, QT_TR_NOOP("All"));
	gtk_file_filter_add_pattern(filter_all, "*");

	return 1;
}

uint8_t setFilter(GtkWidget *dialog)
{
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_all);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_avi);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_mpeg);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_image);

	return 1;
}
/*************/
void init(void)
{
	initFileSelector();
}
} // End of nameSpace
static DIA_FILESEL_DESC_T GtkFileSelDesc=
{
	ADM_GTK_fileSel::init,
	ADM_GTK_fileSel::GUI_FileSelRead,
	ADM_GTK_fileSel::GUI_FileSelWrite,
	ADM_GTK_fileSel::GUI_FileSelRead,
	ADM_GTK_fileSel::GUI_FileSelWrite,
	ADM_GTK_fileSel::FileSel_SelectWrite,
	ADM_GTK_fileSel::FileSel_SelectRead,
	ADM_GTK_fileSel::FileSel_SelectDir
};

// Hook our functions
void initFileSelector(void)
{
	DIA_fileSelInit(&GtkFileSelDesc);
}

//EOF
