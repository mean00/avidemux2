/***************************************************************************

    copyright            : (C) 2007/2015 by mean
    email                : fixounet@free.fr
 * https://arashafiei.wordpress.com/2012/11/13/quick-dash/
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

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"
#include "DIA_coreToolkit.h"
#include "ADM_getbits.h"
#include "ADM_coreUtils.h"
#include "ADM_mp4Tree.h"
#include "ADM_vidMisc.h"

#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif
/**
      \fn parseDash
      \brief Parse sidx header
*/
bool MP4Header::parseMoof(adm_atom &tom)
{        
        ADMAtoms id;
        uint32_t container;
        aprintf("---\n");
        while(!tom.isDone())
        {
            adm_atom son(&tom);
            if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
            {
              aprintf("[MOOF]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
              son.skipAtom();
              continue;
            }
            switch(id)
            {
                case ADM_MP4_MFHD: son.skipAtom();break;
                case ADM_MP4_TRAF: parseTraf(son);break;
                
            }
            aprintf("[MOOF]Found atom %s \n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
        }     
        tom.skipAtom();
        aprintf("---\n");
        return false;
}
/**
 * 
 * @param tom
 * @return 
 */
bool MP4Header::parseTraf(adm_atom &tom)
{        
        ADMAtoms id;
        uint32_t container;
        aprintf("[TRAF]\n");
        while(!tom.isDone())
        {
            adm_atom son(&tom);
            if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
            {
              aprintf("[MOOF]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
              son.skipAtom();
              continue;
            }
            switch(id)
            {
                case ADM_MP4_TRUN:
                    parseTrun(son);
                    break;
                case ADM_MP4_TFHD:
                case ADM_MP4_TFDT:
                    aprintf("[TRAF]Found atom %s \n",fourCC::tostringBE(son.getFCC()));
                    break;
            }
           
            aprintf("[MOOF]Found atom %s \n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
        }     
        tom.skipAtom();
        aprintf("[/TRAF]\n");
        return false;
}
/**
 * \fn parseTrun
 * @param tom
 * @return 
 */
bool MP4Header::parseTrun(adm_atom &tom)
{
    uint32_t version_flags=tom.read32()& 0xfffff;
    aprintf("[TRUN] Flags=%x\n",version_flags);
    uint32_t count=tom.read32();
    int32_t  offset=0;
    uint32_t  firstSampleFlags=0;
    if(version_flags & 0x1)
            offset=tom.read32(); // Signed!
    if(version_flags & 0x4)    
            firstSampleFlags=tom.read32(); // Signed!
    aprintf("[TRUN] count=%d, offset=0x%x,flags=%x\n",count,offset,firstSampleFlags);
    for(int i=0;i<count;i++)
    {
        uint32_t duration;
        uint32_t size;
        uint32_t flags;
        uint32_t composition; // signed ?
        
#define FLAGS(a,b,c) if(version_flags & a)         b=tom.read32(); else b=c;
        FLAGS(0x100,duration,0);
        FLAGS(0x200,size,0);
        FLAGS(0x400,flags,firstSampleFlags);
        FLAGS(0x800,composition,0);
        aprintf("[TRUN] duration=%d, size=%d,flags=%x,composition=%d\n",duration,size,flags,composition);
    }
    tom.skipAtom();
    return true;
}
// EOF


