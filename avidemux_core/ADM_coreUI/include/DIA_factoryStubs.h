/***************************************************************************
                          DIA_factoryStubs.h
  
  (C) Mean 2008 fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIA_FACTORY_STUBS_H
#define DIA_FACTORY_STUBS_H

#define DIA_MKSTUBS(aClass) \
void      aClass::setMe(void *dialog, void *opaque,uint32_t line)\
	{ \
		ADM_assert(internalPointer); \
		internalPointer->setMe(dialog,opaque,line); \
	} \
void      aClass::getMe(void)\
	{ \
	ADM_assert(internalPointer); \
	internalPointer->getMe(); \
	} \
int      aClass::getRequiredLayout(void)\
	{ \
	ADM_assert(internalPointer); \
	return internalPointer->getRequiredLayout(); \
	} 
#endif
