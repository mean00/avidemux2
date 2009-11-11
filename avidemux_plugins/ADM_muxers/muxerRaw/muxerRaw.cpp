/***************************************************************************
            \file            muxerRaw
            \brief           i/f to lavformat mpeg4 muxer
                             -------------------
    
    copyright            : (C) 2008 by mean
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
#include "ADM_default.h"
#include "fourcc.h"
#include "muxerRaw.h"
#include "DIA_coreToolkit.h"
#define ADM_NO_PTS 0xFFFFFFFFFFFFFFFFLL // FIXME


#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif


/**
    \fn     muxerRaw
    \brief  Constructor
*/
muxerRaw::muxerRaw() 
{
    file=NULL;
};
/**
    \fn     muxerRaw
    \brief  Destructor
*/

muxerRaw::~muxerRaw() 
{
    printf("[RAW] Destructing\n");
    close();
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerRaw::open(const char *fil, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{

    vStream=s;
    file=ADM_fopen(fil,"w");
    if(!file) 
    {
        printf("[RawMuxer] Cannot open %s\n",fil);
        return false;
    }
    return true;
}

/**
    \fn save
*/
bool muxerRaw::save(void) 
{
    printf("[RAW] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[bufSize];
    uint32_t len,flags;
    uint64_t pts,dts,rawDts;
    uint64_t lastVideoDts=0;
    int written=0;
    bool result=true;

    initUI("Saving raw video");
    while(true==vStream->getPacket(&len, buffer, bufSize,&pts,&dts,&flags))
    {
        if(dts==ADM_NO_PTS)
            dts=lastVideoDts+videoIncrement;
        if(updateUI(dts)==false)
        {
            result=false;
            goto abt;
        }
        fwrite(buffer,len,1,file);
        written++;

    }
abt:
    closeUI();
    delete [] buffer;
    fclose(file);
    file=NULL;
    printf("[RAW] Wrote %d frames \n",written);
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerRaw::close(void) 
{
    if(file)
    {
        fclose(file);
        file=NULL;
    }
    return true;
}

//EOF



