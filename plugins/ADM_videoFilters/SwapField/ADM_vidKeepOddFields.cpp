/***************************************************************************
                          Swap Fields.cpp  -  description
                             -------------------
Swap each line  (shift up for odd, down for even)


    begin                : Thu Mar 21 2002
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

#include "ADM_default.h"


#include "ADM_videoFilterDynamic.h"
#include "ADM_vidFieldUtil.h"
#include "ADM_vidSwapFields.h"


static FILTER_PARAM swapParam={0,{""}};

   

//********** Register chunk ************

VF_DEFINE_FILTER(AVDMVideoKeepOdd,swapParam,
    keepoddfield,
                QT_TR_NOOP("Keep odd fields"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Keep bottom fields. Gives a half height picture.."));


#include "SwapCommon.cpp"


