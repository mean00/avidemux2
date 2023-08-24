/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_av1AomEnc.h"
#include "av1aom_encoder.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

extern av1aom_encoder encoderSettings;

/**
 *  \fn av1AomEncoderConfigure
 *  \brief Configuration UI for AV1 encoder
 */
bool av1AomEncoderConfigure(void)
{
    av1aom_encoder *cfg = &encoderSettings;
#define PX(x) &(cfg->x)
    diaElemBitrate bitrate(PX(ratectl), NULL);
    diaElemUInteger speedu(PX(speed),
        QT_TRANSLATE_NOOP("aomencoder","Speed"),
        0, 6,
        QT_TRANSLATE_NOOP("aomencoder", "Lower values favor quality over speed."));
    diaElemUInteger concu(PX(nbThreads),
        QT_TRANSLATE_NOOP("aomencoder","Threads"),
        1, AV1_ENC_MAX_THREADS,
        QT_TRANSLATE_NOOP("aomencoder", "Maximum number of threads allowed, the encoder may use less at its discretion."));
    diaElemToggle thrmatic(PX(autoThreads),
        QT_TRANSLATE_NOOP("aomencoder", "Use at most as many threads as CPU cores"));

    thrmatic.link(0, &concu);

    diaElemUInteger gopsize(PX(keyint),
        QT_TRANSLATE_NOOP("aomencoder", "GOP Size"),
        0, 1000);
    diaElemToggle range(PX(fullrange),
        QT_TRANSLATE_NOOP("aomencoder", "Treat input as having full color range"));

    diaElemFrame frameEncMode(QT_TRANSLATE_NOOP("aomencoder", "Encoding Mode"));
    frameEncMode.swallow(&bitrate);

    diaElemFrame frameEncSpeed(QT_TRANSLATE_NOOP("aomencoder", "Speed vs Quality"));
    frameEncSpeed.swallow(&speedu);
    frameEncSpeed.swallow(&concu);
    frameEncSpeed.swallow(&thrmatic);

    diaElemFrame frameIdr(QT_TRANSLATE_NOOP("aomencoder", "Keyframes"));
    frameIdr.swallow(&gopsize);

    diaElemFrame frameMisc(QT_TRANSLATE_NOOP("aomencoder","Miscellaneous"));
    frameMisc.swallow(&range);

    diaElem *dialog[] = { &frameEncMode, &frameEncSpeed, &frameIdr, &frameMisc };
#define NB_ELEM(x) sizeof(x)/sizeof(diaElem *)
    if(diaFactoryRun(QT_TRANSLATE_NOOP("aomencoder","libaom AV1 Encoder Configuration"), NB_ELEM(dialog), dialog))
        return true;

    return false;
}

// EOF

