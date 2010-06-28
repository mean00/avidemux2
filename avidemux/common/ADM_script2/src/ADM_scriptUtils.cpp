/**
    \file ADM_scriptUtils.cpp
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"
#include "ADM_scriptIf.h"
#include "ADM_editor/ADM_edit.hxx"
#include "GUI_ui.h"
#include "ADM_scriptCommon.h"
#include "ADM_confCouple.h"
#include "ADM_scriptUtils.h"
#include "ADM_muxerProto.h"
bool A_setContainer(const char *cont);
/**
    \fn scriptSetContainer
*/
int scriptSetContainer(const char *container, CONFcouple *conf)
{
    bool r=true;
    if(A_setContainer(container))
    {
        int idx=ADM_MuxerIndexFromName(container);
        if(idx!=-1)
        {
            r = ADM_mx_setExtraConf(idx,conf);
        }else
           r=false;
    } else r=false;
    if(conf) delete conf;
    return r;
}

/**
    \fn scriptTestCrash
    \brief Force a crash
*/
int scriptTestCrash(void)
{
  
  int *foobar=NULL;
  *foobar=0; // CRASH!
  return true;
}
/**
    \fn scriptTestAssert
    \brief Force a crash
*/
int scriptTestAssert(void)
{
  
  ADM_assert(0);
  return true;
}
//EOF
