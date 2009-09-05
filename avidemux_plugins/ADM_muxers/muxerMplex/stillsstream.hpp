
/*
 *  stillsstreams.c: Class for elemenary still video streams
 *                   Most functionality is inherited from VideoStream
 *
 *  Copyright (C) 2001 Andrew Stevens <andrew.stevens@philips.com>
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "videostrm.hpp"

//
// Class for video stills sequence for (S)VCD non-mixed stills segment 
// item
//

class StillsStream : public VideoStream
{
public:
	StillsStream( IBitStream &ibs, 
                  StillsParams *parms,
                  Multiplexor &into) :
		VideoStream( ibs, parms, into ),
		current_PTS(0),
		current_DTS(0)
		{}
	void Init( );
private:
	virtual void NextDTSPTS( );
	clockticks current_PTS;
	clockticks current_DTS;
};

//
// Class for video stills sequence for VCD mixed stills Segment item.
// 

class VCDStillsStream : public StillsStream
{
public:
	VCDStillsStream( IBitStream &ibs,
                     StillsParams *vparms,
                     Multiplexor &into ) :
		StillsStream( ibs, vparms, into ),
		sibling( 0 ),
        stream_mismatch_warned( false )
		{}
	
	void SetSibling( VCDStillsStream * );
	virtual bool MuxPossible(clockticks currentSCR);
private:
	bool LastSectorLastAU();
	VCDStillsStream *sibling;
    bool stream_mismatch_warned;
	
};
	



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
