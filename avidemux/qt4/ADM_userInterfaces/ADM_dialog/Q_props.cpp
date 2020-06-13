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
#include "ADM_edAudioTrackExternal.h"
#include "ADM_edAudioTrackFromVideo.h"
#include "ADM_vidMisc.h"
#include "ADM_toolkitQt.h"
#include "ADM_coreUtils.h"
#include <QClipboard>

static const char *yesno[2]={QT_TRANSLATE_NOOP("qprops","No"),QT_TRANSLATE_NOOP("qprops","Yes")};

propWindow::propWindow(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
    uint32_t war,har;
    uint32_t hh, mm, ss, ms;
#define MXL 80
    char text[MXL];
    const char *s;

    text[0] = 0;
    listOfValues.clear();
    if (!avifileinfo)
        return;

#define FILLTEXT(a,b,c) {snprintf(text,MXL,b,c); listOfValues.push_back(QString::fromUtf8(text)); ui.a->setText(QString::fromUtf8(text));}
#define FILLTEXT4(a,b,c,d) {snprintf(text,MXL,b,c,d); listOfValues.push_back(QString::fromUtf8(text)); ui.a->setText(QString::fromUtf8(text));}
#define FILLTEXT5(a,b,c,d,e) {snprintf(text,MXL,b,c,d,e); listOfValues.push_back(QString::fromUtf8(text)); ui.a->setText(QString::fromUtf8(text));}
#define SET_YES(a,b) ui.a->setText(QString::fromUtf8(yesno[b]))
#define FILLQT_TRANSLATE_NOOP(a,q) listOfValues.push_back(QString::fromUtf8(text)); ui.q->setText(QString::fromUtf8(text));

    //------------------------------------

    FILLTEXT(label4CC, "%s", fourCC::tostring(avifileinfo->fcc));

    FILLTEXT4(labeImageSize,QT_TRANSLATE_NOOP("qprops","%" PRIu32" x %" PRIu32), avifileinfo->width,avifileinfo->height);

    war=video_body->getPARWidth();
    har=video_body->getPARHeight();
    getAspectRatioFromAR(war,har, &s);
    FILLTEXT5(LabelAspectRatio,QT_TRANSLATE_NOOP("qprops","%s (%u:%u)"), s,war,har);

    FILLTEXT(labelFrameRate, QT_TRANSLATE_NOOP("qprops","%2.3f fps"), (float) avifileinfo->fps1000 / 1000.F);

    uint64_t duration=video_body->getVideoDuration();
    ms2time(duration/1000,&hh,&mm,&ss,&ms);
    snprintf(text, MXL, QT_TRANSLATE_NOOP("qprops","%02d:%02d:%02d.%03d"), hh, mm, ss, ms);
    listOfValues.push_back(text);
    ui.labelVideoDuration->setText(text);

    uint32_t extraLen;
    uint8_t *extraData;
    video_body->getExtraHeaderData(&extraLen,&extraData);
    FILLTEXT(LabelExtraDataSize,"%02d",extraLen);

    if(extraLen)
    {
        int capped=extraLen;
        if(capped>10) capped=10;
        QString string;
        char smallx[10];
        for(int i=0;i<capped;i++)
        {
            snprintf(smallx,4,"%02X ",extraData[i]);
            string+=QString(smallx);
        }
        listOfValues.push_back(string);
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
        int nbActive=video_body->getNumberOfActiveAudioTracks();
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        QString titleAudio=QCoreApplication::translate("qprops","Audio (%n active track(s))",NULL,nbActive);
#else
        QString titleAudio=QCoreApplication::translate("qprops","Audio (%n active track(s))",NULL,QCoreApplication::UnicodeUTF8,nbActive);
#endif
        ui.groupBoxAudio->setTitle(titleAudio);
        listOfValues.push_back(titleAudio);

        snprintf(text, MXL, "%s", getStrFromAudioCodec(wavinfo->encoding));
        FILLQT_TRANSLATE_NOOP("qprops",labelACodec);

        uint32_t channels=wavinfo->channels;
        uint32_t frequency=wavinfo->frequency;
        duration=0;
        EditableAudioTrack *ed=video_body->getDefaultEditableAudioTrack();
        if(ed)
        {
            ADM_EDAUDIO_TRACK_TYPE type=ed->edTrack->getTrackType();
            switch (type)
            {
                case ADM_EDAUDIO_FROM_VIDEO:
                    duration=ed->edTrack->castToTrackFromVideo()->getDurationInUs();
                    channels=ed->edTrack->castToTrackFromVideo()->getOutputChannels();
                    frequency=ed->edTrack->castToTrackFromVideo()->getOutputFrequency();
                    break;
                case ADM_EDAUDIO_EXTERNAL:
                    duration=ed->edTrack->castToExternal()->getDurationInUs();
                    channels=ed->edTrack->castToExternal()->getOutputChannels();
                    frequency=ed->edTrack->castToExternal()->getOutputFrequency();
                    break;
                default:break;
            }
        }

        switch (channels)
        {
            case 1:
                sprintf(text,"%s", QT_TRANSLATE_NOOP("qprops","Mono"));
                break;
            case 2:
                sprintf(text,"%s", QT_TRANSLATE_NOOP("qprops","Stereo"));
                break;
            default:
                sprintf(text, "%d",channels);
                break;
        }

        FILLQT_TRANSLATE_NOOP("qprops",labelChannels);

        FILLTEXT4(labelBitrate, QT_TRANSLATE_NOOP("qprops","%" PRIu32" Bps / %" PRIu32" kbps"), wavinfo->byterate, wavinfo->byterate * 8 / 1000);

        FILLTEXT(labelVBR,"%s","n/a");

        FILLTEXT(labelFrequency, QT_TRANSLATE_NOOP("qprops","%" PRIu32" Hz"), frequency);

        ms2time(duration/1000,&hh,&mm,&ss,&ms);
        sprintf(text, QT_TRANSLATE_NOOP("qprops","%02d:%02d:%02d.%03d"), hh, mm, ss, ms);
        FILLQT_TRANSLATE_NOOP("qprops",labelAudioDuration);
        ui.labelAudioDuration->setEnabled(!!duration);

//                SET_YES(labelVBR,currentaudiostream->isVBR());
    } else
    {
        ui.groupBoxAudio->setEnabled(false);
    }

    connect(ui.pushButton_c2c,SIGNAL(clicked()),this,SLOT(propsCopyToClipboard()));
}

#define ADDCATEGORY(a) props += QString("\n=====================================================\n")\
                               +QString::fromUtf8(a)\
                               +QString("\n=====================================================\n");
#define ADDNAMEVALUE(a,b) props += QString::fromUtf8(a)+QString("\t")+QString(b)+QString("\n");

/**
    \fn propsCopyToClipboard
    \brief Copy a list of property-value pairs to clipboard
*/
void propWindow::propsCopyToClipboard(void)
{
    if(listOfValues.size()<6) // something went wrong and the list is not sufficiently populated
        return;

    QString props;
    props = QString();

    ADDCATEGORY(QT_TRANSLATE_NOOP("qprops","Video"))

    ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Codec 4CC:\t"),listOfValues.at(0))

    ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Image Size:\t"),listOfValues.at(1))

    ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Aspect Ratio:\t"),listOfValues.at(2))

    ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Frame Rate:\t"),listOfValues.at(3))

    ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Total Duration:\t"),listOfValues.at(4))

    ADDCATEGORY(QT_TRANSLATE_NOOP("qprops","Extra Video Properties"))

    ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","ExtraDataSize:\t"),listOfValues.at(5))

    bool hasExtraData=false;
    if((QString)listOfValues.at(5) != QString("00"))
    {
        hasExtraData=true;
        ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Extra data:\t"),listOfValues.at(6))
    }

    QString aud;
    if(listOfValues.size() == 13+hasExtraData)
        aud=listOfValues.at(6+hasExtraData);
    else
        aud=QString::fromUtf8(QT_TRANSLATE_NOOP("qprops","Audio"));

    ADDCATEGORY(aud.toUtf8().constData())

    if(listOfValues.size() == 13+hasExtraData)
    {
        ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Codec:\t\t"),listOfValues.at(7+hasExtraData))

        ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Channels:\t"),listOfValues.at(8+hasExtraData))

        ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Bitrate:\t"),listOfValues.at(9+hasExtraData))

        //ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Variable Bitrate"),listOfValues.at(10+hasExtraData))

        ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Frequency:\t"),listOfValues.at(11+hasExtraData))

        ADDNAMEVALUE(QT_TRANSLATE_NOOP("qprops","Total Duration:\t"),listOfValues.at(12+hasExtraData))
    }else
    {
        props += QT_TRANSLATE_NOOP("qprops","No Audio");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(props);
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
