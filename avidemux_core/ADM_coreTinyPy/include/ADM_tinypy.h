/***************************************************************************
    \file  ADM_tinypy.cpp
    \brief Wrapper around tinypy
    \author mean fixounet@free.fr (c) 2010

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_TINYPY_H
#define ADM_TINYPY_H
/**
    \class tinyPy
*/
class tinyPy 
{
protected:
        void *instance;
public:
                tinyPy(void);
        bool    init(void);
                ~tinyPy(void);
        bool    execString(const char *s);
        bool    dumpInternals(void);
};
#endif
