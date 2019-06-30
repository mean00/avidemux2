/***************************************************************************
    copyright            : (C) 2017 by mean
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
 */
class mkvDeviation
{
public:
        mkvDeviation(int n);
        ~mkvDeviation();
        int computeDeviation(int num, int den,int &nbSkipped);
        void sort(void);
        uint64_t *getSorted(void) { return sorted; }
        void add(uint64_t p)
        {
            sorted[nbValid++]=p+500; // one ms error, take in the middle
            ADM_assert(nbValid<=total);
        }

protected:
        int  total;
        int  nbValid;
        uint64_t *sorted;
  
};