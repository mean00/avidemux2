/***************************************************************************
                          ADM_vidMSharpen  -  description
                             -------------------
    
    email                : fixounet@free.fr

    Port of Donal Graft Msharpen which is (c) Donald Graft
    http://www.neuron2.net
    http://puschpull.org/avisynth/decomb_reference_manual.html

        It is a bit less efficient as we do hz & vz blur separately
        The formula has been changed a bit from 1 1 1 to 1 2 1
        for speed aspect & MMX  
        Mean

 ***************************************************************************/
/*
	Msharpen plugin for Avisynth -- performs detail-preserving smoothing.

	Copyright (C) 2003 Donald A. Graft

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
#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_coreVideoFilter.h"
#include "msharpen.h"

/**
    \class Msharpen
*/
class Msharpen : public ADM_coreVideoFilterCached
{
private:
        msharpen	_param;
        ADMImage        *blurrImg,*work;

        uint32_t        invstrength;

public:    

                            Msharpen(ADM_coreVideoFilter *in,CONFcouple *couples)   ;
                            ~Msharpen();

       virtual const char  *getConfiguration(void);          /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
       virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual void         setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                 /// Start graphical user interface        
public:
       static  void    detect_edges(ADMImage *src, ADMImage *dst, int plane,const msharpen &param);
       static  void    blur_plane(ADMImage *src, ADMImage *blur, int plane,ADMImage *work) ;
       static  void    detect_edges_HiQ(ADMImage *src, ADMImage *dst, int plane,const msharpen &param);
       static  void    apply_filter(ADMImage *src,ADMImage *blur, ADMImage *dst,int plane,const msharpen &param,uint32_t invstrength) ;

       static  void    reset(msharpen *cfg);
};
