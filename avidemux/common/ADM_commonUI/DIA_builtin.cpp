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
  uint32_t adm_vdpau=0;
#ifdef USE_VDPAU
    adm_vdpau=1;
#endif
#ifdef USE_FONTCONFIG
  adm_fontconfig=1;
#endif

#ifdef USE_FREETYPE
        freetype=1;
#endif

#ifdef _WIN32
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
#ifdef HAVE_GETTEXT
	adm_gettext=1;
#endif
    

    diaElemNotch tFontConfig(adm_fontconfig, QT_TRANSLATE_NOOP("adm","Fontconfig"));
	diaElemNotch tFreetype(freetype, QT_TRANSLATE_NOOP("adm","FreeType 2"));
	diaElemNotch tGettext(adm_gettext, QT_TRANSLATE_NOOP("adm","Gettext"));
    diaElemNotch tSdl(sdl, QT_TRANSLATE_NOOP("adm","SDL"));
	diaElemNotch tXvideo(xvideo, QT_TRANSLATE_NOOP("adm","XVideo"));
    diaElemNotch tVdpau(adm_vdpau, QT_TRANSLATE_NOOP("adm","VDPAU"));

	diaElemNotch tX86(x86, QT_TRANSLATE_NOOP("adm","x86"));
	diaElemNotch tX86_64(x86_64, QT_TRANSLATE_NOOP("adm","x86-64"));


	diaElem *libsElems[] = { &tFontConfig, &tFreetype, &tGettext, &tSdl, &tXvideo,&tVdpau};
	diaElem *CPUElems[] = {&tX86, &tX86_64};

	
	diaElemTabs tabLibs(QT_TRANSLATE_NOOP("adm","Libraries"), 6, libsElems);
	diaElemTabs tabCPU(QT_TRANSLATE_NOOP("adm","CPU"), 2, CPUElems);

	diaElemTabs *tabs[] = {&tabLibs, &tabCPU};

    diaFactoryRunTabs(QT_TRANSLATE_NOOP("adm","Built-in Support"), 2, tabs);

    return 1;
}
