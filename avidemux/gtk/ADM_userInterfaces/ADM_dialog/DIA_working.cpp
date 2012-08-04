/***************************************************************************
                          DIA_working.cpp  -  description
                             -------------------
    begin                : Thu Apr 21 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr

	This class deals with the working window

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>

#include "ADM_toolkitGtk.h"
#include "DIA_coreToolkit.h"
#include "ADM_misc.h"

#include "ADM_debugID.h"
#define MODULE_NAME MODULE_CLOCKnTIMELEFT
#include "ADM_debug.h"

#include "DIA_working.h"
#include "ADM_vidMisc.h"

static GtkWidget	*create_dialog1 (void);
static void on_work_abort(GtkWidget * object, gpointer user_data);
static gint on_destroy_abort(GtkWidget * object, gpointer user_data);

static GtkWidget *progressbar1;
static GtkWidget *labelElapsed;
static GtkWidget *labelRemaining;
static GtkWidget *closebutton1;

void on_work_abort(GtkWidget * object, gpointer user_data)
{
DIA_workingBase *dial;
GtkWidget *dialog;

	UNUSED_ARG(object);
	dial=(DIA_workingBase *)user_data;
	dialog=(GtkWidget *)dial->_priv;
        gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	dial->_priv=NULL;
	UI_purge();

};

gint on_destroy_abort(GtkWidget * object, gpointer user_data)
{
	UNUSED_ARG(object);
	UNUSED_ARG(user_data);

	return TRUE;
};
//********************************
class DIA_workingGtk : public DIA_workingBase
{
protected:
        virtual void 	    postCtor( void );
public:
                            DIA_workingGtk( const char *title=NULL );
        virtual		        ~DIA_workingGtk();
            // If returns 1 -> Means aborted
        virtual uint8_t  	update(uint32_t percent);
        virtual uint8_t 	update(uint32_t current,uint32_t total);
        virtual uint8_t  	isAlive (void );
                void        closeDialog(void);
        
};

//********************************
DIA_workingGtk::DIA_workingGtk( const char *title ) : DIA_workingBase(title)
{
	GtkWidget *dialog;

	dialog=create_dialog1();

    gtk_register_dialog(dialog);
	_priv=(void *)dialog;

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	postCtor();
}

void DIA_workingGtk :: postCtor( void )
{
		GtkWidget 	*dialog;

		dialog=(GtkWidget *)_priv;
		//gtk_window_set_modal(GTK_WINDOW(dialog), 1);
		g_signal_connect(closebutton1, "clicked",
		       G_CALLBACK(on_work_abort), (void *) this);

		       // somehow prevent the window from being erased..
		g_signal_connect(dialog, "delete_event",
		       G_CALLBACK(on_destroy_abort), (void *) this);

		gtk_widget_show(dialog);
		UI_purge();
		lastper=0;
		_nextUpdate=0;
}
uint8_t DIA_workingGtk::update(uint32_t percent)
{
	#define GUI_UPDATE_RATE 1000

	if(!_priv) return 1;
	if(!percent) return 0;

	if(percent==lastper)
	{
	   UI_purge();
	   return 0;
	}

	aprintf("DIA_workingGtk::update(%lu) called\n", percent);
	elapsed=_clock.getElapsedMS();

	if(elapsed<_nextUpdate) 
	{
	  UI_purge();
	  return 0;
	}

	_nextUpdate=elapsed+1000;
	lastper=percent;

	GtkWidget *dialog;
	dialog=(GtkWidget *)_priv;

	uint32_t hh,mm,ss,mms;
	char string[9];

	ms2time(elapsed,&hh,&mm,&ss,&mms);
	sprintf(string,"%02d:%02d:%02d",hh,mm,ss);
	gtk_label_set_text(GTK_LABEL(labelElapsed),string);

//	gtk_label_set_text(GTK_LABEL(labelRemaining), ms2timedisplay((uint32_t) floor(((elapsed * 100.) / percent) - elapsed)));
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar1),(gfloat)(percent / 100.));

	sprintf(string, QT_TR_NOOP("%d%%"), percent);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar1), string);

	UI_purge();
	return 0;
}

uint8_t DIA_workingGtk::update(uint32_t cur, uint32_t total)
{
		double d,n;
		uint32_t percent;
		if(!_priv) return 1;

		aprintf("DIA_workingGtk::update(uint32_t %lu,uint32_t %lu) called\n", cur, total);
		if(!total) return 0;

		d=total;
		n=cur;
		n=n*100.;

		n=n/d;

		percent=(uint32_t )floor(n);
		return update(percent);
}

uint8_t DIA_workingGtk::isAlive (void )
{
	if(!_priv) return 0;
	return 1;
}

DIA_workingGtk::~DIA_workingGtk()
{
	closeDialog();
        
}

void DIA_workingGtk::closeDialog( void )
{
	GtkWidget *dialog;

	dialog=(GtkWidget *)_priv;

	if(dialog)
	{
		gtk_unregister_dialog(dialog);
		gtk_widget_destroy(dialog);
		_priv=NULL;
	}
}
namespace ADM_GtkCoreUIToolkit
{
DIA_workingBase *createWorking(const char *title)
{
    return new DIA_workingGtk(title);
}
};
//-------------------------------------------------------------
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *vbox1;
  GtkWidget *alignment1;
  GtkWidget *table1;
  GtkWidget *label2;
  GtkWidget *label4;
  GtkWidget *alignment2;
  GtkWidget *hbox1;
  GtkWidget *label5;

  dialog1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (dialog1, 250, -1);
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Processing"));
  gtk_window_set_position (GTK_WINDOW (dialog1), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (dialog1), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 12);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_box_pack_start (GTK_BOX (vbox1), alignment1, FALSE, FALSE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 12, 0, 0);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (alignment1), table1);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 12);

  label2 = gtk_label_new (QT_TR_NOOP("<b>Elapsed:</b>"));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  labelElapsed = gtk_label_new ("00:00:00");
  gtk_widget_show (labelElapsed);
  gtk_table_attach (GTK_TABLE (table1), labelElapsed, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelElapsed), 0, 0.5);

  label4 = gtk_label_new (QT_TR_NOOP("<b>Time remaining:</b>"));
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label4), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  labelRemaining = gtk_label_new ("");
  gtk_widget_show (labelRemaining);
  gtk_table_attach (GTK_TABLE (table1), labelRemaining, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelRemaining), 0, 0.5);

  progressbar1 = gtk_progress_bar_new ();
  gtk_widget_show (progressbar1);
  gtk_box_pack_start (GTK_BOX (vbox1), progressbar1, FALSE, FALSE, 0);
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progressbar1), "0%");

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_box_pack_start (GTK_BOX (vbox1), alignment2, FALSE, FALSE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 24, 0, 0, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox1);

  label5 = gtk_label_new ("");
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (hbox1), label5, TRUE, TRUE, 0);

  closebutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (closebutton1);
  gtk_box_pack_start (GTK_BOX (hbox1), closebutton1, FALSE, FALSE, 0);
  
  return dialog1;
}
