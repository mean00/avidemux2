/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_vp8.h"
#include "vp8_encoder.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

extern vp8_encoder vp8Settings;

/**
 *  \fn vp8EncoderConfigure
 *  \brief Configuration UI for VP8 encoder
 */
bool vp8EncoderConfigure(void)
{
    vp8_encoder *cfg = &vp8Settings;
    int spdi = cfg->speed - 16;

    diaMenuEntry dltype[]={
        {REALTIME,QT_TRANSLATE_NOOP("vp8encoder","Realtime"),NULL},
        {GOOD_QUALITY,QT_TRANSLATE_NOOP("vp8encoder","Good quality"),NULL},
        {BEST_QUALITY,QT_TRANSLATE_NOOP("vp8encoder","Best quality"),NULL}
    };
#define PX(x) &(cfg->x)
    diaElemBitrate bitrate(PX(ratectl),NULL);
    diaElemMenu menudl(PX(deadline),QT_TRANSLATE_NOOP("vp8encoder","Deadline"),3,dltype);
    diaElemInteger speedi(&spdi,QT_TRANSLATE_NOOP("vp8encoder","Speed"),-16,16);
    diaElemUInteger conc(PX(nbThreads),QT_TRANSLATE_NOOP("vp8encoder","Threads"),1,VP8_ENC_MAX_THREADS);
    diaElemToggle thrmatic(PX(autoThreads),QT_TRANSLATE_NOOP("vp8encoder","Use as many threads as CPU cores"));

    thrmatic.link(0,&conc);

    diaElemUInteger gopsize(PX(keyint),QT_TRANSLATE_NOOP("vp8encoder","GOP Size"),0,1000);

    diaElemFrame frameEncMode(QT_TRANSLATE_NOOP("vp8encoder","Encoding Mode"));
    frameEncMode.swallow(&bitrate);

    diaElemFrame frameEncSpeed(QT_TRANSLATE_NOOP("vp8encoder","Speed vs Quality"));
    frameEncSpeed.swallow(&speedi);
    frameEncSpeed.swallow(&conc);
    frameEncSpeed.swallow(&thrmatic);
    frameEncSpeed.swallow(&menudl);

    diaElemFrame frameIdr(QT_TRANSLATE_NOOP("vp8encoder","Keyframes"));
    frameIdr.swallow(&gopsize);

    diaElem *dialog[] = {&frameEncMode,&frameEncSpeed,&frameIdr};
    if(diaFactoryRun(QT_TRANSLATE_NOOP("vp8encoder","libvpx VP8 Encoder Configuration"),3,dialog))
    {
        cfg->speed=spdi+9;
        return true;
    }
    return false;
}

// EOF

