//____________________________________________________________________
//
//
// Author: mean <fixounet@free.fr>, (C) 2003-2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
//____________________________________________________________________
#ifndef ADM_CTNER_
#define ADM_CTNER_
typedef struct ADM_CONTAINER
{
    ADM_OUT_FORMAT type;
    const char     *name;
}ADM_CONTAINER;
#define MK_CONT(x) {ADM_##x,#x}

const ADM_CONTAINER container[]=
{
  MK_CONT(AVI),
  MK_CONT(OGM),
  MK_CONT(ES),
  MK_CONT(PS),
  MK_CONT(TS),
  MK_CONT(AVI_DUAL),
  MK_CONT(AVI_UNP),
  MK_CONT(MP4),
  MK_CONT(PSP),
  MK_CONT(FLV),
  MK_CONT(DUMMY),
  MK_CONT(MATROSKA)  
};
#define NB_CONT (sizeof(container)/sizeof(ADM_CONTAINER))
#endif
