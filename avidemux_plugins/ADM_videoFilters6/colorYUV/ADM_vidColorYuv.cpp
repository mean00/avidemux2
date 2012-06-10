
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
    bool r=false;

// --- Tab 1 ----
#define PX(x) (&(param.x))
     diaElemToggle     tAutoWhite(PX(autowhite),QT_TR_NOOP("AutoWhite"));
     diaElemToggle     tAutoGain(PX(autogain),QT_TR_NOOP("AutoGain"));
     diaElemToggle     tOpt(PX(opt),QT_TR_NOOP("Clip to Tv Range (16-235)"));
     diaMenuEntry      levelMenus[]={{0,QT_TR_NOOP("None"),NULL},
                                     {1,QT_TR_NOOP("PC->TV"),NULL},
                                     {2,QT_TR_NOOP("TV->PC"),NULL}};
// levels 	case 1:	// PC->TV Scale			case 2:	// TV->PC Scale  case 0:	//none
        diaElemMenu mLevel(PX(levels),QT_TR_NOOP("Levels:"), sizeof(levelMenus)/sizeof(diaMenuEntry),levelMenus,"");    
    diaElem *dia1[]={&tAutoWhite,&tAutoGain,&tOpt,&mLevel};
//  diaElemFloat(ELEM_TYPE_FLOAT *intValue,const char *toggleTitle, ELEM_TYPE_FLOAT min, 
//               ELEM_TYPE_FLOAT max,const char *tip=NULL, int decimals = 2);

// --- Tab 2 ----

    diaElemFloat        yGain(PX(y_gain),QT_TR_NOOP("Y gain"),0,256*3,NULL,3);
    diaElemFloat        yBright(PX(y_bright),QT_TR_NOOP("Y Brightness"),0,256*3,NULL,3);
    diaElemFloat        yGamma(PX(y_gamma),QT_TR_NOOP("Y Gamma"),0,256*2,NULL,3);
    diaElemFloat        yContrast(PX(y_contrast),QT_TR_NOOP("Y Contrast"),-256*3,256*3,NULL,3);
 
    diaElem *dia2[]={&yGain,&yBright,&yGamma,&yContrast};
// --- Tab 3 ----

    diaElemFloat        uGain(PX(u_gain),QT_TR_NOOP("U gain"),0,256*3,NULL,3);
    diaElemFloat        uBright(PX(u_bright),QT_TR_NOOP("U Brightness"),0,256*3,NULL,3);
    //diaElemFloat        uGamma(PX(u_gamma),QT_TR_NOOP("U Gamma"),0,100,NULL,3);
    diaElemFloat        uContrast(PX(u_contrast),QT_TR_NOOP("U Contrast"),-256*3,256*3,NULL,3);
 
    diaElem *dia3[]={&uGain,&uBright,&uContrast};

// --- Tab 4 ----

    diaElemFloat        vGain(PX(u_gain),QT_TR_NOOP("V gain"),0,256*3,NULL,3);
    diaElemFloat        vBright(PX(u_bright),QT_TR_NOOP("V Brightness"),0,256*3,NULL,3);
    //diaElemFloat        vGamma(PX(u_gamma),QT_TR_NOOP("V Gamma"),0,100,NULL,3);
    diaElemFloat        vContrast(PX(u_contrast),QT_TR_NOOP("V Contrast"),-256*3,256*3,NULL,3);
 
    diaElem *dia4[]={&vGain,&vBright,&vContrast};
//
    diaElemTabs tab1(QT_TR_NOOP("Flags"),4,dia1);
    diaElemTabs tab2(QT_TR_NOOP("Y"),4,dia2);
    diaElemTabs tab3(QT_TR_NOOP("U"),3,dia3);
    diaElemTabs tab4(QT_TR_NOOP("V"),3,dia4);
    
    diaElemTabs *tabs[]={&tab1,&tab2,&tab3,&tab4};
    if( diaFactoryRunTabs(QT_TR_NOOP("colorYuv"),4,tabs))
    {
        r=true;
    }

    MakeGammaLUT();
    return r;
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

void vidColorYuv::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, colorYuv_param, &param);
}

/**
    \fn dtor
*/
vidColorYuv::~vidColorYuv()
{
}


// EOF
