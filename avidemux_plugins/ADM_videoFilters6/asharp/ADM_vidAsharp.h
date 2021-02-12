/**/
/***************************************************************************
                          ADM_vidASharp  -  description
                             -------------------
    Port of avisynth one

 ***************************************************************************/
/*

        Copyright (C) 2003 Donald A. Graft
        Copyright (C) 2002 Marc Fauconneau
        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#pragma once
#include "asharp.h"
#define uc uint8_t
/**
    \class ASharp
*/
class ASharp : public ADM_coreVideoFilter
{
private:
        asharp      _param;
        int32_t     T,D,B,B2;
        uint8_t     *lineptr;

public:
                    ASharp(ADM_coreVideoFilter *in,CONFcouple *couples);
                    ~ASharp();

        void        update(void);
virtual const char  *getConfiguration(void); /// Return  current configuration as a human readable string
virtual bool        getNextFrame(uint32_t *fn,ADMImage *image); /// Return the next image
virtual bool        getCoupledConf(CONFcouple **couples); /// Return the current filter configuration
virtual void        setCoupledConf(CONFcouple *couples);
virtual bool        configure(void); /// Start graphical user interface

        static void reset(asharp *cfg);
        static void asharp_run_c(uc* planeptr, int pitch, int height, int width,
                                int T,int D, int B, int B2, bool bf, uint8_t *lineptr);
};

