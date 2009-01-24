/***************************************************************************
                          ADM_ompages.h  -  description
                             -------------------

			Low level page handler

    begin                : Tue Apr 28 2003
    copyright            : (C) 2003 by mean
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
#ifndef __OPAGES__
#define __OPAGES__
class OGMDemuxer
{
	protected:
			FILE 		*_fd;
			OG_Header  		_page;
			uint32_t		_payload;
			uint64_t		_hdrpos;
			uint64_t		_filesize;
			uint8_t			_lace[255];
			uint32_t		_nbLace;
			uint32_t		_nbFrag;

	public:

						OGMDemuxer(void);
						~OGMDemuxer();
			uint8_t		dumpHeaders(uint8_t *ptr,uint32_t *size);
			uint8_t 		open(const char *file);			
			uint8_t		setPos( uint64_t pos );
			uint8_t		readHeader(uint32_t *paySize, uint32_t *flag,uint64_t *frame,
							uint8_t *id);
			uint8_t		readHeaderOfType(uint8_t type,uint32_t *paySize, uint32_t *flag,
							uint64_t *frame);
			uint8_t		readPayload(uint8_t *data);
			uint8_t		readBytes(uint32_t size, uint8_t *data);
			uint8_t		skipBytes(uint32_t off);
			uint64_t 		getPos( void );
			uint64_t 		getFileSize( void );
			uint32_t		getLace(uint8_t *lace,uint8_t **laces)
							{
											*lace=_nbLace;
											*laces=_lace;
											return 1;
							}
			uint32_t 		getFrag( void ) { return _nbFrag;}
};
#endif

