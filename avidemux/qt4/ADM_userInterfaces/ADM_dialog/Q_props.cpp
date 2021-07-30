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
    gotExtraData = false;
    gotAudio = false;
    firstTime = true;
    if (!avifileinfo)
        return;

#define FILL(a) ui.a->setText(QString::fromUtf8(text));
#define FILLTEXT(a,b,c) { snprintf(text,MXL,b,c); FILL(a) }
#define FILLTEXT4(a,b,c,d) { snprintf(text,MXL,b,c,d); FILL(a) }
#define FILLTEXT5(a,b,c,d,e) { snprintf(text,MXL,b,c,d,e); FILL(a) }
//#define SET_YES(a,b) ui.a->setText(QString::fromUtf8(yesno[b]))

    //------------------------------------

    FILLTEXT(label4CCValue, "%s", fourCC::tostring(avifileinfo->fcc))

    FILLTEXT4(labelImageSizeValue,QT_TRANSLATE_NOOP("qprops","%" PRIu32" x %" PRIu32), avifileinfo->width,avifileinfo->height)

    war=video_body->getPARWidth();
    har=video_body->getPARHeight();
    getAspectRatioFromAR(war,har, &s);

    FILLTEXT5(labelAspectRatioValue, QT_TRANSLATE_NOOP("qprops","%s (%u:%u)"), s, war, har)

    FILLTEXT(labelFrameRateValue, QT_TRANSLATE_NOOP("qprops","%2.3f fps"), (float) avifileinfo->fps1000 / 1000.F)

    if (avifileinfo->bitrate < 0)
    {
        FILLTEXT(labelVideoBitrateValue, "%s", QT_TRANSLATE_NOOP("qprops","n/a"))
    }
    else
    {
        FILLTEXT(labelVideoBitrateValue, QT_TRANSLATE_NOOP("qprops","%d kbps"), avifileinfo->bitrate)
    }

    uint64_t duration=video_body->getVideoDuration();
    ms2time(duration/1000,&hh,&mm,&ss,&ms);
    snprintf(text, MXL, QT_TRANSLATE_NOOP("qprops","%02d:%02d:%02d.%03d"), hh, mm, ss, ms);

    FILL(labelVideoDurationValue)

    uint32_t extraLen = 0;
    uint8_t *extraData;
    video_body->getExtraHeaderData(&extraLen,&extraData);

    FILLTEXT(labelExtraDataSizeValue,"%d",extraLen)

    if(extraLen)
    {
        int capped=extraLen;
#define EXTRADATA_CAP 64
        if(capped > EXTRADATA_CAP) capped = EXTRADATA_CAP;
        int missing = extraLen - capped;
        QString string;
        char smallx[4];
        for(int i=0;i<capped;i++)
        {
            snprintf(smallx,4,"%02X ",extraData[i]);
            string += smallx;
        }
        if(missing)
        {
            char hint[32];
            snprintf(hint, 32, QT_TRANSLATE_NOOP("qprops","(+%d bytes)"), missing);
            string += hint;
        }
        ui.lineEditExtraData->insert(string);
        ui.lineEditExtraData->setCursorPosition(0);
        gotExtraData = true;
    }else
    {
        ui.lineEditExtraData->clear();
    }
    ui.labelExtraData->setEnabled(gotExtraData);
    ui.lineEditExtraData->setVisible(gotExtraData);

    //------------------------------------
    WAVHeader *wavinfo=NULL;
    ADM_audioStream *st;
    video_body->getDefaultAudioTrack(&st);
    if(st)
        wavinfo=st->getInfo();
    if(wavinfo)
    {
        gotAudio = true;
        int nbActive=video_body->getNumberOfActiveAudioTracks();
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        QString titleAudio=QCoreApplication::translate("qprops","Audio (%n active track(s))",NULL,nbActive);
#else
        QString titleAudio=QCoreApplication::translate("qprops","Audio (%n active track(s))",NULL,QCoreApplication::UnicodeUTF8,nbActive);
#endif
        ui.groupBoxAudio->setTitle(titleAudio);

        snprintf(text, MXL, "%s", getStrFromAudioCodec(wavinfo->encoding));

        FILL(labelACodecName)

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

        FILL(labelChannelsValue)

        FILLTEXT4(labelAudioBitrateValue, QT_TRANSLATE_NOOP("qprops","%" PRIu32" Bps / %" PRIu32" kbps"), wavinfo->byterate, wavinfo->byterate * 8 / 1000)

        FILLTEXT(labelVBRDetected,"%s","n/a")

        FILLTEXT(labelFrequencyValue, QT_TRANSLATE_NOOP("qprops","%" PRIu32" Hz"), frequency)

        ms2time(duration/1000,&hh,&mm,&ss,&ms);
        sprintf(text, QT_TRANSLATE_NOOP("qprops","%02d:%02d:%02d.%03d"), hh, mm, ss, ms);

        FILL(labelAudioDurationValue)

        ui.labelAudioDuration->setEnabled(!!duration);

//                SET_YES(labelVBR,currentaudiostream->isVBR());
    } else
    {
        ui.groupBoxAudio->setEnabled(false);
#define CLEAR(x) ui.x->clear();
        CLEAR(labelACodecName)
        CLEAR(labelChannelsValue)
        CLEAR(labelAudioBitrateValue)
        CLEAR(labelVBRDetected)
        CLEAR(labelFrequencyValue)
        CLEAR(labelAudioDurationValue)
#undef CLEAR
    }

    connect(ui.pushButton_c2c,SIGNAL(clicked()),this,SLOT(propsCopyToClipboard()));

    adjustSize(); // squeeze out all the air
}
/**
    \fn showEvent
    \brief Align the columns in the video and audio group boxes.
*/
void propWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if(!firstTime) return;
    firstTime = false;

    int cmp,w = 0;

#define MAXME(x) if(ui.x->isVisible()) { cmp = ui.x->width(); if(cmp > w) w = cmp; }

    MAXME(label4CC)
    MAXME(labelImageSize)
    MAXME(labelAspectRatio)
    MAXME(labelFrameRate)
    MAXME(labelVideoBitrate)
    MAXME(labelVideoDuration)
    MAXME(labelACodec)
    MAXME(labelChannels)
    MAXME(labelAudioBitrate)
    MAXME(labelVBR)
    MAXME(labelFrequency)
    MAXME(labelAudioDuration)

    ui.label4CC->setMinimumWidth(w);
    ui.labelACodec->setMinimumWidth(w);
    // We don't force minimum width for the left column in the video extradata
    // group box to give more space to QLineEdit displaying extradata hexdump,
    // unless we have no extradata.
    if(!gotExtraData)
    {
        ui.labelExtraDataSize->setMinimumWidth(w);
        // When QLineEdit is hidden, the copy to clipboard button gets the focus.
        // Move focus back to the OK button.
        ui.pushButton_ok->setFocus(Qt::OtherFocusReason);
    }

    w = 0;

    MAXME(label4CCValue)
    MAXME(labelImageSizeValue)
    MAXME(labelAspectRatioValue)
    MAXME(labelFrameRateValue)
    MAXME(labelVideoBitrateValue)
    MAXME(labelVideoDurationValue)
    MAXME(labelACodecName)
    MAXME(labelChannelsValue)
    MAXME(labelAudioBitrateValue)
    MAXME(labelVBRDetected)
    MAXME(labelFrequencyValue)
    MAXME(labelAudioDurationValue)

    ui.label4CCValue->setMinimumWidth(w);
    ui.labelExtraDataSizeValue->setMinimumWidth(w);
    ui.labelACodecName->setMinimumWidth(w);

    // work around dialog window not resized properly at least on macOS
    resize(sizeHint());
}

#define ADDCATEGORY(a) props += "\n=====================================================\n" \
                             +  ui.a->title() \
                             +  "\n=====================================================\n";
#define ADDNAMEVALUE(a,b) { \
    int l = ui.a->text().size(); \
    props += ui.a->text(); \
    do { props += "\t"; l += 8; } while(l < 24); /* assuming tab width of 8 characters */ \
    props += ui.b->text() + "\n"; \
}

/**
    \fn propsCopyToClipboard
    \brief Copy a list of property-value pairs to clipboard
*/
void propWindow::propsCopyToClipboard(void)
{
    QString props;

    ADDCATEGORY(groupBoxVideo)

    ADDNAMEVALUE(label4CC,label4CCValue)
    ADDNAMEVALUE(labelImageSize,labelImageSizeValue)
    ADDNAMEVALUE(labelAspectRatio,labelAspectRatioValue)

    ADDNAMEVALUE(labelFrameRate,labelFrameRateValue)
    ADDNAMEVALUE(labelVideoBitrate,labelVideoBitrateValue)
    ADDNAMEVALUE(labelVideoDuration,labelVideoDurationValue)

    ADDCATEGORY(groupBoxExtradata)

    ADDNAMEVALUE(labelExtraDataSize,labelExtraDataSizeValue)

    if(gotExtraData)
        ADDNAMEVALUE(labelExtraData,lineEditExtraData)

    ADDCATEGORY(groupBoxAudio)

    if(gotAudio)
    {
        ADDNAMEVALUE(labelACodec,labelACodecName)
        ADDNAMEVALUE(labelChannels,labelChannelsValue)
        ADDNAMEVALUE(labelAudioBitrate,labelAudioBitrateValue)

        //ADDNAMEVALUE(labelVBR,labelVBRDetected)
        ADDNAMEVALUE(labelFrequency,labelFrequencyValue)
        ADDNAMEVALUE(labelAudioDuration,labelAudioDurationValue)
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
