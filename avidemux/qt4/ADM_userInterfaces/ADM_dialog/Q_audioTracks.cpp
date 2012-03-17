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
#include "Q_audioTracks.h"

#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_toolkitQt.h"
#include "ADM_edit.hxx"
#include "DIA_audioTracks.h"
extern void UI_purge(void);
/**
    \fn audioTrackQt4
*/



class audioTrackQt4: public DIA_audioTrackBase
{
protected:
            audioTrackWindow *window;
          
public:
       
                    audioTrackQt4( PoolOfAudioTracks *pool, ActiveAudioTracks *active ) : DIA_audioTrackBase( pool, active )
                    {
                            window=new audioTrackWindow();
                            qtRegisterDialog(window);
                            window->setModal(TRUE);
                            // Fill in the menu

                            window->show();
                    };
            virtual		~audioTrackQt4(){};
            virtual   bool      run(void)
                        {
                            ADM_info("Running QT4 audioTrack GUI\n");
                        }
              
};

/**
        \fn createEncoding
*/
namespace ADM_Qt4CoreUIToolkit
{
DIA_audioTrackBase *createAudioTrack( PoolOfAudioTracks *pool, ActiveAudioTracks *active )
{
        return new audioTrackQt4(pool,active);
}
}
//********************************************
//EOF
