/***************************************************************************
                          ADM_Atom  -  description
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


#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_atom.h"
#define aprintf(...) {}

adm_atom::adm_atom(adm_atom *atom)
{
	_fd=atom->_fd;
	_atomStart=ftello(_fd);
	_atomSize=read32();
	_atomFCC=read32();
	// Gross hack for some (buggy ?) movie
	if(!_atomSize)
	{
		printf("3GP:Workaround: detected wrong sized atom!\nTrying to continue\n");
		_atomStart+=4;
		_atomSize-=4;
		fseeko(_fd,_atomStart,SEEK_SET);
		_atomSize=read32();
		_atomFCC=read32();
	}

	if (fourCC::check(_atomFCC, (uint8_t *)"tadm") && _atomSize == 1)	// mdat
		_atomSize=read64();

#ifdef ATOM_DEBUG
	dumpAtom();
#endif
}
/**
    \fn duplicate constructor
    \brief returns a copy of the current atom
*/
adm_atom::adm_atom(adm_atom *atom,int duplicate)
{
   memcpy(this,atom,sizeof(adm_atom));
}
adm_atom::adm_atom(FILE *fd )
{
int64_t orgpos;
	_fd=fd;
        orgpos=ftello(fd);
	fseeko(_fd,0,SEEK_END);
	_atomFCC=fourCC::get((uint8_t *)"MOVI");
	_atomSize=ftello(_fd);//-orgpos;

	fseeko(_fd,orgpos,SEEK_SET);
	_atomStart=0;
#ifdef ATOM_DEBUG
	dumpAtom();
#endif
#ifdef _3G_LOGO
        printf("Starting at %x  atom ",_atomStart);
        fourCC::printBE(_atomFCC);
        printf("\n");
#endif

}
uint8_t adm_atom::skipBytes( uint32_t nb )
{
int64_t pos;
	fseeko(_fd,nb,SEEK_CUR);
	pos=ftello(_fd);
	if(pos>_atomStart+_atomSize+1) ADM_assert(0);
	return 1;
}

uint8_t adm_atom::read( void )
{
	uint8_t a1;

		a1=fgetc(_fd);
	return a1;

}

uint16_t adm_atom::read16( void )
{
	uint8_t a1,a2;

		a1=fgetc(_fd);
		a2=fgetc(_fd);
	return (a1<<8)+(a2);

}


uint32_t adm_atom::read32( void )
{
	uint8_t a1,a2,a3,a4;

		a1=fgetc(_fd);
		a2=fgetc(_fd);
		a3=fgetc(_fd);
		a4=fgetc(_fd);
	return (a1<<24)+(a2<<16)+(a3<<8)+(a4);

}

uint64_t adm_atom::read64( void )
{
	uint64_t a1 = read32();

	return (a1 << 32) + read32();
}

uint32_t adm_atom::getFCC( void )
{
	return _atomFCC;
}

int64_t adm_atom::getRemainingSize( void )
{
        int64_t pos=ftello(_fd);

        return _atomStart+_atomSize-pos;
}

uint8_t adm_atom::readPayload( uint8_t *whereto, uint32_t rd)
{
	int64_t pos;

	pos=ftello(_fd);
	if(pos+rd>_atomSize+_atomStart)
	{
		printf("\n Going out of atom's bound!! (%"LLD"  / %"LLD" )\n",pos+rd,_atomSize+_atomStart);
		dumpAtom();
		exit(0);
	}
	uint32_t i;
	i=fread(whereto,rd,1,_fd);
	if(i!=1)
	{
		printf("\n oops asked %"LU" got %"LU" \n",rd,i);
	return 0;
	}
	return 1;

}
uint8_t adm_atom::dumpAtom( void )
{

	aprintf("Atom :");
	fourCC::print(_atomFCC);
	aprintf(" starting at pos %lu, size %lu\n",_atomStart,_atomSize);
	return 1;

}

uint8_t adm_atom::skipAtom( void )
{
	fseeko(_fd,_atomStart+_atomSize,SEEK_SET);
#ifdef _3G_LOGO
        printf("Branching to %x ending atom ",_atomStart+_atomSize);
        fourCC::printBE(_atomFCC);
        printf("\n");
#endif
	return 1;


}
uint8_t adm_atom::isDone( void )
{
	int64_t pos=ftello(_fd);

	if(pos>=(_atomStart+_atomSize)) return 1;
	return 0;

}




//EOF

