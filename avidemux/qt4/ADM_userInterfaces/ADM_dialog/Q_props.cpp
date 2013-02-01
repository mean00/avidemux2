/***************************************************************************
    copyright            : (C) 2001 by mean
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
#include "ADM_cpp.h"
#include <math.h>

#include "Q_props.h"
#include "avi_vars.h"
#include "ADM_vidMisc.h"
#include "ADM_toolkitQt.h"
#include "ADM_coreUtils.h"

static const char *yesno[2]={QT_TR_NOOP("No"),QT_TR_NOOP("Yes")};

propWindow::propWindow(QWidget *parent) : QDialog(parent)
 {
     ui.setupUi(this);
     uint8_t gmc=0, qpel=0,vop=0;
 uint32_t info=0;
 uint32_t war,har;
 uint32_t hh, mm, ss, ms;
 char text[80];
 const char *s;
  
    text[0] = 0;
    if (!avifileinfo)
        return;

#define FILLTEXT(a,b,c) {snprintf(text,79,b,c);ui.a->setText(text);}
#define FILLTEXT4(a,b,c,d) {snprintf(text,79,b,c,d);ui.a->setText(text);}
#define FILLTEXT5(a,b,c,d,e) {snprintf(text,79,b,c,d,e);ui.a->setText(text);}
#define SET_YES(a,b) ui.a->setText(yesno[b])
#define FILLQT_TR_NOOP(q) ui.q->setText(text);
        
    
        //------------------------------------
    
        FILLTEXT4(labeImageSize,QT_TR_NOOP("%"PRIu32" x %"PRIu32), avifileinfo->width,avifileinfo->height);
        FILLTEXT(labelFrameRate, QT_TR_NOOP("%2.3f fps"), (float) avifileinfo->fps1000 / 1000.F);
        FILLTEXT(label4CC, "%s",      fourCC::tostring(avifileinfo->fcc));
        uint64_t duration=video_body->getVideoDuration();
        ms2time(duration/1000,&hh,&mm,&ss,&ms);
        snprintf(text,79, QT_TR_NOOP("%02d:%02d:%02d.%03d"), hh, mm, ss, ms);
        ui.labelVideoDuration->setText(text);

        war=video_body->getPARWidth();
        har=video_body->getPARHeight();
        getAspectRatioFromAR(war,har, &s);
        FILLTEXT5(LabelAspectRatio,QT_TR_NOOP("%s (%u:%u)"), s,war,har);
        
        uint32_t extraLen;
        uint8_t *extraData;
        video_body->getExtraHeaderData(&extraLen,&extraData);
        FILLTEXT(LabelExtraDataSize,"%02d",extraLen);
        if(extraLen)
        {
            int capped=extraLen;
            if(capped>10) capped=10;
            QString string;
            char    small[10];
            for(int i=0;i<capped;i++)
            {
                snprintf(small,4,"%02X ",extraData[i]);
                string+=QString(small);
            }
            ui.LabelExtraData->setText(string);
        }else
            ui.LabelExtraData->clear();
        
        //------------------------------------
         WAVHeader *wavinfo=NULL;
         ADM_audioStream *st;
         video_body->getDefaultAudioTrack(&st);
         if(st)
            wavinfo=st->getInfo();
          if(wavinfo)
          {
              
              switch (wavinfo->channels)
                {
                case 1:
                    sprintf(text,"%s", QT_TR_NOOP("Mono"));
                    break;
                case 2:
                    sprintf(text,"%s", QT_TR_NOOP("Stereo"));
                    break;
                default:
                    sprintf(text, "%d",wavinfo->channels);
                    break;
                }

                FILLQT_TR_NOOP(labelChannels);
                FILLTEXT(labelFrequency, QT_TR_NOOP("%"PRIu32" Hz"), wavinfo->frequency);
                FILLTEXT4(labelBitrate, QT_TR_NOOP("%"PRIu32" Bps / %"PRIu32" kbps"), wavinfo->byterate,wavinfo->byterate * 8 / 1000);
                
                sprintf(text, "%s", getStrFromAudioCodec(wavinfo->encoding));
                FILLQT_TR_NOOP(labelACodec);
                //
                duration=st->getDurationInUs();
                ms2time(duration/1000,&hh,&mm,&ss,&ms);

                sprintf(text, QT_TR_NOOP("%02d:%02d:%02d.%03d"), hh, mm, ss, ms);
                FILLQT_TR_NOOP(labelAudioDuration);
//                SET_YES(labelVBR,currentaudiostream->isVBR());
        } else
          {
			  ui.groupBoxAudio->setEnabled(false);
          }
 }
/**
    \fn DIA_properties
    \brief Display dialog with file information (size, codec, duration etc....)
*/
void DIA_properties( void )
{
	if (playing)
		return;

	if (!avifileinfo)
		return;

	propWindow propwindow(qtLastRegisteredDialog());
	qtRegisterDialog(&propwindow);
	propwindow.exec();
	qtUnregisterDialog(&propwindow);
}  
//********************************************
//EOF
