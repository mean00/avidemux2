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
#include "prefs.h"
#include "ADM_tray.h"
#include "DIA_coreToolkit.h"
#include "avidemutils.h"
#include "DIA_working.h"
#include "DIA_encoding.h"
#include "ADM_vidMisc.h"

/**
    \class DIA_encodingGtk

*/
class DIA_encodingGtk : public DIA_encodingBase
{
private:
                ADM_tray    *tray;
                void        setBitrate(uint32_t br,uint32_t globalbr);
                void        updateUI(void);
                void        setQuantIn(int size);
                void        setAudioSizeIn(int size);
                void        setVideoSizeIn(int size);
                void        setSize(int size);

public:
                             DIA_encodingGtk( uint32_t fps1000 );
                virtual      ~DIA_encodingGtk( );

                virtual void reset( void );
                virtual void setPhasis(const char *n);
                virtual void setCodec(const char *n);
                virtual void setAudioCodec(const char *n);
                virtual void setFps(uint32_t fps);
                virtual void setFrame(uint32_t nb,uint32_t size, uint32_t quant,uint32_t total);
                virtual void setContainer(const char *container);
                virtual void setAudioSize(uint32_t size);
                virtual uint8_t isAlive(void);
                virtual void setPercent(uint32_t percent);
};

static GtkWidget *dialog=NULL;
static GtkWidget	*create_dialog1 (void);
static char string[500];
extern uint8_t DIA_Paused( void );
extern void UI_deiconify( void );
extern void UI_iconify( void );
static gint on_destroy_abort(GtkObject * object, gpointer user_data);
static void DIA_stop( void);
static void change_priority(void);

#ifndef __WIN32
static void shutdown_toggled(void);
#endif

static uint8_t stopReq=0;
/**
    \fn DIA_encodingGtk
*/
DIA_encodingGtk::DIA_encodingGtk( uint32_t fps1000 ) : DIA_encodingBase(fps1000)
{
uint32_t useTray=0;
        tray=NULL;
        if(!prefs->get(FEATURE_USE_SYSTRAY,&useTray)) useTray=0;

        ADM_assert(dialog==NULL);
        stopReq=0;
        setFps(fps1000);

        dialog=create_dialog1();

        gtk_register_dialog(dialog);

	#ifndef __WIN32
		// check for root privileges
		if (getuid() != 0)
		{
			// set priority to normal, regardless of preferences
			gtk_combo_box_set_active(GTK_COMBO_BOX(WID(combobox_priority)), 2);
		}else
                {
                  	gtk_widget_set_sensitive(WID(combobox_priority), 0);
                }

        gtk_signal_connect(GTK_OBJECT(WID(checkbutton_shutdown)), "toggled",
                      GTK_SIGNAL_FUNC(shutdown_toggled), NULL);
	#endif

        gtk_signal_connect(GTK_OBJECT(WID(combobox_priority)), "changed",
              GTK_SIGNAL_FUNC(change_priority), NULL);

        gtk_signal_connect(GTK_OBJECT(WID(closebutton1)), "clicked",
                      GTK_SIGNAL_FUNC(DIA_stop),                   NULL);
        gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                      GTK_SIGNAL_FUNC(on_destroy_abort), NULL);

		// set priority
		uint32_t priority;

		prefs->get(PRIORITY_ENCODING,&priority);

	#ifndef __WIN32
		// check for root privileges
		if (getuid() == 0)
		{
			gtk_combo_box_set_active(GTK_COMBO_BOX(WID(combobox_priority)),priority);
		}else
                  gtk_widget_set_sensitive(WID(combobox_priority), 0);
	#else
		gtk_combo_box_set_active(GTK_COMBO_BOX(WID(combobox_priority)),priority);
	#endif

        gtk_widget_show(dialog);
        UI_purge();
		if (useTray)
		{
			gtk_window_iconify(GTK_WINDOW(dialog));
			UI_iconify();
			tray = new ADM_tray(dialog);
		}
		else
			tray = NULL;

}

void DIA_encodingGtk::setFps(uint32_t fps)
{
        _roundup=(uint32_t )floor( (fps+999)/1000);
        _fps1000=fps;
        ADM_assert(_roundup<MAX_BR_SLOT);
        memset(_bitrate,0,sizeof(_bitrate));
        _bitrate_sum=0;
        _average_bitrate=0;

}
gint on_destroy_abort(GtkObject * object, gpointer user_data)
{


      UNUSED_ARG(object);
      UNUSED_ARG(user_data);

      stopReq=1;
      return TRUE;

};

void DIA_stop( void)
{
	printf("Stop request\n");
	stopReq=1;
}

void change_priority(void)
{
#ifndef __WIN32
	if (getuid() != 0)
	{
		gtk_signal_disconnect_by_func(GTK_OBJECT(WID(combobox_priority)), GTK_SIGNAL_FUNC(change_priority), NULL);
		gtk_combo_box_set_active(GTK_COMBO_BOX(WID(combobox_priority)), 2);
		gtk_signal_connect(GTK_OBJECT(WID(combobox_priority)), "changed", GTK_SIGNAL_FUNC(change_priority), NULL);

		GUI_Error_HIG(QT_TR_NOOP("Privileges Required"), QT_TR_NOOP( "Root privileges are required to perform this operation."));

		return;
	}
#endif

	uint32_t priorityLevel;

	priorityLevel = gtk_combo_box_get_active(GTK_COMBO_BOX(WID(combobox_priority)));

	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));
}

#ifndef __WIN32
void shutdown_toggled(void)
{
	if (getuid() != 0)
	{
		gtk_signal_disconnect_by_func(GTK_OBJECT(WID(checkbutton_shutdown)), GTK_SIGNAL_FUNC(shutdown_toggled), NULL);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton_shutdown)), false);
		gtk_signal_connect(GTK_OBJECT(WID(checkbutton_shutdown)), "toggled", GTK_SIGNAL_FUNC(shutdown_toggled), NULL);

		GUI_Error_HIG(QT_TR_NOOP("Privileges Required"), QT_TR_NOOP( "Root privileges are required to perform this operation."));
	}
}
#endif

DIA_encodingGtk::~DIA_encodingGtk( )
{
	bool shutdownRequired = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton_shutdown)));

	setpriority(PRIO_PROCESS, 0, _originalPriority);

	if(tray) delete tray;
	tray=NULL;
	ADM_assert(dialog);
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	dialog=NULL;
	UI_deiconify();

	if (shutdownRequired && !stopReq)
	{
		DIA_workingBase *work=createWorking(QT_TR_NOOP("Shutting down"));
		bool performShutdown=true;

		for(int i = 0; i <= 30; i++)
		{
			if (work->isAlive())
			{
				GUI_Sleep(1000);
				work->update(i, 30);
			}
			else
			{
				performShutdown=false;
				break;
			}
		}

		if (performShutdown && shutdown())
		{
			GUI_Sleep(5000);
		}

		delete work;
	}
}

void DIA_encodingGtk::setPhasis(const char *n)
{
          ADM_assert(dialog);
          gtk_label_set_text(GTK_LABEL(WID(label_phasis)),n);

}
void DIA_encodingGtk::setAudioCodec(const char *n)
{
          ADM_assert(dialog);
          gtk_label_set_text(GTK_LABEL(WID(label_acodec)),n);

}

void DIA_encodingGtk::setCodec(const char *n)
{
          ADM_assert(dialog);
          gtk_label_set_text(GTK_LABEL(WID(label_vcodec)),n);

}
void DIA_encodingGtk::setBitrate(uint32_t br,uint32_t globalbr)
{
          ADM_assert(dialog);
          sprintf(string,"%u kB/s",globalbr);
          gtk_label_set_text(GTK_LABEL(WID(label_bitrate)),string);

}
void DIA_encodingGtk::reset(void)
{
          ADM_assert(dialog);
          _totalSize=0;
          _videoSize=0;
          _current=0;
}
void DIA_encodingGtk::setContainer(const char *container)
{
        ADM_assert(dialog);
        gtk_label_set_text(GTK_LABEL(WID(label_container)),container);
}
#define  ETA_SAMPLE_PERIOD 60000 //Use last n millis to calculate ETA
#define  GUI_UPDATE_RATE 500

void DIA_encodingGtk::setFrame(uint32_t nb,uint32_t size, uint32_t quant,uint32_t total)
{
          _total=total;
          _videoSize+=size;
          if(nb < _lastnb || _lastnb == 0) // restart ?
           {
                _lastnb = nb;
                clock.reset();
                _lastTime=clock.getElapsedMS();
                _lastFrame=0;
                _fps_average=0;
                _videoSize=size;

                _nextUpdate = _lastTime + GUI_UPDATE_RATE;
                _nextSampleStartTime=_lastTime + ETA_SAMPLE_PERIOD;
                _nextSampleStartFrame=0;
          }
          _lastnb = nb;
          _current=nb%_roundup;
          _bitrate[_current].size=size;
          _bitrate[_current].quant=quant;
}
/**
        \fn setPercent
*/
void DIA_encodingGtk::setPercent(uint32_t percent)
{
uint32_t tim;

	   ADM_assert(dialog);

          tim=clock.getElapsedMS();
           // printf(" cur:%u last:%u next :%u\n",tim,_lastTime,_nextUpdate);
          if(_lastTime > tim) return;
          if( tim < _nextUpdate) return ;
          _nextUpdate = tim+GUI_UPDATE_RATE;


          // update progress bar
            float f=percent;
            f=f/100;
          if(tray)
                  tray->setPercent((int)(f*100.));
          gtk_progress_set_percentage(GTK_PROGRESS(WID(progressbar1)),(gfloat)f);

          sprintf(string,QT_TR_NOOP("%d%%"),(int)(100*f));

          if(GUI_isQuiet()) printf("[Encoding]%s\n",string);
              gtk_progress_bar_set_text       (GTK_PROGRESS_BAR(WID(progressbar1)), string);

        UI_purge();

}
void DIA_encodingGtk::updateUI(void)
{
uint32_t tim;

	   ADM_assert(dialog);
     	   //
           //	nb/total=timestart/totaltime -> total time =timestart*total/nb
           //
           //
          if(!_lastnb) return;

          tim=clock.getElapsedMS();
          if(_lastTime > tim) return;
          if( tim < _nextUpdate) return ;
          _nextUpdate = tim+GUI_UPDATE_RATE;

          sprintf(string,"%u",_lastnb);
          gtk_label_set_text(GTK_LABEL(WID(label_frame)),string);

		  sprintf(string,"%u",_total);
		  gtk_label_set_text(GTK_LABEL(WID(label_totalframe)),string);

          // Average bitrate  on the last second
          uint32_t sum=0,aquant=0,gsum;
          for(int i=0;i<_roundup;i++)
          {
            sum+=_bitrate[i].size;
            aquant+=_bitrate[i].quant;
          }

          aquant/=_roundup;

          sum=(sum*8)/1000;

          // Now compute global average bitrate
          float whole=_videoSize,second;
            second=_lastnb;
            second/=_fps1000;
            second*=1000;

          whole/=second;
          whole/=1000;
          whole*=8;

          gsum=(uint32_t)whole;

          setBitrate(sum,gsum);
          setQuantIn(aquant);

          // compute fps
          uint32_t deltaFrame, deltaTime;
          deltaTime=tim-_lastTime;
          deltaFrame=_lastnb-_lastFrame;

          _fps_average    =(float)( deltaFrame*1000.0F / deltaTime );

          sprintf(string,"%.2f",_fps_average);
          gtk_label_set_text(GTK_LABEL(WID(label_fps)),string);

          uint32_t   hh,mm,ss;

            double framesLeft=(_total-_lastnb);

			ms2time(tim,&hh,&mm,&ss);
			sprintf(string,"%02d:%02d:%02d",hh,mm,ss);
			gtk_label_set_text(GTK_LABEL(WID(label_elapsed)),string);

		//	gtk_label_set_text(GTK_LABEL(WID(label_eta)), ms2timedisplay((uint32_t) floor(0.5 + deltaTime * framesLeft / deltaFrame)));

           // Check if we should move on to the next sample period
          if (tim >= _nextSampleStartTime + ETA_SAMPLE_PERIOD ) {
            _lastTime=_nextSampleStartTime;
            _lastFrame=_nextSampleStartFrame;
            _nextSampleStartTime=tim;
            _nextSampleStartFrame=0;
          } else if (tim >= _nextSampleStartTime && _nextSampleStartFrame == 0 ) {
            // Store current point for use later as the next sample period.
            //
            _nextSampleStartTime=tim;
            _nextSampleStartFrame=_lastnb;
          }
          // update progress bar
            float f=_lastnb;
            f=f/_total;
          if(tray)
                  tray->setPercent((int)(f*100.));
          gtk_progress_set_percentage(GTK_PROGRESS(WID(progressbar1)),(gfloat)f);

          sprintf(string,QT_TR_NOOP("%d%%"),(int)(100*f));

          if(GUI_isQuiet()) printf("[Encoding]%s\n",string);
              gtk_progress_bar_set_text       (GTK_PROGRESS_BAR(WID(progressbar1)), string);

        _totalSize=_audioSize+_videoSize;
        setSize(_totalSize>>20);
        setAudioSizeIn((_audioSize>>20));
        setVideoSizeIn((_videoSize>>20));
        UI_purge();

}
void DIA_encodingGtk::setQuantIn(int size)
{
      ADM_assert(dialog);
          sprintf(string,"%d",size);
          gtk_label_set_text(GTK_LABEL(WID(label_quant)),string);

}

void DIA_encodingGtk::setSize(int size)
{
      ADM_assert(dialog);
          sprintf(string,QT_TR_NOOP("%"LU" MB"),size);
          gtk_label_set_text(GTK_LABEL(WID(label_size)),string);

}
void DIA_encodingGtk::setAudioSizeIn(int size)
{
      ADM_assert(dialog);
          sprintf(string,QT_TR_NOOP("%"LU" MB"),size);
          gtk_label_set_text(GTK_LABEL(WID(label_asize)),string);

}
void DIA_encodingGtk::setVideoSizeIn(int size)
{
      ADM_assert(dialog);
          sprintf(string,QT_TR_NOOP("%"LU" MB"),size);
          gtk_label_set_text(GTK_LABEL(WID(label_vsize)),string);

}

void DIA_encodingGtk::setAudioSize(uint32_t size)
{
      _audioSize=size;
}
uint8_t DIA_encodingGtk::isAlive( void )
{

        updateUI();

	if(stopReq)
	{
		if(DIA_Paused(  ))		//=GUI_Question("Continue encoding, no will stop it ?"))
		{
			stopReq=0;
		}
	}

	if(!stopReq) return 1;

	return 0;
}

/*-----------------------------------------------------------*/
namespace ADM_GtkCoreUIToolkit
{
DIA_encodingBase *createEncodingGtk(uint32_t title)
{
    return new DIA_encodingGtk(title);
}
};

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *vbox3;
  GtkWidget *table5;
  GtkWidget *frame1;
  GtkWidget *alignment2;
  GtkWidget *table6;
  GtkWidget *label39;
  GtkWidget *label_phasis;
  GtkWidget *label41;
  GtkWidget *label_vcodec;
  GtkWidget *label43;
  GtkWidget *label_acodec;
  GtkWidget *label45;
  GtkWidget *label_container;
  GtkWidget *frame2;
  GtkWidget *alignment3;
  GtkWidget *table7;
  GtkWidget *label47;
  GtkWidget *label48;
  GtkWidget *label49;
  GtkWidget *label50;
  GtkWidget *label_bitrate;
  GtkWidget *label_quant;
  GtkWidget *label_totalframe;
  GtkWidget *label_frame;
  GtkWidget *frame3;
  GtkWidget *alignment4;
  GtkWidget *table8;
  GtkWidget *label55;
  GtkWidget *label56;
  GtkWidget *label57;
  GtkWidget *label_size;
  GtkWidget *label_asize;
  GtkWidget *label_vsize;
  GtkWidget *frame4;
  GtkWidget *alignment5;
  GtkWidget *table9;
  GtkWidget *label61;
  GtkWidget *label62;
  GtkWidget *label63;
  GtkWidget *label_fps;
  GtkWidget *label_eta;
  GtkWidget *label_elapsed;
  GtkWidget *alignment8;
  GtkWidget *vbox4;
  GtkWidget *progressbar1;
  GtkWidget *hbox4;
  GtkWidget *checkbutton_shutdown;
  GtkWidget *label69;
  GtkWidget *label68;
  GtkWidget *combobox_priority;
  GtkWidget *alignment11;
  GtkWidget *hseparator1;
  GtkWidget *hbox2;
  GtkWidget *table10;
  GtkWidget *closebutton1;
  GtkWidget *alignment9;
  GtkWidget *hbox3;
  GtkWidget *image2;
  GtkWidget *label67;

  dialog1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (dialog1, 500, -1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 12);
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Encoding..."));
  gtk_window_set_position (GTK_WINDOW (dialog1), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (dialog1), vbox3);

  table5 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table5);
  gtk_box_pack_start (GTK_BOX (vbox3), table5, TRUE, FALSE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table5), 12);
  gtk_table_set_col_spacings (GTK_TABLE (table5), 12);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_table_attach (GTK_TABLE (table5), frame1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame1), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 10, 10, 12, 12);

  table6 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table6);
  gtk_container_add (GTK_CONTAINER (alignment2), table6);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 14);

  label39 = gtk_label_new (QT_TR_NOOP("<b>Phase:</b>"));
  gtk_widget_show (label39);
  gtk_table_attach (GTK_TABLE (table6), label39, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label39), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label39), 0, 0.5);

  label_phasis = gtk_label_new (QT_TR_NOOP("None"));
  gtk_widget_show (label_phasis);
  gtk_table_attach (GTK_TABLE (table6), label_phasis, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_phasis), 0, 0.5);

  label41 = gtk_label_new (QT_TR_NOOP("<b>Video Codec:</b>"));
  gtk_widget_show (label41);
  gtk_table_attach (GTK_TABLE (table6), label41, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label41), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label41), 0, 0.5);

  label_vcodec = gtk_label_new (QT_TR_NOOP("None"));
  gtk_widget_show (label_vcodec);
  gtk_table_attach (GTK_TABLE (table6), label_vcodec, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_vcodec), 0, 0.5);

  label43 = gtk_label_new (QT_TR_NOOP("<b>Audio Codec:</b>"));
  gtk_widget_show (label43);
  gtk_table_attach (GTK_TABLE (table6), label43, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label43), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label43), 0, 0.5);

  label_acodec = gtk_label_new (QT_TR_NOOP("None"));
  gtk_widget_show (label_acodec);
  gtk_table_attach (GTK_TABLE (table6), label_acodec, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_acodec), 0, 0.5);

  label45 = gtk_label_new (QT_TR_NOOP("<b>Container:</b>"));
  gtk_widget_show (label45);
  gtk_table_attach (GTK_TABLE (table6), label45, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label45), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label45), 0, 0.5);

  label_container = gtk_label_new (QT_TR_NOOP("Unknown"));
  gtk_widget_show (label_container);
  gtk_table_attach (GTK_TABLE (table6), label_container, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_container), 0, 0.5);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_table_attach (GTK_TABLE (table5), frame2, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (frame2), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 10, 10, 12, 12);

  table7 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table7);
  gtk_container_add (GTK_CONTAINER (alignment3), table7);
  gtk_table_set_row_spacings (GTK_TABLE (table7), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table7), 14);

  label47 = gtk_label_new (QT_TR_NOOP("<b>Processed Frames:</b>"));
  gtk_widget_show (label47);
  gtk_table_attach (GTK_TABLE (table7), label47, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label47), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label47), 0, 0.5);

  label48 = gtk_label_new (QT_TR_NOOP("<b>Total Frames:</b>"));
  gtk_widget_show (label48);
  gtk_table_attach (GTK_TABLE (table7), label48, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label48), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label48), 0, 0.5);

  label49 = gtk_label_new (QT_TR_NOOP("<b>Quantiser:</b>"));
  gtk_widget_show (label49);
  gtk_table_attach (GTK_TABLE (table7), label49, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label49), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label49), 0, 0.5);

  label50 = gtk_label_new (QT_TR_NOOP("<b>Average Bitrate:</b>"));
  gtk_widget_show (label50);
  gtk_table_attach (GTK_TABLE (table7), label50, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label50), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label50), 0, 0.5);

  label_bitrate = gtk_label_new (QT_TR_NOOP("0 kB/s"));
  gtk_widget_show (label_bitrate);
  gtk_table_attach (GTK_TABLE (table7), label_bitrate, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_bitrate), 0, 0.5);

  label_quant = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label_quant);
  gtk_table_attach (GTK_TABLE (table7), label_quant, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_quant), 0, 0.5);

  label_totalframe = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label_totalframe);
  gtk_table_attach (GTK_TABLE (table7), label_totalframe, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_totalframe), 0, 0.5);

  label_frame = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label_frame);
  gtk_table_attach (GTK_TABLE (table7), label_frame, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_frame), 0, 0.5);

  frame3 = gtk_frame_new (NULL);
  gtk_widget_show (frame3);
  gtk_table_attach (GTK_TABLE (table5), frame3, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  alignment4 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment4);
  gtk_container_add (GTK_CONTAINER (frame3), alignment4);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment4), 10, 10, 12, 12);

  table8 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table8);
  gtk_container_add (GTK_CONTAINER (alignment4), table8);
  gtk_table_set_row_spacings (GTK_TABLE (table8), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table8), 14);

  label55 = gtk_label_new (QT_TR_NOOP("<b>Video Size:</b>"));
  gtk_widget_show (label55);
  gtk_table_attach (GTK_TABLE (table8), label55, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label55), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label55), 0, 0.5);

  label56 = gtk_label_new (QT_TR_NOOP("<b>Audio Size:</b>"));
  gtk_widget_show (label56);
  gtk_table_attach (GTK_TABLE (table8), label56, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label56), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label56), 0, 0.5);

  label57 = gtk_label_new (QT_TR_NOOP("<b>Total Size:</b>"));
  gtk_widget_show (label57);
  gtk_table_attach (GTK_TABLE (table8), label57, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label57), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label57), 0, 0.5);

  label_size = gtk_label_new (QT_TR_NOOP("0 MB"));
  gtk_widget_show (label_size);
  gtk_table_attach (GTK_TABLE (table8), label_size, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_size), 0, 0.5);
  gtk_label_set_width_chars (GTK_LABEL (label_size), 10);

  label_asize = gtk_label_new (QT_TR_NOOP("0 MB"));
  gtk_widget_show (label_asize);
  gtk_table_attach (GTK_TABLE (table8), label_asize, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_asize), 0, 0.5);
  gtk_label_set_width_chars (GTK_LABEL (label_asize), 10);

  label_vsize = gtk_label_new (QT_TR_NOOP("0 MB"));
  gtk_widget_show (label_vsize);
  gtk_table_attach (GTK_TABLE (table8), label_vsize, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_vsize), 0, 0.5);
  gtk_label_set_width_chars (GTK_LABEL (label_vsize), 10);

  frame4 = gtk_frame_new (NULL);
  gtk_widget_show (frame4);
  gtk_table_attach (GTK_TABLE (table5), frame4, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  alignment5 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment5);
  gtk_container_add (GTK_CONTAINER (frame4), alignment5);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment5), 10, 10, 12, 12);

  table9 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table9);
  gtk_container_add (GTK_CONTAINER (alignment5), table9);
  gtk_table_set_row_spacings (GTK_TABLE (table9), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table9), 14);

  label61 = gtk_label_new (QT_TR_NOOP("<b>Elapsed:</b>"));
  gtk_widget_show (label61);
  gtk_table_attach (GTK_TABLE (table9), label61, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label61), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label61), 0, 0.5);

  label62 = gtk_label_new (QT_TR_NOOP("<b>Time Remaining:</b>"));
  gtk_widget_show (label62);
  gtk_table_attach (GTK_TABLE (table9), label62, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label62), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label62), 0, 0.5);

  label63 = gtk_label_new (QT_TR_NOOP("<b>Frames/sec:</b>"));
  gtk_widget_show (label63);
  gtk_table_attach (GTK_TABLE (table9), label63, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label63), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label63), 0, 0.5);

  label_fps = gtk_label_new (QT_TR_NOOP("0"));
  gtk_widget_show (label_fps);
  gtk_table_attach (GTK_TABLE (table9), label_fps, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_fps), 0, 0.5);

  label_eta = gtk_label_new (QT_TR_NOOP("Unknown"));
  gtk_widget_show (label_eta);
  gtk_table_attach (GTK_TABLE (table9), label_eta, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_eta), 0, 0.5);

  label_elapsed = gtk_label_new (QT_TR_NOOP("00:00:00"));
  gtk_widget_show (label_elapsed);
  gtk_table_attach (GTK_TABLE (table9), label_elapsed, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_elapsed), 0, 0.5);

  alignment8 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment8);
  gtk_box_pack_start (GTK_BOX (vbox3), alignment8, FALSE, FALSE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment8), 10, 15, 0, 0);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (alignment8), vbox4);

  progressbar1 = gtk_progress_bar_new ();
  gtk_widget_show (progressbar1);
  gtk_box_pack_start (GTK_BOX (vbox4), progressbar1, FALSE, FALSE, 4);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox4, TRUE, TRUE, 0);

  checkbutton_shutdown = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("Shut down computer when finished"));
  gtk_widget_show (checkbutton_shutdown);
  gtk_box_pack_start (GTK_BOX (hbox4), checkbutton_shutdown, FALSE, FALSE, 0);

  label69 = gtk_label_new ("");
  gtk_widget_show (label69);
  gtk_box_pack_start (GTK_BOX (hbox4), label69, TRUE, FALSE, 0);

  label68 = gtk_label_new (QT_TR_NOOP("Priority:"));
  gtk_widget_show (label68);
  gtk_box_pack_start (GTK_BOX (hbox4), label68, FALSE, FALSE, 6);
  gtk_misc_set_alignment (GTK_MISC (label68), 0, 0.5);

  combobox_priority = gtk_combo_box_new_text ();
  gtk_widget_show (combobox_priority);
  gtk_box_pack_start (GTK_BOX (hbox4), combobox_priority, FALSE, FALSE, 0);
  GTK_WIDGET_SET_FLAGS (combobox_priority, GTK_CAN_FOCUS);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_priority), QT_TR_NOOP("High"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_priority), QT_TR_NOOP("Above Normal"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_priority), QT_TR_NOOP("Normal"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_priority), QT_TR_NOOP("Below Normal"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox_priority), QT_TR_NOOP("Low"));

  alignment11 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment11);
  gtk_box_pack_start (GTK_BOX (vbox3), alignment11, TRUE, TRUE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment11), 6, 30, 0, 0);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_container_add (GTK_CONTAINER (alignment11), hseparator1);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox2, FALSE, TRUE, 0);

  table10 = gtk_table_new (1, 1, FALSE);
  gtk_widget_show (table10);
  gtk_box_pack_start (GTK_BOX (hbox2), table10, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table10), 5);

  closebutton1 = gtk_button_new ();
  gtk_widget_show (closebutton1);
  gtk_box_pack_start (GTK_BOX (hbox2), closebutton1, FALSE, FALSE, 0);
  GTK_WIDGET_SET_FLAGS (closebutton1, GTK_CAN_DEFAULT);

  alignment9 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment9);
  gtk_container_add (GTK_CONTAINER (closebutton1), alignment9);

  hbox3 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (alignment9), hbox3);

  image2 = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox3), image2, FALSE, FALSE, 0);

  label67 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Pause / Abort"));
  gtk_widget_show (label67);
  gtk_box_pack_start (GTK_BOX (hbox3), label67, FALSE, FALSE, 0);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (dialog1, table5, "table5");
  GLADE_HOOKUP_OBJECT (dialog1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (dialog1, table6, "table6");
  GLADE_HOOKUP_OBJECT (dialog1, label39, "label39");
  GLADE_HOOKUP_OBJECT (dialog1, label_phasis, "label_phasis");
  GLADE_HOOKUP_OBJECT (dialog1, label41, "label41");
  GLADE_HOOKUP_OBJECT (dialog1, label_vcodec, "label_vcodec");
  GLADE_HOOKUP_OBJECT (dialog1, label43, "label43");
  GLADE_HOOKUP_OBJECT (dialog1, label_acodec, "label_acodec");
  GLADE_HOOKUP_OBJECT (dialog1, label45, "label45");
  GLADE_HOOKUP_OBJECT (dialog1, label_container, "label_container");
  GLADE_HOOKUP_OBJECT (dialog1, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (dialog1, alignment3, "alignment3");
  GLADE_HOOKUP_OBJECT (dialog1, table7, "table7");
  GLADE_HOOKUP_OBJECT (dialog1, label47, "label47");
  GLADE_HOOKUP_OBJECT (dialog1, label48, "label48");
  GLADE_HOOKUP_OBJECT (dialog1, label49, "label49");
  GLADE_HOOKUP_OBJECT (dialog1, label50, "label50");
  GLADE_HOOKUP_OBJECT (dialog1, label_bitrate, "label_bitrate");
  GLADE_HOOKUP_OBJECT (dialog1, label_quant, "label_quant");
  GLADE_HOOKUP_OBJECT (dialog1, label_totalframe, "label_totalframe");
  GLADE_HOOKUP_OBJECT (dialog1, label_frame, "label_frame");
  GLADE_HOOKUP_OBJECT (dialog1, frame3, "frame3");
  GLADE_HOOKUP_OBJECT (dialog1, alignment4, "alignment4");
  GLADE_HOOKUP_OBJECT (dialog1, table8, "table8");
  GLADE_HOOKUP_OBJECT (dialog1, label55, "label55");
  GLADE_HOOKUP_OBJECT (dialog1, label56, "label56");
  GLADE_HOOKUP_OBJECT (dialog1, label57, "label57");
  GLADE_HOOKUP_OBJECT (dialog1, label_size, "label_size");
  GLADE_HOOKUP_OBJECT (dialog1, label_asize, "label_asize");
  GLADE_HOOKUP_OBJECT (dialog1, label_vsize, "label_vsize");
  GLADE_HOOKUP_OBJECT (dialog1, frame4, "frame4");
  GLADE_HOOKUP_OBJECT (dialog1, alignment5, "alignment5");
  GLADE_HOOKUP_OBJECT (dialog1, table9, "table9");
  GLADE_HOOKUP_OBJECT (dialog1, label61, "label61");
  GLADE_HOOKUP_OBJECT (dialog1, label62, "label62");
  GLADE_HOOKUP_OBJECT (dialog1, label63, "label63");
  GLADE_HOOKUP_OBJECT (dialog1, label_fps, "label_fps");
  GLADE_HOOKUP_OBJECT (dialog1, label_eta, "label_eta");
  GLADE_HOOKUP_OBJECT (dialog1, label_elapsed, "label_elapsed");
  GLADE_HOOKUP_OBJECT (dialog1, alignment8, "alignment8");
  GLADE_HOOKUP_OBJECT (dialog1, vbox4, "vbox4");
  GLADE_HOOKUP_OBJECT (dialog1, progressbar1, "progressbar1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton_shutdown, "checkbutton_shutdown");
  GLADE_HOOKUP_OBJECT (dialog1, label69, "label69");
  GLADE_HOOKUP_OBJECT (dialog1, label68, "label68");
  GLADE_HOOKUP_OBJECT (dialog1, combobox_priority, "combobox_priority");
  GLADE_HOOKUP_OBJECT (dialog1, alignment11, "alignment11");
  GLADE_HOOKUP_OBJECT (dialog1, hseparator1, "hseparator1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (dialog1, table10, "table10");
  GLADE_HOOKUP_OBJECT (dialog1, closebutton1, "closebutton1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment9, "alignment9");
  GLADE_HOOKUP_OBJECT (dialog1, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (dialog1, image2, "image2");
  GLADE_HOOKUP_OBJECT (dialog1, label67, "label67");

  gtk_widget_grab_focus (closebutton1);
  gtk_widget_grab_default (closebutton1);
  return dialog1;
}
