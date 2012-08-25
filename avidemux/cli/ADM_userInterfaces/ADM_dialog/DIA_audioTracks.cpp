/***************************************************************************
    copyright            : (C) 2001 by mean
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

#include "ADM_inttype.h"
#include <string>
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_edit.hxx"
#include "DIA_audioTracks.h"

/**
    \fn audioTrackCli
*/
class audioTrackCli: public DIA_audioTrackBase
{
public:
       
                    audioTrackCli( PoolOfAudioTracks *pool, ActiveAudioTracks *active ) : DIA_audioTrackBase( pool, active )
                    {

                    };
            virtual		~audioTrackCli(){};
            virtual   bool      run(void)
                        {
                            ADM_info("Running Cli audioTrack GUI\n");

							return true;
                        }
              
};

/**
        \fn createEncoding
*/
namespace ADM_CliCoreUIToolkit
{
DIA_audioTrackBase *createAudioTrack( PoolOfAudioTracks *pool, ActiveAudioTracks *active )
{
        return new audioTrackCli(pool,active);
}
}
//********************************************
//EOF
