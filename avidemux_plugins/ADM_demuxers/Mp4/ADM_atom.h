/***************************************************************************
                          ADM_Aton  -  description
                             -------------------

	Helper class to deal with atom

    begin                : Mon Jul 21 2003
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
#ifndef _ADM_ATOM
#define _ADM_ATOM
class adm_atom
{
private:
		FILE 		*_fd;
		int64_t		_atomStart,_atomSize;
		uint32_t	_atomFCC;
		uint8_t		dumpAtom( void );
public:
				adm_atom(FILE *fd);
				adm_atom(adm_atom *tm,int duplicate);
				adm_atom(adm_atom *atom);
		uint8_t	        skipAtom( void );
                uint32_t        getStartPos(void) {return _atomStart;}
		uint32_t	getFCC( void );
		int64_t         getRemainingSize( void );
		uint8_t		readPayload( uint8_t *whereto, uint32_t rd );
		uint8_t		isDone(void );
		uint8_t		skipBytes(uint32_t nb );

		uint64_t	read64( void );
		uint32_t	read32( void );
		uint16_t	read16( void );
		uint8_t	        read( void );


};

#endif
