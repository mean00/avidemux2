
/***************************************************************************
    Port of ColorYuv from avisynth to avidemux by mean
 ***************************************************************************/
 // Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidColorYuv.h"
#include "DIA_factory.h"
#include "colorYuv_desc.cpp"


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   vidColorYuv,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_COLORS,            // Category
                        "colorYuv",            // internal name (must be uniq!)
                        "Avisynth color filter.",            // Display name
                        "Color management filter." // Description
                    );

/**
    \fn configure
*/
bool         vidColorYuv::configure(void)
{
    MakeGammaLUT();
    return true;
}
/**
    \fn getConfiguration
*/
const char   *vidColorYuv::getConfiguration(void)
{
      const char *foobar="colorYuv";
      return foobar;
}
/**
    \fn ctor
*/
vidColorYuv::vidColorYuv( ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{	
	 if(!setup || !ADM_paramLoad(setup,colorYuv_param,&param))
    {
        // Default value
        #define MKP(x,y) param.x=y;
            MKP(y_contrast,0);
            MKP(y_bright,0);
            MKP(y_gamma,0);
            MKP(y_gain,0);

            MKP(u_contrast,0);
            MKP(u_bright,0);
            MKP(u_gamma,0);
            MKP(u_gain,0);

            MKP(v_contrast,0);
            MKP(v_bright,0);
            MKP(v_gamma,0);
            MKP(v_gain,0);
            
            MKP(matrix,0);
            MKP(levels,0);
            MKP(opt,0); 
       
            MKP(colorbars,0);
            MKP(analyze,1);
            MKP(autowhite,1); 
            MKP(autogain,0); 
    }  	 
    MakeGammaLUT(); 	
}
/**
    \fn getCoupledConf
*/
bool         vidColorYuv::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, colorYuv_param,&param);
}


/**
    \fn dtor
*/
vidColorYuv::~vidColorYuv()
{
}


// EOF
