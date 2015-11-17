/***************************************************************************
    copyright            : (C) 2015 by mean
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
#pragma once
/**
 * 
 * @param curFrame
 */
class admUITaskBarProgress
{
public:    
    virtual bool enable()=0;
    virtual bool disable()=0;
    virtual bool setProgress(int percent)=0; 
};
admUITaskBarProgress *UI_getTaskBarProgress(); 