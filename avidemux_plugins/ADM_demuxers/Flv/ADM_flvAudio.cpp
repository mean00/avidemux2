/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include "ADM_Video.h"

#include <string.h>
#include <math.h>

#include "ADM_flv.h"

/**
    \fn getExtraData
*/
bool      ADM_flvAccess::getExtraData(uint32_t *l, uint8_t **d)
{
#if 0
        uint8_t dd[2]={0x12,0x10};
        *l=2;
        *d=dd;
        return true;
#else
    *l=_track->extraDataLen;
    *d=_track->extraData;
#endif
    return true;
}   

/**
    \fn ADM_audioAccess
    \brief Constructor

*/
ADM_flvAccess::ADM_flvAccess(const char *name,flvTrak *track) : ADM_audioAccess()
{
    _fd=ADM_fopen(name,"rb");
    ADM_assert(_fd);
    _track=track;
    goToBlock(0);
    currentBlock=0;
    _msg_counter = 0;
    _msg_ratelimit = new ADMCountdown(200);
    _msg_ratelimit->reset();
}
/**
    \fn ADM_audioAccess
    \brief Destructor

*/
ADM_flvAccess::~ADM_flvAccess()
{
    if(_fd) fclose(_fd);
    _fd=NULL;
    if(_msg_ratelimit)
        delete _msg_ratelimit;
    _msg_ratelimit = NULL;
}
/**
    \fn getDurationInUs

*/ 
uint64_t  ADM_flvAccess::getDurationInUs(void)
{
    if(!_track->_nbIndex) return 0;
    // ms -> us
    uint64_t dur=_track->_index[_track->_nbIndex-1].dtsUs;
    
    return dur;
}
/**
    \fn goToTime
    \brief
*/
bool      ADM_flvAccess::goToTime(uint64_t timeUs)
{

uint64_t target=timeUs;
uint64_t mstime=target;
uint32_t _nbClusters=_track->_nbIndex;

      // First identify the cluster...
      // Special case when first chunk does not start at 0
      if(_nbClusters && mstime<_track->_index[0].dtsUs)
      {
            goToBlock(0);
            return true;
      }
      int clus=-1;
            for(int i=0;i<_nbClusters-1;i++)
            {
              if(target>=_track->_index[i].dtsUs && target<_track->_index[i+1].dtsUs)
              {
                clus=i;
                i=_nbClusters; 
              }
            }
            if(clus==-1) clus=_nbClusters-1; // Hopefully in the last one
            goToBlock(clus);
            return true;
}
/**
    \fn getPacket
*/
bool      ADM_flvAccess::getPacket(uint8_t *buffer, uint32_t *osize, uint32_t maxSize,uint64_t *dts)
{
    flvIndex *x;
    if(false==goToBlock(currentBlock))
    {
        if(_msg_ratelimit->done())
        {
            if(_msg_counter)
            {
                printf("[ADM_flvAccess::getPacket] Packet out of bounds (message repeated %" PRIu32" times)\n",_msg_counter);
                _msg_counter = 0;
            }else
            {
                printf("[ADM_flvAccess::getPacket] Packet out of bounds\n");
            }
            _msg_ratelimit->reset();
        }else
        {
            _msg_counter++;
        }
        return false;
    }
    x=&(_track->_index[currentBlock]);
    fread(buffer,x->size,1,_fd);
    *osize=x->size;
    *dts=((uint64_t)x->dtsUs);
    
    currentBlock++;
    _msg_counter = 0;
    return 1;
}
/**
    \fn goToBlock
*/
bool      ADM_flvAccess::goToBlock(uint32_t block)
{
    if(block>=_track->_nbIndex)
    {
        if(_msg_ratelimit->done())
        {
            if(_msg_counter)
                printf("[ADM_flvAccess::goToBlock] Exceeding max cluster: asked: %u max: %u (message repeated %" PRIu32" times)\n",block,_track->_nbIndex,_msg_counter);
            else
                printf("[ADM_flvAccess::goToBlock] Exceeding max cluster: asked: %u max: %u\n",block,_track->_nbIndex);
        }
        return false;  // FIXME
    }
    currentBlock=block;
    fseeko(_fd,_track->_index[currentBlock].pos,SEEK_SET);
    return 1;
}

//EOF
