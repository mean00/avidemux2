/* macroblock.hh macroblock class... */

/*  (C) 2003 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#include <stdio.h>

#include "macroblock.hh"
#include "mpeg2enc.h"
#include "picture.hh"

void MacroBlock::MotionEstimate()
{
	if (picture->pict_struct==FRAME_PICTURE)
	{			
		FrameMEs();
	}
	else
	{		
		FieldME();
	}
    vector<MotionEst>::iterator i;
    vector<MotionEst>::iterator min_me = plausible_me.begin();
    for( i = plausible_me.begin(); i < plausible_me.end(); ++ i)
        if( i->var < min_me->var )
            min_me = i;
    final_me = *min_me;
}

 


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */

