/*
 * 
 * Ported from yadif c plugin, then ported from ffmpeg vf_yadif
 * 
 * 
 * Copyright (C) 2006-2011 Michael Niedermayer <michaelni@gmx.at>
 *               2010      James Darnley <james.darnley@gmail.com>

 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * 
 * 
 	Yadif C-plugin for Avisynth 2.5 - Yet Another DeInterlacing Filter
	Copyright (C)2007 Alexander G. Balakhnin aka Fizick  http://avisynth.org.ru
    Port of YADIF filter from MPlayer
	Copyright (C) 2006 Michael Niedermayer <michaelni@gmx.at>

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

    Avisynth_C plugin
	Assembler optimized for GNU C compiler

*/

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "yadif.h"
#include "yadif_desc.cpp"
#include "libavutil/common.h"
#if defined( ADM_CPU_X86) 
        #define CAN_DO_INLINE_X86_ASM
#endif


//************************************************


static void filter_line_c( uint8_t *dst1,  const uint8_t  *prev1, const uint8_t  *cur1, const uint8_t  *next1, int w, int prefs, int mrefs, int parity, int mode);
static void filter_edges_c(uint8_t *dst1, const uint8_t  *prev1, const uint8_t  *cur1, const uint8_t  *next1, int w, int prefs, int mrefs, int parity, int mode);
static void filter_end_c(void) {};

//===========================================================================//
#ifdef CAN_DO_INLINE_X86_ASM
extern "C"
{
 void filter_end_sse(void);     // emms , not needed for sse2
 void adm_yadif_filter_line_sse2( uint8_t *dst1,  const uint8_t  *prev1, const uint8_t  *cur1, const uint8_t  *next1, int w, int prefs, int mrefs, int parity, int mode);
 void adm_yadif_filter_line_ssse3( uint8_t *dst1,  const uint8_t  *prev1, const uint8_t  *cur1, const uint8_t  *next1, int w, int prefs, int mrefs, int parity, int mode);
}
#endif


enum YADIFMode {
    YADIF_MODE_SEND_FRAME           = 0, ///< send 1 frame for each frame
    YADIF_MODE_SEND_FIELD           = 1, ///< send 1 frame for each field
    YADIF_MODE_SEND_FRAME_NOSPATIAL = 2, ///< send 1 frame for each frame but skips spatial interlacing check
    YADIF_MODE_SEND_FIELD_NOSPATIAL = 3, ///< send 1 frame for each field but skips spatial interlacing check
};

enum YADIFParity {
    YADIF_PARITY_TFF  =  0, ///< top field first
    YADIF_PARITY_BFF  =  1, ///< bottom field first
    YADIF_PARITY_AUTO = -1, ///< auto detection
};

enum YADIFDeint {
    YADIF_DEINT_ALL        = 0, ///< deinterlace all frames
    YADIF_DEINT_INTERLACED = 1, ///< only deinterlace frames marked as interlaced
};


//
/**
    \class yadifFilter
*/
class yadifFilter : public  ADM_coreVideoFilterCached
{
protected:
                    yadif       configuration;
                    void        updateInfo(void);
public:
                    yadifFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~yadifFilter() {};

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
		virtual void setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void) ;                        /// Start graphical user interface
        virtual bool         goToTime(uint64_t usSeek);
        
protected:
    void (*filter_line) (uint8_t *dst, const uint8_t  *prev, const uint8_t  *cur, const uint8_t  *next, int w, int prefs, int mrefs, int parity, int mode);
    void (*filter_edges)(uint8_t *dst, const uint8_t  *prev, const uint8_t  *cur, const uint8_t  *next, int w, int prefs, int mrefs, int parity, int mode);
    void (*filter_end)(void);    
    
    void filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx);
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   yadifFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "yadif",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("yadif","Yadif"),            // Display name
                        QT_TRANSLATE_NOOP("yadif","Yadif, port of avisynth version (c) Fizick.") // Description
                    );

//

/**
    \fn constructor
*/
yadifFilter::yadifFilter(ADM_coreVideoFilter *in, CONFcouple *setup): 
            ADM_coreVideoFilterCached(10,in,setup)
{
    if(!setup || !ADM_paramLoad(setup,yadif_param,&configuration))
    {
        // Default value
        configuration.mode=YADIF_MODE_SEND_FRAME;
        configuration.deint=YADIF_DEINT_ALL; 
        configuration.parity=YADIF_PARITY_TFF;
    }
    updateInfo();

    filter_line=filter_line_c;
    filter_edges=filter_edges_c;
    filter_end=filter_end_c;

#ifdef CAN_DO_INLINE_X86_ASM    
     if (CpuCaps::hasSSSE3())
     {
        filter_line=adm_yadif_filter_line_ssse3;
     }else 
        if (CpuCaps::hasSSE2()) 
        {
           filter_line=adm_yadif_filter_line_sse2;
        }
#endif
    
    myName="yadif";
}
/**
    \fn updateInfo
*/
void yadifFilter::updateInfo(void)
{
    memcpy(&info,previousFilter->getInfo(),sizeof(info));
    if(configuration.mode &1 ) // Bob
    {
        info.frameIncrement/=2;
        if(info.timeBaseNum && info.timeBaseDen)
        {
            if(info.timeBaseDen<=30000 && (info.timeBaseNum & 1))
                info.timeBaseDen*=2;
            else
                info.timeBaseNum/=2;
        }
    }
}
/**
    \fn configure
*/
bool yadifFilter::configure( void) 
{
    
    diaMenuEntry tMode[]={
                             {0,      QT_TRANSLATE_NOOP("yadif","Frame : Temporal & spatial check"),NULL},
                             {1,   QT_TRANSLATE_NOOP("yadif","Field :  Temporal & spatial check"),NULL},
                             {2,      QT_TRANSLATE_NOOP("yadif","Frame : Skip spatial temporal check"),NULL},
                             {3,  QT_TRANSLATE_NOOP("yadif","Field : Skip spatial temporal check"),NULL}
          };
    diaMenuEntry tOrder[]={
                            //{0,   QT_TRANSLATE_NOOP("yadif","Auto"),NULL},
                            {1,   QT_TRANSLATE_NOOP("yadif","Top field first"),NULL},
                            {2,      QT_TRANSLATE_NOOP("yadif","Bottom field first"),NULL}                             
          };

    diaMenuEntry tDeint[]={
                            {0,   QT_TRANSLATE_NOOP("yadif","Deint all"),NULL},
                            {1,   QT_TRANSLATE_NOOP("yadif","Deint interlaced"),NULL}
          };

     
    uint32_t parity=configuration.parity+1;
    diaElemMenu mMode(&(configuration.mode),   QT_TRANSLATE_NOOP("yadif","_Mode:"), 4,tMode);
    diaElemMenu mDeint(&(configuration.deint),   QT_TRANSLATE_NOOP("yadif","_Deint:"), 2,tDeint);    
    diaElemMenu mOrder(&(parity),   QT_TRANSLATE_NOOP("yadif","_Order:"), 2,tOrder);
    diaElem *elems[]={&mMode,&mOrder}; //,&mDeint};
     
    if(diaFactoryRun(QT_TRANSLATE_NOOP("yadif","yadif"),sizeof(elems)/sizeof(diaElem *),elems))
    {
        configuration.parity=(int)parity-1;
        updateInfo();
        return 1;
    }
    return 0;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         yadifFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, yadif_param,&configuration);
}

void yadifFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, yadif_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *yadifFilter::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"yadif : mode=%d, order=%d, deint=%d\n",
                (int)configuration.mode, (int)configuration.parity,(int)configuration.deint);
    return conf;
}

/**
    \fn goToTime
    \brief Seek in filter preview mode
*/
bool yadifFilter::goToTime(uint64_t usSeek)
{
    uint32_t oldFrameIncrement=info.frameIncrement;
    if(configuration.mode &1) // Bob
        info.frameIncrement*=2;
    bool r=ADM_coreVideoFilterCached::goToTime(usSeek);
    info.frameIncrement=oldFrameIncrement;
    return r;
}

#include "ADM_vidYadif_body.cpp"
