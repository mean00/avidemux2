/***************************************************************************
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
    
      Handle flvTrack/index
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


#include "fourcc.h"


#include "ADM_flv.h"

flvTrak::flvTrak(int nb)
{
  memset(this,0,sizeof(flvTrak)); 
  _index=new flvIndex[nb];
  _indexMax=nb;
}
flvTrak::~flvTrak()
{
  if(_index) delete [] _index;
  _index=NULL;
}
/**
      \fn grow
      \brief reallocate index if needed
*/
uint8_t flvTrak::grow(void)
{
   if(_indexMax==_nbIndex)
    {
      // Grow
      flvIndex *ix=new flvIndex[ _indexMax*2];
      memcpy(ix,_index,sizeof(flvIndex)*_nbIndex);
      delete [] _index;
      _index=ix;
      _indexMax*=2;
    }
    return 1;
}
//EOF
