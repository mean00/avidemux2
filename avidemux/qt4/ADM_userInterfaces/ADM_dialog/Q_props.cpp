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

#include <math.h>

#include "Q_props.h"
#include "avi_vars.h"
#include "avidemutils.h"
#include "ADM_vidMisc.h"

static const char *yesno[2]={QT_TR_NOOP("No"),QT_TR_NOOP("Yes")};
extern const char *getStrFromAudioCodec( uint32_t codec);

propWindow::propWindow() : QDialog()
 {
     ui.setupUi(this);
     uint8_t gmc, qpel,vop;
 uint32_t info=0;
 uint32_t war,har;
 uint32_t hh, mm, ss, ms;
 char text[80];
 const char *s;
  
    text[0] = 0;
    if (!avifileinfo)
        return;
#if 0  
        // Fetch info
        info=video_body->getSpecificMpeg4Info();
        vop=!!(info & ADM_VOP_ON);
        qpel=!!(info & ADM_QPEL_ON);
        gmc=!!(info & ADM_GMC_ON);
#endif
#define FILLTEXT(a,b,c) {snprintf(text,79,b,c);ui.a->setText(text);}
#define FILLTEXT4(a,b,c,d) {snprintf(text,79,b,c,d);ui.a->setText(text);}
#define FILLTEXT5(a,b,c,d,e) {snprintf(text,79,b,c,d,e);ui.a->setText(text);}
        
        FILLTEXT4(labeImageSize,QT_TR_NOOP("%"LU" x %"LU), avifileinfo->width,avifileinfo->height);
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
#define SET_YES(a,b) ui.a->setText(yesno[b])
#define FILLQT_TR_NOOP(q) ui.q->setText(text);
        SET_YES(LabelPackedBitstream,vop);
        SET_YES(LabelQuarterPixel,qpel);
        SET_YES(LabelGMC,gmc);
        
         WAVHeader *wavinfo=NULL;
         wavinfo=video_body->getInfo();
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
                FILLTEXT(labelFrequency, QT_TR_NOOP("%"LU" Hz"), wavinfo->frequency);
                FILLTEXT4(labelBitrate, QT_TR_NOOP("%"LU" Bps / %"LU" kbps"), wavinfo->byterate,wavinfo->byterate * 8 / 1000);
                
                sprintf(text, "%s", getStrFromAudioCodec(wavinfo->encoding));
                FILLQT_TR_NOOP(labelACodec);
                //
                duration=video_body->getDurationInUs();
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
     // Fetch info
     propWindow *propwindow=new propWindow ;
     propwindow->exec();
     delete propwindow;
}  
//********************************************
//EOF
