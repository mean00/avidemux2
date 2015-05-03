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

#if 1
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
 * @param desc
 * @return 
 */
int  MP4Header::lookupIndex(int desc)
{
    for(int i=0;i<=this->nbAudioTrack;i++)
        if(_tracks[i].id==desc)
            return i;
    return -1;
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
        int trackIndex=-1;
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
                    if(trackIndex==-1)
                        break;
                    else
                        parseTrun(trackIndex,son,info);
                    break;
                case ADM_MP4_TFHD:
                {
                    trafFlags=son.read32()&0xfffff;
                    
                    info.trackID=son.read32();
                    aprintf("[TFHD] Track=%d flags =0x%x\n",info.trackID,trafFlags);
#define TRAF_INFO(a,b,s)   if(trafFlags&a)  {info.b=son.read##s();aprintf("TFHD:"#b"=%d\n",(int)info.b);}
                    
                    TRAF_INFO(1,baseOffset,64);
                    TRAF_INFO(2,sampleDesc,32);
                    TRAF_INFO(8,defaultDuration,32);
                    TRAF_INFO(0x10,defaultSize,32);
                    TRAF_INFO(0x20,defaultFlags,32);                    
                   
                    if(trafFlags&0x10000) {aprintf("Empty duration\n");info.emptyDuration=true;}
                    if(trafFlags&0x20000) 
                    {
                            
                            info.baseIsMoof=true;
                            info.baseOffset=moofStart;
                            aprintf("base is moof at %llx\n",(long long int)info.baseOffset);
                    }
                    trackIndex=lookupIndex(info.sampleDesc);
                    if(-1==trackIndex)
                    {
                        ADM_warning("Cannot find track for %d\n",info.sampleDesc);
                    }
                }
                case ADM_MP4_TFDT:
                {
                    uint32_t version=son.read();
                    aprintf("[TRAF]Found atom %s version=%d\n",fourCC::tostringBE(son.getFCC()),version);
                    son.read();son.read();son.read();
                    if(version==1)
                        info.baseDts=son.read64();
                    else
                        info.baseDts=son.read32();
                    aprintf("[TFDT] Base DTS=%ld\n",(long int)info.baseDts);
                }   
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
bool MP4Header::parseTrun(int trackNo,adm_atom &tom,const mp4TrafInfo &info)
{
    uint32_t flags=tom.read32()& 0xfffffff;
    aprintf("[TRUN] Flags=%x\n",flags);
    uint32_t count=tom.read32();
    int64_t  firstOffset=info.baseOffset;
    uint32_t  firstSampleFlags=0;
    std::vector <mp4Fragment>   &fragList=_tracks[trackNo].fragments;
    if(flags & 0x1)
    {
            firstOffset+=tom.read32()&0xffff; // Signed!
    }
    if(flags & 0x4)    
            firstSampleFlags=tom.read32(); // Signed!
    else 
            firstSampleFlags=info.defaultFlags;
   
    aprintf("[TRUN] count=%d, offset=0x%x,synth=0x%x, flags=%x\n",count,firstOffset,(long long int)firstOffset+info.baseOffset,firstSampleFlags);
    for(int i=0;i<count;i++)
    {
       mp4Fragment frag;
        
#define FLAGS(a,b,c) {if(flags & a)         frag.b=tom.read32(); else frag.b=c;}
        FLAGS(0x100,duration,info.defaultDuration);        
        FLAGS(0x200,size,info.defaultSize);    
        if(!i)
            FLAGS(0x400,flags,firstSampleFlags)
        else
            FLAGS(0x400,flags,info.defaultFlags)
        
        frag.offset=firstOffset;
        firstOffset+=frag.size;
        FLAGS(0x800,composition,0);
        aprintf("[TRUN] duration=%d, size=%d,flags=%x,composition=%d\n",frag.duration,frag.size,frag.flags,frag.composition);
        fragList.push_back(frag);
        
    }
    tom.skipAtom();
    return true;
}
/**
 * 
 * @return 
 */
bool MP4Header::indexVideoFragments(int trackNo)
{
     MP4Track *trk=&_tracks[trackNo];
     std::vector <mp4Fragment>   &fragList=_tracks[trackNo].fragments;
    trk->nbIndex=fragList.size();
    trk->index=new MP4Index[trk->nbIndex];
    uint64_t sum=0;
    int intra=0;
    for(int i=0;i<trk->nbIndex;i++)
    {
        MP4Index *dex=trk->index+i;
        dex->offset=fragList[i].offset;
        dex->size=fragList[i].size;
        
        double dts=sum;
        double ctts=fragList[i].composition;
        dts=sum;
        
        dts=dts/_videoScale;
        dts*=1000000.;
        ctts=ctts/_videoScale;
        ctts*=1000000.;
        dex->dts=dts;
        dex->pts=dex->dts+ctts;
        if(!(fragList[i].flags &(0x00010000|0x01000000)))
        {
            dex->intra=AVI_KEY_FRAME;
            intra++;
        }
        else
            dex->intra=0;    
        sum+=fragList[i].duration;
        aprintf("[FRAG] offset=0x%llx size=%d dts=%s pts=%s\n",dex->offset,(int)dex->size,ADM_us2plain(dex->dts),ADM_us2plain(dex->pts));
    }
    printf("Found %d intra\n");
    MP4Index *ff=trk->index;
    ff->intra=AVI_KEY_FRAME;   
    _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].nbIndex;
    fragList.clear();
    return true;
}

/**
 * 
 * @return 
 */
bool MP4Header::indexAudioFragments(int trackNo)
{
     MP4Track *trk=&_tracks[trackNo];
     std::vector <mp4Fragment>   &fragList=_tracks[trackNo].fragments;
    trk->nbIndex=fragList.size();
    trk->index=new MP4Index[trk->nbIndex];
    uint64_t sum=0;
    for(int i=0;i<trk->nbIndex;i++)
    {
        MP4Index *dex=trk->index+i;
        dex->offset=fragList[i].offset;
        dex->size=fragList[i].size;
        
        double dts=sum;
        dts=sum;
        dex->dts=dts;
        dex->pts=dex->dts+fragList[i].composition*10;
        dex->intra=0;    
        sum+=fragList[i].duration;
        aprintf("[FRAG] offset=0x%llx size=%d dts=%s pts=%s\n",dex->offset,(int)dex->size,ADM_us2plain(dex->dts),ADM_us2plain(dex->pts));
    }   
    return true;
}
// EOF


