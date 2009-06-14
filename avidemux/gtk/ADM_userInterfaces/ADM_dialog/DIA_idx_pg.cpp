/***************************************************************************
                          DIA_dmx.cpp  -  description
                             -------------------
Indexer progress dialog

    copyright            : (C) 2005 by mean
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
#include <math.h>
#if 0
#include "ADM_toolkitGtk.h"
#include "DIA_coreToolkit.h"

#include "ADM_videoFilter.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_encoder/adm_encoder.h"
#include "DIA_idx_pg.h"
#include "ADM_video/ADM_vidMisc.h"
static GtkWidget *dialog=NULL;
static GtkWidget       *create_dialog1 (void);
static int abted;
static gint on_destroy_abort(GtkObject * object, gpointer user_data)
{
DIA_progressIndexing *pf;

        UNUSED_ARG(object);
        UNUSED_ARG(user_data);

        pf=(DIA_progressIndexing *)user_data;
        if(!GUI_Confirmation_HIG(QT_TR_NOOP("Continue indexing"),QT_TR_NOOP("Abort Requested"),QT_TR_NOOP("Do you want to abort indexing ?")))
        {
         //       pf->abortRequest();
                abted=1;
        }

        return TRUE;

};


DIA_progressIndexing::DIA_progressIndexing(const char *name)
{
        dialog=create_dialog1();
        gtk_register_dialog(dialog);
        gtk_label_set_text(GTK_LABEL(WID(labelName)),name);
        gtk_widget_show(dialog);
        clock.reset();
        aborted=0;
        abted=0;
	_nextUpdate=0;
        gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                       GTK_SIGNAL_FUNC(on_destroy_abort), (void *) this);

}
DIA_progressIndexing::~DIA_progressIndexing()
{
        ADM_assert(dialog);
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        dialog=NULL;
}
uint8_t       DIA_progressIndexing::isAborted(void)
{
        return abted;
}
uint8_t DIA_progressIndexing::abortRequest(void)
{
        aborted=1;
        abted=1;
        return 1;
}
uint8_t       DIA_progressIndexing::update(uint32_t done,uint32_t total, uint32_t nbImage, uint32_t hh, uint32_t mm, uint32_t ss)
{
        uint32_t tim;
	#define  GUI_UPDATE_RATE 1000

	tim=clock.getElapsedMS();
	if(tim>_nextUpdate)
	{
        char string[256];
        double f;
        	uint32_t   tom,zhh,zmm,zss;

		_nextUpdate=tim+GUI_UPDATE_RATE;
        sprintf(string,"%02d:%02d:%02d",hh,mm,ss);
        gtk_label_set_text(GTK_LABEL(WID(labelTime)),string);

        sprintf(string,"%0"LU,nbImage);
        gtk_label_set_text(GTK_LABEL(WID(labelNbImage)),string);

        f=done;
        f/=total;

        gtk_progress_set_percentage(GTK_PROGRESS(WID(progressbar1)),(gfloat)f);

        /* compute ETL */
       // Do a simple relation between % and time
        // Elapsed time =total time*percent
        if(f<0.01) return 1;
        f=tim/f;
        // Tom is total time
        tom=(uint32_t)floor(f);
        if(tim>tom) return 1;
        tom=tom-tim;
        ms2time(tom,&zhh,&zmm,&zss);
        sprintf(string,"%02d:%02d:%02d",zhh,zmm,zss);
        gtk_label_set_text(GTK_LABEL(WID(label6)),string);
        	UI_purge();
	}
        return 1;
}


GtkWidget       *create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *label6;
  GtkWidget *label5;
  GtkWidget *label3;
  GtkWidget *label1;
  GtkWidget *labelTime;
  GtkWidget *labelNbImage;
  GtkWidget *label7;
  GtkWidget *labelName;
  GtkWidget *progressbar1;
  GtkWidget *dialog_action_area1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Indexing..."));

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  table1 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);

  label6 = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  label5 = gtk_label_new (QT_TR_NOOP("Time left :"));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  label3 = gtk_label_new (QT_TR_NOOP("Timecode :"));
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label1 = gtk_label_new (QT_TR_NOOP("Nb pictures seen :"));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  labelTime = gtk_label_new (QT_TR_NOOP("00:00:00"));
  gtk_widget_show (labelTime);
  gtk_table_attach (GTK_TABLE (table1), labelTime, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelTime), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelTime), 0, 0.5);

  labelNbImage = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (labelNbImage);
  gtk_table_attach (GTK_TABLE (table1), labelNbImage, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelNbImage), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelNbImage), 0, 0.5);

  label7 = gtk_label_new (QT_TR_NOOP("Indexing :"));
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  labelName = gtk_label_new (QT_TR_NOOP("/dev/null"));
  gtk_widget_show (labelName);
  gtk_table_attach (GTK_TABLE (table1), labelName, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelName), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelName), 0, 0.5);

  progressbar1 = gtk_progress_bar_new ();
  gtk_widget_show (progressbar1);
  gtk_box_pack_start (GTK_BOX (vbox1), progressbar1, FALSE, FALSE, 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, table1, "table1");
  GLADE_HOOKUP_OBJECT (dialog1, label6, "label6");
  GLADE_HOOKUP_OBJECT (dialog1, label5, "label5");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, labelTime, "labelTime");
  GLADE_HOOKUP_OBJECT (dialog1, labelNbImage, "labelNbImage");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, labelName, "labelName");
  GLADE_HOOKUP_OBJECT (dialog1, progressbar1, "progressbar1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");

  return dialog1;
}
#endif