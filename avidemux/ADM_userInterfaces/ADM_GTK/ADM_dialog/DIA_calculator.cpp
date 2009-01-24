/***************************************************************************
                          Simple bitrate calculator
                             -------------------
    begin                : Wed Dec 18 2002
    copyright            : (C) 2004 by mean
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

#include "ADM_toolkitGtk.h"


#include "DIA_fileSel.h"

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"

#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_audiofilter/audioeng_buildfilters.h"


#include "ADM_video/ADM_vidMisc.h"
#include "avi_vars.h"

void DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate );
static GtkWidget	*create_Calculator (void);
static GtkWidget *dialog;
static void prepare( void );
static void update( void );
static int cb_mod(GtkObject * object, gpointer user_data);
static int cb_mod2(GtkObject * object, gpointer user_data);

static uint32_t videoDuration;
static uint32_t track1, track2;

static uint32_t numberOfVideoFrames;

static uint32_t videoSize,videoBitrate;

static uint32_t getPicSize(void);
extern uint8_t         videoCodecSetFinalSize(uint32_t size);;
//************************************
void DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate )
{
	if(!avifileinfo) return ;

	dialog=create_Calculator();
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_CANCEL,
										GTK_RESPONSE_APPLY,
										-1);
	gtk_register_dialog(dialog);
        gtk_signal_connect(GTK_OBJECT(WID(optionmenu1)), "changed",
                      GTK_SIGNAL_FUNC(cb_mod),   (void *) 0);
        gtk_signal_connect(GTK_OBJECT(WID(optionmenu2)), "changed",
                      GTK_SIGNAL_FUNC(cb_mod2),   (void *) 0);
	prepare();
	update();
        gtk_widget_set_sensitive(WID(entryCustom),0);
	while(1)
	{

		if(GTK_RESPONSE_APPLY==gtk_dialog_run(GTK_DIALOG(dialog)))
		{
			update();
                        videoCodecSetFinalSize(videoSize);

		}
	else
		{
			break;
		}
	}
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	*sizeInMeg=videoSize;
	*avgBitrate=videoBitrate;

}
//******************************************
// Compute the value that will not change
// (duration)
// and init the entries for current bitrate
//******************************************
void prepare( void )
{
#if 0
	float duration=0;
	aviInfo info;
	char string[200];
	 uint16_t mm,hh,ss,ms;
         AVDMGenericAudioStream *stream;

	 if(frameStart<frameEnd) numberOfVideoFrames=frameEnd-frameStart;
	 else			 numberOfVideoFrames=frameStart-frameEnd;



	duration=(float)video_body->getTime (numberOfVideoFrames);
	duration=duration/1000.;

	if(duration<0) duration=-duration;

	video_body->getVideoInfo(&info);

	videoDuration=(uint32_t)ceil(duration);
	frame2time(numberOfVideoFrames,info.fps1000, &hh, &mm, &ss, &ms);
	// now we can set it
	sprintf(string,"%02d:%02d:%02d",hh,mm,ss);
	gtk_label_set_text(GTK_LABEL(WID(labelDuration)),string);

	printf("Video duration :%lu\n",videoDuration);

	// Now get audio info
	track1=0;
	if(audioProcessMode() && currentaudiostream)
	{
//		stream=buildAudioFilter(currentaudiostream,0);

		if(stream)
		{
			track1=(stream->getInfo()->byterate*8)/1000;
		}
//                deleteAudioFilter(stream);
	}else
	{
		if(currentaudiostream) track1=(currentaudiostream->getInfo()->byterate*8)/1000;
	}

	track2=0;
	gtk_write_entry(WID(entry3), track1);
	gtk_write_entry(WID(entry4), track2);

	printf("Track1 bitrate :%lu\n",track1);
	printf("Track2 bitrate :%lu\n",track2);

#endif

}
/************************************/
uint32_t getPicSize(void)
{
uint32_t size;
         AVDMGenericVideoStream *last;
                last=getLastVideoFilter();
                size=last->getInfo()->width*last->getInfo()->height;

                return size;
}
//*************************************
void update( void )
{
uint32_t audioSize;
uint32_t totalSize;
char string[200];
aviInfo info;

        video_body->getVideoInfo(&info);

	track1=gtk_read_entry(WID(entry3));
	track2=gtk_read_entry(WID(entry4));
	gtk_write_entry(WID(entry3), track1);
	gtk_write_entry(WID(entry4), track2);

	// kb->Byte
	audioSize=(track1+track2)*1000;
	audioSize/=8;

	audioSize*=videoDuration;
	audioSize>>=20;
	sprintf(string,"%"LU,audioSize);
	gtk_label_set_text(GTK_LABEL(WID(labelAudio)),string);

	// Compute total size (for Avi)
	uint32_t s74,s80,dvd;

	// For avi/ogm
	int f = getRangeInMenu(WID(optionmenu2));
	if(f==2)
	{ // Mpeg
		s74=730;
		s80=790;
		dvd=4300;

	}
	else
	{//AVI or OGM
		s74=650;
		s80=700;
		dvd=4300;
	}

	int j=getRangeInMenu(WID(optionmenu1));
	switch(j)
	{
		case 2: totalSize=1*s74;break;
		case 3: totalSize=2*s74;break;
		case 0: totalSize=1*s80;break;
		case 1: totalSize=2*s80;break;
		case 4: totalSize=dvd;break;
                case 5: totalSize=gtk_read_entry(WID(entryCustom)); if(totalSize<1) totalSize=1;break;
		default:
			ADM_assert(0);
	}
	sprintf(string,"%"LU,totalSize);
	gtk_label_set_text(GTK_LABEL(WID(labelTotal)),string);

	// Compute muxing overhead size
	uint32_t muxingOverheadSize;
	int numberOfAudioTracks = 0;
	int numberOfChunks;
	switch (f)
	{
		case 0:
			// AVI
			/*
				Muxing overhead is 8 + 32 = 40 bytes per chunk.
				More or less: numberOfChunks = (x + 1) * numberOfVideoFrames,
				where x - the number of audio tracks
			*/
			if (track1 != 0)
			{
				numberOfAudioTracks++;
			}
			if (track2 != 0)
			{
				numberOfAudioTracks++;
			}
			numberOfChunks = (numberOfAudioTracks + 1) * numberOfVideoFrames;
			muxingOverheadSize = (uint32_t) ceil((numberOfChunks * 40) / 1048576.0);;
			break;
		case 1:
			// OGM
			// Muxing overhead is 1.1% to 1.2% of (videoSize + audioSize)
			muxingOverheadSize = (uint32_t) ceil(totalSize - totalSize / 1.012);
			break;
		case 2:
			// MPEG
			// Muxing overhead is 1%  to 2% of (videoSize + audioSize)
			muxingOverheadSize = (uint32_t) ceil(totalSize - totalSize / 1.02);
			break;
		default:
			ADM_assert(0);
	}
	//sprintf(string,"%lu",muxingOverheadSize);
	//gtk_label_set_text(GTK_LABEL(WID(labelMux)),string);

	// and compute
	//
	//uint32_t videoSize;

	if(audioSize>=totalSize) sprintf(string,"** NO ROOM LEFT**");
	else
		{
                        uint32_t picSize;
			videoSize=totalSize-audioSize - muxingOverheadSize;
			// Compute average bps
			float avg;
                        float bpp;
			avg=videoSize;
			avg*=1024.*1024.;
			avg/=videoDuration;
			// now we have byte /sec
			// convert to kb per sec
			avg=(avg*8)/1000;
			videoBitrate=(uint32_t)avg;
			sprintf(string,"%"LU,(uint32_t)videoBitrate);

			gtk_label_set_text(GTK_LABEL(WID(labelBitrate)),string);

			//
			sprintf(string,"%"LU,videoSize);
	                gtk_label_set_text(GTK_LABEL(WID(labelVideo)),string);
                        // Bpp
                        bpp=videoBitrate;
                        bpp=bpp*1000000.;  // kbit->bit + compensate for fps1000
                        // Fetch info from filter


                        if(videoProcessMode())
                                picSize=getPicSize();
                        else
                                picSize=info.width*info.height;
                        bpp/=picSize;
                        //printf("w:%d h:%d\n",info.width,info.height);
                        bpp/=info.fps1000;
                        sprintf(string,"%1.3f",bpp);
                        gtk_label_set_text(GTK_LABEL(WID(labelBPP)),string);

		}



}
//
int cb_mod(GtkObject * object, gpointer user_data)
{
int r;
int sens=0;
        r=getRangeInMenu(WID(optionmenu1));
        if(r==5)
        {
                sens=1;
        }
        gtk_widget_set_sensitive(WID(entryCustom),sens);
        update();
        return 0;
}
int cb_mod2(GtkObject * object, gpointer user_data)
{
        update();
        return 0;
}
//*****************

GtkWidget*
create_Calculator (void)
{
  GtkWidget *Calculator;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *framebox;
  GtkWidget *alignment1;
  GtkWidget *table2;
  GtkWidget *hbox5;
  GtkWidget *heading;
  GtkWidget *labelDOO;
  GtkWidget *labelDuration;
  GtkWidget *hbox14;
  GtkWidget *label2;
  GtkWidget *hbox16;
  GtkWidget *label16;
  GtkWidget *entryCustom;
  GtkWidget *hbox21;
  GtkWidget *label14;
  GtkWidget *hbox18;
  GtkWidget *optionmenu2;
  GtkWidget *menu2;
  GtkWidget *avi1;
  GtkWidget *ogm1;
  GtkWidget *mpeg1;
  GtkWidget *hbox17;
  GtkWidget *optionmenu1;
  GtkWidget *menu1;
  GtkWidget *_1x80_cd1;
  GtkWidget *_2x80_cd1;
  GtkWidget *_1x74_cd1;
  GtkWidget *_2x74_cd1;
  GtkWidget *dvd1;
  GtkWidget *custom1;
  GtkWidget *hbox11;
  GtkWidget *hbox12;
  GtkWidget *hbox13;
  GtkWidget *label6;
  GtkWidget *entry3;
  GtkWidget *label7;
  GtkWidget *entry4;
  GtkWidget *hbox20;
  GtkWidget *table3;
  GtkWidget *label9;
  GtkWidget *labelAudio;
  GtkWidget *label11;
  GtkWidget *labelVideo;
  GtkWidget *label17;
  GtkWidget *labelTotal;
  GtkWidget *table4;
  GtkWidget *label12;
  GtkWidget *label15;
  GtkWidget *labelBitrate;
  GtkWidget *labelBPP;

  GtkWidget *dialog_action_area1;
  GtkWidget *doit;
  GtkWidget *button1;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  Calculator = gtk_dialog_new ();
  gtk_container_set_border_width (GTK_CONTAINER (Calculator), 6);
  gtk_window_set_title (GTK_WINDOW (Calculator), QT_TR_NOOP("Calculator"));
  gtk_window_set_resizable (GTK_WINDOW (Calculator), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (Calculator), FALSE);

  dialog_vbox1 = GTK_DIALOG (Calculator)->vbox;
  gtk_box_set_spacing (GTK_BOX(dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 18);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 6);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  heading = gtk_label_new (QT_TR_NOOP("<b>Target</b>"));
  gtk_misc_set_alignment (GTK_MISC (heading), 0, 1);
  gtk_label_set_use_markup (GTK_LABEL (heading), TRUE);
  gtk_widget_show (heading);

  table2 = gtk_table_new (3, 4, FALSE);
  gtk_widget_show (table2);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 12);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 6, 0, 18, 0);
  gtk_container_add (GTK_CONTAINER (alignment1), table2);

  framebox = gtk_vbox_new (0, 0);
  gtk_box_pack_start (GTK_BOX(vbox1), framebox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(framebox), heading, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(framebox), alignment1, FALSE, FALSE, 0);
  gtk_widget_show(framebox);

  hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_table_attach (GTK_TABLE (table2), hbox5, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  labelDOO = gtk_label_new (QT_TR_NOOP("Duration:"));
  gtk_widget_show (labelDOO);
  gtk_box_pack_start (GTK_BOX (hbox5), labelDOO, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (labelDOO), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelDOO), 0, 0.5);

  labelDuration = gtk_label_new (QT_TR_NOOP("00:00:00"));
  gtk_widget_show (labelDuration);
  gtk_table_attach (GTK_TABLE (table2), labelDuration, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelDuration), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelDuration), 0, 0.5);

  hbox14 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox14);
  gtk_table_attach (GTK_TABLE (table2), hbox14, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label2 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Medium:"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox14), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  hbox16 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox16);
  gtk_table_attach (GTK_TABLE (table2), hbox16, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label16 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Format:"));
  gtk_widget_show (label16);
  gtk_box_pack_start (GTK_BOX (hbox16), label16, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label16), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);

  entryCustom = gtk_entry_new ();
  gtk_widget_show (entryCustom);
  gtk_table_attach (GTK_TABLE (table2), entryCustom, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, entryCustom, QT_TR_NOOP("Total size (custom)"), NULL);
  gtk_entry_set_max_length (GTK_ENTRY (entryCustom), 10);
  gtk_entry_set_width_chars (GTK_ENTRY (entryCustom), 6);

  hbox21 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox21);
  gtk_table_attach (GTK_TABLE (table2), hbox21, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label14 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Custom size (MB):"));
  gtk_widget_show (label14);
  gtk_box_pack_start (GTK_BOX (hbox21), label14, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label14), GTK_JUSTIFY_LEFT);

  hbox18 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox18);
  gtk_table_attach (GTK_TABLE (table2), hbox18, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  optionmenu2 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu2);
  gtk_box_pack_start (GTK_BOX (hbox18), optionmenu2, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, optionmenu2, QT_TR_NOOP("Output container format"), NULL);

  menu2 = gtk_menu_new ();

  avi1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("AVI"));
  gtk_widget_show (avi1);
  gtk_container_add (GTK_CONTAINER (menu2), avi1);

  ogm1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("OGM"));
  gtk_widget_show (ogm1);
  gtk_container_add (GTK_CONTAINER (menu2), ogm1);

  mpeg1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("MPEG"));
  gtk_widget_show (mpeg1);
  gtk_container_add (GTK_CONTAINER (menu2), mpeg1);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), menu2);

  hbox17 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox17);
  gtk_table_attach (GTK_TABLE (table2), hbox17, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  optionmenu1 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu1);
  gtk_box_pack_start (GTK_BOX (hbox17), optionmenu1, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, optionmenu1, QT_TR_NOOP("Total size"), NULL);

  menu1 = gtk_menu_new ();

  _1x80_cd1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("1x 80 min. CD"));
  gtk_widget_show (_1x80_cd1);
  gtk_container_add (GTK_CONTAINER (menu1), _1x80_cd1);
  gtk_tooltips_set_tip (tooltips, _1x80_cd1, QT_TR_NOOP("Total size (custom)"), NULL);

  _2x80_cd1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("2x 80 min. CD"));
  gtk_widget_show (_2x80_cd1);
  gtk_container_add (GTK_CONTAINER (menu1), _2x80_cd1);

  _1x74_cd1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("1x 74 min. CD"));
  gtk_widget_show (_1x74_cd1);
  gtk_container_add (GTK_CONTAINER (menu1), _1x74_cd1);

  _2x74_cd1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("2x 74 min. CD"));
  gtk_widget_show (_2x74_cd1);
  gtk_container_add (GTK_CONTAINER (menu1), _2x74_cd1);

  dvd1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("DVD5"));
  gtk_widget_show (dvd1);
  gtk_container_add (GTK_CONTAINER (menu1), dvd1);

  custom1 = gtk_menu_item_new_with_mnemonic (QT_TR_NOOP("Custom"));
  gtk_widget_show (custom1);
  gtk_container_add (GTK_CONTAINER (menu1), custom1);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), menu1);

  hbox11 = gtk_hbox_new (TRUE, 12);
  gtk_widget_show (hbox11);

  hbox12 = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (hbox11), hbox12, TRUE, TRUE, 0);
  gtk_widget_show (hbox12);
  label6 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Track _1:"));
  gtk_widget_show (label6);
  gtk_box_pack_start (GTK_BOX (hbox12), label6, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);

  entry3 = gtk_entry_new ();
  gtk_widget_show (entry3);
  gtk_box_pack_start (GTK_BOX (hbox12), entry3, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, entry3, QT_TR_NOOP("Audio bitrate in kb/s for track 1 (0 for no audio)"), NULL);
  gtk_entry_set_max_length (GTK_ENTRY (entry3), 10);
  gtk_entry_set_width_chars (GTK_ENTRY (entry3), 6);

  hbox13 = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (hbox11), hbox13, TRUE, TRUE, 0);
  gtk_widget_show (hbox13);
  label7 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Track _2: "));
  gtk_widget_show (label7);
  gtk_box_pack_start (GTK_BOX (hbox13), label7, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);

  entry4 = gtk_entry_new ();
  gtk_widget_show (entry4);
  gtk_box_pack_start (GTK_BOX (hbox13), entry4, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, entry4, QT_TR_NOOP("Audio bitrate in kb/s for track 2 (optional)"), NULL);
  gtk_entry_set_max_length (GTK_ENTRY (entry4), 10);
  gtk_entry_set_width_chars (GTK_ENTRY (entry4), 6);

  heading = gtk_label_new (QT_TR_NOOP("<b>Audio Bitrate</b>"));
  gtk_label_set_use_markup (GTK_LABEL (heading), TRUE);
  gtk_misc_set_alignment (GTK_MISC (heading), 0, 1);
  gtk_widget_show (heading);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 6, 0, 18, 0);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox11);

  framebox = gtk_vbox_new (0, 0);
  gtk_box_pack_start (GTK_BOX(vbox1), framebox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(framebox), heading, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(framebox), alignment1, FALSE, FALSE, 0);
  gtk_widget_show(framebox);


  hbox20 = gtk_hbox_new (TRUE, 12);
  gtk_widget_show (hbox20);

  table3 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table3);
  gtk_box_pack_start (GTK_BOX (hbox20), table3, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 12);

  label9 = gtk_label_new (QT_TR_NOOP("Audio size (MB):"));
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table3), label9, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  labelAudio = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (labelAudio);
  gtk_table_attach (GTK_TABLE (table3), labelAudio, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelAudio), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelAudio), 0, 0.5);

  label11 = gtk_label_new (QT_TR_NOOP("Video size (MB):"));
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table3), label11, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  labelVideo = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (labelVideo);
  gtk_table_attach (GTK_TABLE (table3), labelVideo, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelVideo), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelVideo), 0, 0.5);

  label17 = gtk_label_new (QT_TR_NOOP("Total size (MB):"));
  gtk_widget_show (label17);
  gtk_table_attach (GTK_TABLE (table3), label17, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label17), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label17), 0, 0.5);

  labelTotal = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (labelTotal);
  gtk_table_attach (GTK_TABLE (table3), labelTotal, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelTotal), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelTotal), 0, 0.5);

  table4 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table4);
  gtk_box_pack_start (GTK_BOX (hbox20), table4, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table4), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table4), 6);

  label12 = gtk_label_new (QT_TR_NOOP("Video bitrate:"));
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table4), label12, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label12), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  label15 = gtk_label_new (QT_TR_NOOP("Bits per pixel:"));
  gtk_widget_show (label15);
  gtk_table_attach (GTK_TABLE (table4), label15, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label15), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label15), 0, 0.5);

  labelBitrate = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (labelBitrate);
  gtk_table_attach (GTK_TABLE (table4), labelBitrate, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelBitrate), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelBitrate), 0, 0.5);

  labelBPP = gtk_label_new (QT_TR_NOOP("0.0"));
  gtk_widget_show (labelBPP);
  gtk_table_attach (GTK_TABLE (table4), labelBPP, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelBPP), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelBPP), 0, 0.5);

  heading = gtk_label_new (QT_TR_NOOP("<b>Result</b>"));
  gtk_misc_set_alignment (GTK_MISC (heading), 0, 1);
  gtk_label_set_use_markup (GTK_LABEL (heading), TRUE);
  gtk_widget_show (heading);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 6, 0, 18, 0);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox20);

  framebox = gtk_vbox_new (0, 0);
  gtk_box_pack_start (GTK_BOX(vbox1), framebox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(framebox), heading, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX(framebox), alignment1, FALSE, FALSE, 0);
  gtk_widget_show(framebox);

  dialog_action_area1 = GTK_DIALOG (Calculator)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  doit = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (doit);
  gtk_dialog_add_action_widget (GTK_DIALOG (Calculator), doit, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (doit, GTK_CAN_DEFAULT);

  button1 = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (button1);
  gtk_dialog_add_action_widget (GTK_DIALOG (Calculator), button1, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label2), optionmenu1);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label16), optionmenu2);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label14), entryCustom);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label6), entry3);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label7), entry4);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (Calculator, Calculator, "Calculator");
  GLADE_HOOKUP_OBJECT_NO_REF (Calculator, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (Calculator, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (Calculator, framebox, "framebox");
  GLADE_HOOKUP_OBJECT (Calculator, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (Calculator, table2, "table2");
  GLADE_HOOKUP_OBJECT (Calculator, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (Calculator, heading, "heading");
  GLADE_HOOKUP_OBJECT (Calculator, labelDOO, "labelDOO");
  GLADE_HOOKUP_OBJECT (Calculator, labelDuration, "labelDuration");
  GLADE_HOOKUP_OBJECT (Calculator, hbox14, "hbox14");
  GLADE_HOOKUP_OBJECT (Calculator, label2, "label2");
  GLADE_HOOKUP_OBJECT (Calculator, hbox16, "hbox16");
  GLADE_HOOKUP_OBJECT (Calculator, label16, "label16");
  GLADE_HOOKUP_OBJECT (Calculator, entryCustom, "entryCustom");
  GLADE_HOOKUP_OBJECT (Calculator, hbox21, "hbox21");
  GLADE_HOOKUP_OBJECT (Calculator, label14, "label14");
  GLADE_HOOKUP_OBJECT (Calculator, hbox18, "hbox18");
  GLADE_HOOKUP_OBJECT (Calculator, optionmenu2, "optionmenu2");
  GLADE_HOOKUP_OBJECT (Calculator, menu2, "menu2");
  GLADE_HOOKUP_OBJECT (Calculator, avi1, "avi1");
  GLADE_HOOKUP_OBJECT (Calculator, ogm1, "ogm1");
  GLADE_HOOKUP_OBJECT (Calculator, mpeg1, "mpeg1");
  GLADE_HOOKUP_OBJECT (Calculator, hbox17, "hbox17");
  GLADE_HOOKUP_OBJECT (Calculator, optionmenu1, "optionmenu1");
  GLADE_HOOKUP_OBJECT (Calculator, menu1, "menu1");
  GLADE_HOOKUP_OBJECT (Calculator, _1x80_cd1, "_1x80_cd1");
  GLADE_HOOKUP_OBJECT (Calculator, _2x80_cd1, "_2x80_cd1");
  GLADE_HOOKUP_OBJECT (Calculator, _1x74_cd1, "_1x74_cd1");
  GLADE_HOOKUP_OBJECT (Calculator, _2x74_cd1, "_2x74_cd1");
  GLADE_HOOKUP_OBJECT (Calculator, dvd1, "dvd1");
  GLADE_HOOKUP_OBJECT (Calculator, custom1, "custom1");
  GLADE_HOOKUP_OBJECT (Calculator, hbox11, "hbox11");
  GLADE_HOOKUP_OBJECT (Calculator, hbox12, "hbox12");
  GLADE_HOOKUP_OBJECT (Calculator, hbox13, "hbox13");
  GLADE_HOOKUP_OBJECT (Calculator, label6, "label6");
  GLADE_HOOKUP_OBJECT (Calculator, entry3, "entry3");
  GLADE_HOOKUP_OBJECT (Calculator, label7, "label7");
  GLADE_HOOKUP_OBJECT (Calculator, entry4, "entry4");
  GLADE_HOOKUP_OBJECT (Calculator, hbox20, "hbox20");
  GLADE_HOOKUP_OBJECT (Calculator, table3, "table3");
  GLADE_HOOKUP_OBJECT (Calculator, label9, "label9");
  GLADE_HOOKUP_OBJECT (Calculator, labelAudio, "labelAudio");
  GLADE_HOOKUP_OBJECT (Calculator, label11, "label11");
  GLADE_HOOKUP_OBJECT (Calculator, labelVideo, "labelVideo");
  GLADE_HOOKUP_OBJECT (Calculator, label17, "label17");
  GLADE_HOOKUP_OBJECT (Calculator, labelTotal, "labelTotal");
  GLADE_HOOKUP_OBJECT (Calculator, table4, "table4");
  GLADE_HOOKUP_OBJECT (Calculator, label12, "label12");
  GLADE_HOOKUP_OBJECT (Calculator, label15, "label15");
  GLADE_HOOKUP_OBJECT (Calculator, labelBitrate, "labelBitrate");
  GLADE_HOOKUP_OBJECT (Calculator, labelBPP, "labelBPP");

  GLADE_HOOKUP_OBJECT_NO_REF (Calculator, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (Calculator, doit, "doit");
  GLADE_HOOKUP_OBJECT (Calculator, button1, "button1");
  GLADE_HOOKUP_OBJECT_NO_REF (Calculator, tooltips, "tooltips");

  gtk_widget_grab_focus (entry3);
  return Calculator;
}

