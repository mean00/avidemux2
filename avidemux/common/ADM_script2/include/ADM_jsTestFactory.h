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
int jsTestCrash(void);
int jsTestAssert(void);
int jsTestFacNotch(void);
int jsTestFacThreadCount(void);
int jsTestFacMatrix(void);
int jsTestFacMatrix(void);
int jsTestFacHex(void);
int jsTestFacFrame(void);
int jsTestFacTab(void);
int jsTestFacText(void);
int jsTestFacRoText(void);
int jsTestFacSlider(void);
int jsTestFacButton(void);
int jsTestFacBar(void);
int jsTestFacBitrate(void);
int jsTestFacDirSel(void);
int jsTestFacFile(void);
int jsTestFacMenu(void);
int jsTestFacToggle(void);
int jsTestFacFloat(void);
int jsTestFacInt(void);
int jsTestFacEncoding(void);
};


#endif
