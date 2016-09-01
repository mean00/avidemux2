/***************************************************************************
           \file               ADM_edFrameType.cpp  -  description
           \brief              Rederive Frame type if needed. Works only with a few
                                                    codecs (mpeg1/2/4)

    
    copyright            : (C) 2002/2009 by mean
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
#include "ADM_default.h"

#include "ADM_edit.hxx"
#include "DIA_coreToolkit.h"
#include "ADM_frameType.h"
#include "DIA_working.h"
 
/**
    \fn rederiveFrameType
    \brief 
*/
bool        ADM_Composer::rederiveFrameType(vidHeader *demuxer)
{
    uint32_t flags,flagsDecoded;
    uint8_t *buffer;
    aviInfo info;

    demuxer->getVideoInfo (&info);
    frameIdentifier id=ADM_getFrameIdentifier(info.fcc);
    if(!id)
    {
        printf("[Editor]Cannot get identifer for this fourcc\n");
        return true;
    }
    uint32_t size=demuxer->getMainHeader()->dwWidth;
    size*=demuxer->getMainHeader()->dwHeight*3;
    buffer=new uint8_t[size];

    ADMCompressedImage img;
    img.data=buffer;

    int max=100;
    if(max>info.nb_frames)max=info.nb_frames;

    int nbOk=0,nbKo=0;

    for(int i=0;i<max;i++)
    {
        demuxer->getFlags(i,&flags);
        demuxer->getFrame(i,&img);
        flagsDecoded=id(img.dataLength,img.data);
        if(flagsDecoded==flags) nbOk++;
            else nbKo++;
    }
    printf("[Editor] Muxer has %d frames right, %d frames wrong\n",nbOk,nbKo);
    if(!nbKo)  
    {
        delete [] buffer;
        buffer=NULL;
        return true;
    }
    // Demuxer is wrong, rederive all frames...
    DIA_workingBase *work=createWorking(QT_TRANSLATE_NOOP("adm","Updating frametype"));
    for(int i=0;i<info.nb_frames;i++)
    {
        work->update(i,info.nb_frames);
        demuxer->getFrame(i,&img);
        flagsDecoded=id(img.dataLength,img.data);
        demuxer->setFlag(i,flagsDecoded);
    }
    delete [] buffer;
    buffer=NULL;
    delete work;
    return false;
}
// EOF
