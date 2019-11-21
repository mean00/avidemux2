/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_vp9.h"
#include "vp9_encoder.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

extern vp9_encoder vp9Settings;

/**
 *  \fn vp9EncoderConfigure
 *  \brief Configuration UI for VP9 encoder
 */
bool vp9EncoderConfigure(void)
{
    vp9_encoder *cfg = &vp9Settings;
    int spdi = cfg->speed - 9;

    diaMenuEntry dltype[]={
        {REALTIME,QT_TRANSLATE_NOOP("vp9encoder","Realtime")},
        {GOOD_QUALITY,QT_TRANSLATE_NOOP("vp9encoder","Good quality")},
        {BEST_QUALITY,QT_TRANSLATE_NOOP("vp9encoder","Best quality")}
    };
#define PX(x) &(cfg->x)
    diaElemBitrate bitrate(PX(ratectl),NULL);
    diaElemReadOnlyText advice(QT_TRANSLATE_NOOP("vp9encoder","For optimal quality, select 2-pass average bitrate mode and set target bitrate to zero"),NULL);

    diaElemMenu menudl(PX(deadline),QT_TRANSLATE_NOOP("vp9encoder","Deadline"),3,dltype);
    diaElemInteger speedi(&spdi,QT_TRANSLATE_NOOP("vp9encoder","Speed"),-9,9);
    diaElemUInteger conc(PX(nbThreads),QT_TRANSLATE_NOOP("vp9encoder","Threads"),1,VP9_ENC_MAX_THREADS);
    diaElemToggle thrmatic(PX(autoThreads),QT_TRANSLATE_NOOP("vp9encoder","Use as many threads as CPU cores"));

    thrmatic.link(0,&conc);

    diaElemUInteger gopsize(PX(keyint),QT_TRANSLATE_NOOP("vp9encoder","GOP Size"),0,1000);
    diaElemToggle range(PX(fullrange),QT_TRANSLATE_NOOP("vp9encoder","Use full color range"));

    diaElemFrame frameEncMode(QT_TRANSLATE_NOOP("vp9encoder","Encoding Mode"));
    frameEncMode.swallow(&bitrate);
    frameEncMode.swallow(&advice);

    diaElemFrame frameEncSpeed(QT_TRANSLATE_NOOP("vp9encoder","Speed vs Quality"));
    frameEncSpeed.swallow(&speedi);
    frameEncSpeed.swallow(&conc);
    frameEncSpeed.swallow(&thrmatic);
    frameEncSpeed.swallow(&menudl);

    diaElemFrame frameIdr(QT_TRANSLATE_NOOP("vp9encoder","Keyframes"));
    frameIdr.swallow(&gopsize);

    diaElemFrame frameMisc(QT_TRANSLATE_NOOP("vp9encoder","Miscellaneous"));
    frameMisc.swallow(&range);

    diaElem *dialog[] = {&frameEncMode,&frameEncSpeed,&frameIdr,&frameMisc};
    if(diaFactoryRun(QT_TRANSLATE_NOOP("vp9encoder","libvpx VP9 Encoder Configuration"),4,dialog))
    {
        cfg->speed=spdi+9;
        return true;
    }
    return false;
}

// EOF

