/***************************************************************************
                          ADM_mp4Tree.h  -  description
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 * 
 * 
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
#include "fourcc.h"
#include "ADM_atom.h"
#include "ADM_mp4Tree.h"


#define aprintf(...) {}
#define adm_printf(...) {}


typedef struct ADMAtomDesc
{
    uint32_t  atom;
    ADMAtoms  atomId;
    uint32_t  isContainerAtom;
}ADMAtomDesc;

#define ADMMP4_TAB_LEAF

ADMAtomDesc allAtoms[]=
{
#include "ADM_mp4Leaf.h"
};
/**
      \fn     ADM_mp4SearchAtomName
      \brief  Search the atom to know if it is a container one. Additionnaly give its enum id
*/
uint8_t ADM_mp4SearchAtomName(uint32_t atom, ADMAtoms *atomId,uint32_t *isContainer)
{
  int nb=sizeof(allAtoms)/sizeof(ADMAtomDesc);
  
#if 1//def ADM_BIG_ENDIAN
  uint32_t swAtom= atom;
#else
  uint8_t *ptr=(uint8_t *)&atom;
  uint32_t swAtom= (ptr[0]<<24)+(ptr[1]<<16)+      (ptr[2]<<8)+(ptr[3]);
#endif
  
  ADMAtomDesc *tom=allAtoms;
  for(int i=0;i<nb;i++)
  {
     //printf("Searching ! %s %s %d/%d n",fourCC::tostringBE(atom),fourCC::tostringBE(swAtom),i,nb);
     if(tom->atom==swAtom)
     {
        *atomId=tom->atomId;
        *isContainer=tom->isContainerAtom;
        return 1;
      }
    tom++;
  }
#if 0
  printf("[MP4] Atom ");fourCC::print(swAtom);printf(":");fourCC::print(atom);
  printf("not found in table\n");
#endif
  return 0;
}
/**
      \fn     ADM_mp4SimpleSearchAtom
      \brief  Search inside an atom to find another one
*/
uint8_t ADM_mp4SimpleSearchAtom(adm_atom *rootAtom, ADMAtoms atomToFind,adm_atom **atomFound)
{
  ADMAtoms curAtom;
  uint32_t isContainer;
  *atomFound=NULL;
  
  while(!rootAtom->isDone())
  {
      adm_atom son(rootAtom);
      printf("Found atom %s size=%d \n",fourCC::tostringBE(son.getFCC()),(int)son.getRemainingSize());
      // lookup our atom
       if(ADM_mp4SearchAtomName(son.getFCC(), &curAtom,&isContainer))
       {
         if(curAtom==atomToFind) // Got it, duplicate
         {
             *atomFound=son.duplicate(); // Duplicate atom
             //adm_printf(ADM_PRINT_DEBUG,"Searching atom %s :found\n",fourCC::tostringBE(son.getFCC()));
             return 1;
         }
         
       }
      
      son.skipAtom();
  }
  adm_printf(ADM_PRINT_DEBUG,"Searching atom %s in atom %s:not found\n",
              fourCC::tostringBE(atomToFind),fourCC::tostringBE(rootAtom->getFCC()));
  
  return 0;
  
}


