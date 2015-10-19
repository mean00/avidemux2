/***************************************************************************
                         ADM_vs
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
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
#pragma once 


extern "C"
{
#undef __cplusplus
#include "VSScript.h"
#include "VSHelper.h"
#define __cplusplus
}
#include "../../ADM_coreSocket/include/ADM_coreAvsProtocol.h"

/**
    \Class vsHeader
    \brief Flash demuxer

*/
class vapourSynthProxy      
{
public:
                      vapourSynthProxy();
                      ~vapourSynthProxy();
   bool              run(int port, const char *name);
protected:

    uint8_t        *_buffer;

protected:
    VSScript       *_script;
    int            _outputIndex;
    VSNodeRef       *_node;
    avsyInfo        _info;
    bool            manageSlave(avsSocket *slave,const VSVideoInfo *vi);
    bool            packFrame( const VSVideoInfo *vi,const VSFrameRef *frame);
    bool            fillInfo( const VSVideoInfo *vi);
    void            abort(void);
};


