/***************************************************************************
                          ADM_mp4Tree.h  -  description
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
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
#ifndef ADM_MP4_TREE_H
#define ADM_MP4_TREE_H

typedef enum 
{
#include "ADM_mp4Leaf.h"
}ADMAtoms;

uint8_t ADM_mp4SearchAtomName(uint32_t atom, ADMAtoms *atomId,uint32_t *isContainer);
uint8_t ADM_mp4SimpleSearchAtom(adm_atom *rootAtom, ADMAtoms atomToFind,adm_atom **atomFound);
#endif
