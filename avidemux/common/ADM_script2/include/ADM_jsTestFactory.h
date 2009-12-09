/**
    \file ADM_jsTestFactory.h
    \brief  Simple js hook to test factory
    \author mean (c) 2009 fixounet@free.fr


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADMJS_FACTORY_H
#define ADMJS_FACTORY_H
extern "C"
{
bool jsTestCrash(void);
bool jsTestAssert(void);
bool jsTestFacNotch(void);
bool jsTestFacThreadCount(void);
bool jsTestFacMatrix(void);
bool jsTestFacMatrix(void);
bool jsTestFacHex(void);
bool jsTestFacFrame(void);
bool jsTestFacTab(void);
bool jsTestFacText(void);
bool jsTestFacRoText(void);
bool jsTestFacSlider(void);
bool jsTestFacButton(void);
bool jsTestFacBar(void);
bool jsTestFacBitrate(void);
bool jsTestFacDirSel(void);
bool jsTestFacFile(void);
bool jsTestFacMenu(void);
bool jsTestFacToggle(void);
bool jsTestFacFloat(void);
bool jsTestFacInt(void);
};


#endif
