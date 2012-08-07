/***************************************************************************
    \fn DIA_working.cpp 
    \brief UI that handles working state with cancel & percent

    copyright            : (C) 2003/2009 by mean fixounet@free.fr



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DIA_AUDIO_TRACKS__
#define __DIA_AUDIO_TRACKS__

#include "ADM_coreUI6_export.h"

class PoolOfAudioTracks;
class ActiveAudioTracks;
/**
    \class DIA_audioTrackBase
*/
class DIA_audioTrackBase
{
    protected :
            PoolOfAudioTracks       *_pool;
            ActiveAudioTracks       *_srcActive;  
    public:
            void 		*_priv;
                                DIA_audioTrackBase( PoolOfAudioTracks *pool, ActiveAudioTracks *active )
                                {
                                        _pool=pool;
                                        _srcActive=active;
                                };
            virtual		~DIA_audioTrackBase(){};
            virtual   bool      run(void)=0;
            
};
ADM_COREUI6_EXPORT DIA_audioTrackBase *createAudioTrack(PoolOfAudioTracks *pool, ActiveAudioTracks *active );
#endif
// EOF

