//
/**/
/***************************************************************************
                          DIA_hue
                             -------------------

                           Ui for hue & sat

    begin                : 08 Apr 2005
    copyright            : (C) 2004/5 by mean
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



#include "config.h"


#include <string.h>
#include <stdio.h>
#include <math.h>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_assert.h"

/**
    \fn DIA_builtin(void)
    \brief Display component that are built in. They are detected at configure time.

*/

uint8_t DIA_builtin(void)
{
  uint32_t altivec=0,freetype=0,win32=0;
  uint32_t sdl=0,oss=0,xvideo=0,x86=0,x86_64=0;
  uint32_t adm_powerpc=0,adm_gettext=0,adm_fontconfig=0;
#ifdef USE_FONTCONFIG
  adm_fontconfig=1;
#endif
#ifdef ADM_CPU_ALTIVEC
        altivec=1;
#endif

#ifdef USE_FREETYPE
        freetype=1;
#endif

#ifdef __WIN32
        win32=1;
#endif

#ifdef USE_SDL
	sdl=1;
#endif

#ifdef USE_XV
	xvideo=1;
#endif
#ifdef ADM_CPU_X86
	x86=1;
#endif
#ifdef ADM_CPU_X86_64
	x86_64=1;
#endif
#ifdef ADM_CPU_PPC
	adm_powerpc=1;
#endif
#ifdef HAVE_GETTEXT
	adm_gettext=1;
#endif
    

    diaElemNotch tFontConfig(adm_fontconfig, QT_TR_NOOP("Fontconfig"));
	diaElemNotch tFreetype(freetype, QT_TR_NOOP("FreeType 2"));
	diaElemNotch tGettext(adm_gettext, QT_TR_NOOP("Gettext"));
    diaElemNotch tSdl(sdl, QT_TR_NOOP("SDL"));
	diaElemNotch tXvideo(xvideo, QT_TR_NOOP("XVideo"));

	diaElemNotch tAltivec(altivec, QT_TR_NOOP("AltiVec"));
	diaElemNotch tPowerPc(adm_powerpc, QT_TR_NOOP("PowerPC"));
	diaElemNotch tX86(x86, QT_TR_NOOP("x86"));
	diaElemNotch tX86_64(x86_64, QT_TR_NOOP("x86-64"));


	diaElem *libsElems[] = { &tFontConfig, &tFreetype, &tGettext, &tSdl, &tXvideo};
	diaElem *CPUElems[] = {&tAltivec, &tPowerPc, &tX86, &tX86_64};

	
	diaElemTabs tabLibs(QT_TR_NOOP("Libraries"), 5, libsElems);
	diaElemTabs tabCPU(QT_TR_NOOP("CPU"), 4, CPUElems);

	diaElemTabs *tabs[] = {&tabLibs, &tabCPU};

    diaFactoryRunTabs(QT_TR_NOOP("Built-in Support"), 2, tabs);

    return 1;
}
