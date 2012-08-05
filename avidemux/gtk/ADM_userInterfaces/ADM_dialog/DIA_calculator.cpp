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
#include "GUI_glade.h"
#include "DIA_coreToolkit.h"
#include "DIA_fileSel.h"

#include "ADM_edit.hxx"

#include "ADM_vidMisc.h"
#include "avi_vars.h"
#include "prototype.h"

#define GW(x) glade.getWidget(#x)

void DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate );
static GtkWidget	*create_Calculator (void);
static admGlade glade;
static void prepare( void );
static void update( void );
static int cb_mod(GtkWidget * object, gpointer user_data);
static int cb_mod2(GtkWidget * object, gpointer user_data);

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

	glade.init();
	if (!glade.loadFile("calculator.gtkBuilder"))
	{
		GUI_Error_HIG(QT_TR_NOOP("Cannot load dialog"), 
		              QT_TR_NOOP("File \"calculator.gtkBuilder\" could not be loaded."));
		return;
	}
	GtkWidget *dialog = GW(dialogCalculator);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_CANCEL,
										GTK_RESPONSE_APPLY,
										-1);
	gtk_register_dialog(dialog);
	g_signal_connect(GW(comboboxFormat), "changed", G_CALLBACK(cb_mod), (void *) 0);
	g_signal_connect(GW(comboboxMedium), "changed", G_CALLBACK(cb_mod2), (void *) 0);
	prepare();
	update();
        gtk_widget_set_sensitive(GW(spinbuttonCustom),0);
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
	gtk_label_set_text(GTK_LABEL(GW(labelDuration),string);

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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(GW(spinbuttonTrack1)), track1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(GW(spinbuttonTrack2)), track2);

	printf("Track1 bitrate :%lu\n",track1);
	printf("Track2 bitrate :%lu\n",track2);

#endif

}
/************************************/
uint32_t getPicSize(void)
{
#warning FIXME
return 720*576;
#if 0
uint32_t size;
         AVDMGenericVideoStream *last;
                last=getLastVideoFilter();
                size=last->getInfo()->width*last->getInfo()->height;

                return size;
#endif
}
//*************************************
void update( void )
{
uint32_t audioSize;
uint32_t totalSize;
char string[200];
aviInfo info;

        video_body->getVideoInfo(&info);

	track1=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(GW(spinbuttonTrack1)));
	track2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(GW(spinbuttonTrack2)));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(GW(spinbuttonTrack1)), track1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(GW(spinbuttonTrack2)), track1);

	// kb->Byte
	audioSize=(track1+track2)*1000;
	audioSize/=8;

	audioSize*=videoDuration;
	audioSize>>=20;
	sprintf(string,"%"PRIu32,audioSize);
	gtk_label_set_text(GTK_LABEL(GW(labelAudio)),string);

	// Compute total size (for Avi)
	uint32_t s74,s80,dvd;

	// For avi/ogm
	int f = gtk_combo_box_get_active(GTK_COMBO_BOX(GW(comboboxFormat)));
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

	int j=gtk_combo_box_get_active(GTK_COMBO_BOX(GW(comboboxMedium)));
	switch(j)
	{
		case 2: totalSize=1*s74;break;
		case 3: totalSize=2*s74;break;
		case 0: totalSize=1*s80;break;
		case 1: totalSize=2*s80;break;
		case 4: totalSize=dvd;break;
		case 5: totalSize=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(GW(spinbuttonCustom))); 
		        if(totalSize<1) totalSize=1;
				break;
		default:
			ADM_assert(0);
	}
	sprintf(string,"%"PRIu32,totalSize);
	gtk_label_set_text(GTK_LABEL(GW(labelTotal)),string);

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
	//gtk_label_set_text(GTK_LABEL(GW(labelMux)),string);

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
			sprintf(string,"%"PRIu32,(uint32_t)videoBitrate);

			gtk_label_set_text(GTK_LABEL(GW(labelBitrate)),string);

			//
			sprintf(string,"%"PRIu32,videoSize);
	                gtk_label_set_text(GTK_LABEL(GW(labelVideo)),string);
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
                        gtk_label_set_text(GTK_LABEL(GW(labelBPP)),string);

		}



}
//
int cb_mod(GtkWidget * object, gpointer user_data)
{
int r;
int sens=0;
        r=gtk_combo_box_get_active(GTK_COMBO_BOX(GW(comboboxMedium)));
        if(r==5)
        {
                sens=1;
        }
        gtk_widget_set_sensitive(GW(spinbuttonCustom),sens);
        update();
        return 0;
}
int cb_mod2(GtkWidget * object, gpointer user_data)
{
        update();
        return 0;
}
