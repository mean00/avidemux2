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
                case ADM_MP4_TRAF: parseTraf(son,tom.getStartPos());break;
                
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
bool MP4Header::parseTraf(adm_atom &tom,uint64_t moofStart)
{        
        ADMAtoms id;
        uint32_t container;
        aprintf("[TRAF]\n");
        uint32_t trafFlags=0;
        mp4TrafInfo info;
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
                    parseTrun(son,info);
                    break;
                case ADM_MP4_TFHD:
                {
                    trafFlags=son.read32()&0xfffff;
                    aprintf("[TFHD] flags =0x%x\n",trafFlags);
#define TRAF_INFO(a,b,s)   if(trafFlags&a)  {info.b=son.read##s();aprintf("TFHD:"#b"=%d\n",info.b);}
                    
                    TRAF_INFO(1,base,64);
                    TRAF_INFO(2,sampleDesc,32);
                    TRAF_INFO(8,defaultDuration,32);
                    TRAF_INFO(0x10,defaultSize,32);
                    TRAF_INFO(0x20,defaultFlags,32);                    
                   
                    if(trafFlags&0x10000) {aprintf("Empty duration\n");info.emptyDuration=true;}
                    if(trafFlags&0x20000) 
                    {
                            
                            info.baseIsMoof=true;
                            info.base=moofStart;
                            aprintf("base is moof at %llx\n",info.base);
                    }
                }
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
bool MP4Header::parseTrun(adm_atom &tom,const mp4TrafInfo &info)
{
    uint32_t version_flags=tom.read32()& 0xfffff;
    aprintf("[TRUN] Flags=%x\n",version_flags);
    uint32_t count=tom.read32();
    int64_t  firstOffset=info.base;
    uint32_t  firstSampleFlags=0;
    if(version_flags & 0x1)
    {
            firstOffset+=tom.read32(); // Signed!
    }
    if(version_flags & 0x4)    
            firstSampleFlags=tom.read32(); // Signed!
    else 
            firstSampleFlags=info.defaultFlags;
   
    aprintf("[TRUN] count=%d, offset=0x%x,synth=0x%x, flags=%x\n",count,firstOffset,firstOffset+info.base,firstSampleFlags);
    for(int i=0;i<count;i++)
    {
       mp4Fragment frag;
        
#define FLAGS(a,b,c) if(version_flags & a)         frag.b=tom.read32(); else frag.b=c;
        FLAGS(0x100,duration,info.defaultDuration);        
        FLAGS(0x200,size,info.defaultSize);        
        FLAGS(0x400,flags,firstSampleFlags); // FIXME
        
        frag.offset=firstOffset;
        
            
        firstOffset+=frag.size;
        FLAGS(0x800,composition,0);
        aprintf("[TRUN] duration=%d, size=%d,flags=%x,composition=%d\n",frag.duration,frag.size,frag.flags,frag.composition);
        fragments.push_back(frag);
        
    }
    tom.skipAtom();
    return true;
}
/**
 * 
 * @return 
 */
bool MP4Header::indexFragments()
{
    MP4Track *trk=_tracks;
    trk->nbIndex=fragments.size();
    trk->index=new MP4Index[trk->nbIndex];
    for(int i=0;i<trk->nbIndex;i++)
    {
        MP4Index *dex=trk->index+i;
        dex->offset=fragments[i].offset;
        dex->size=fragments[i].size;
        dex->pts=i;
        dex->dts=i;
        dex->intra=0;    
        aprintf("[FRAG] offset=0x%llx size=%d\n",dex->offset,dex->size);
    }
    MP4Index *ff=trk->index;
    ff->intra=AVI_KEY_FRAME;
    ff->dts=0;
    ff->pts=0;
    _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].nbIndex;
    return true;
}
// EOF


