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
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_toolkitGtk.h"
#include "ADM_edit.hxx"
#include "DIA_audioTracks.h"
extern void UI_purge(void);
/**
    \fn audioTrackGtk
*/
class audioTrackGtk: public DIA_audioTrackBase
{
public:
       
                    audioTrackGtk( PoolOfAudioTracks *pool, ActiveAudioTracks *active ) : DIA_audioTrackBase( pool, active )
                    {

                    };
            virtual		~audioTrackGtk(){};
            virtual   bool      run(void)
                        {
                            ADM_info("Running Gtk audioTrack GUI\n");
                        }
              
};

/**
        \fn createEncoding
*/
namespace ADM_GtkCoreUIToolkit
{
DIA_audioTrackBase *createAudioTrack( PoolOfAudioTracks *pool, ActiveAudioTracks *active )
{
        return new audioTrackGtk(pool,active);
}
}
//********************************************
//EOF
