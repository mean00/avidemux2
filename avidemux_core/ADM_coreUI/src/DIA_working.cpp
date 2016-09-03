/***************************************************************************
   \file DIA_working.cpp
   \brief trampoline to real DIA_working class
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

#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "DIA_working.h"

#define SON ((DIA_workingBase *)son)
#include "DIA_coreUI_internal.h"
#if 0
extern DIA_workingBase *createWorking(const char *title);
/**
    \fn DIA_working
    \brief Constructor
*/
DIA_working::DIA_working( const char *title )
{
    son=NULL;
    son=createWorking(title);
}
/**
    \fn DIA_working
    \brief Destructor
*/
DIA_working::~DIA_working(  )
{
    if(son)
        delete SON;
    son=NULL;
}
/**
    \fn update
*/
uint8_t  	DIA_working::update(uint32_t percent)
{
    if(son) return SON->update(percent);
    return 0;
}
/**
    \fn update
*/
uint8_t 	DIA_working::update(uint32_t current,uint32_t total)
{
    if(son) return SON->update(current,total);
    return 0;
}
/**
    \fn isAlive
*/
uint8_t  	DIA_working::isAlive (void )
{
    if(son) return SON->isAlive();
    return 0;
}
#endif