/**
    \file avi_vars.h
    \brief a couple of global variables

*/
#ifndef __AVI_VARS
#define __AVI_VARS

#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"
//----------------------

#ifdef __DECLARE__
#define EXTERN 
#else
#define EXTERN extern
#endif

#define DEBUG
#define DEBUG_L2


EXTERN ADM_Composer *video_body
#ifdef __DECLARE__
=NULL
#endif
;

EXTERN uint8_t 	playing
#ifdef __DECLARE__
=0
#endif
;

EXTERN ADM_audioStream *aviaudiostream
#ifdef __DECLARE__
=(ADM_audioStream *)NULL
#endif
;
EXTERN ADM_audioStream *currentaudiostream
#ifdef __DECLARE__
=(ADM_audioStream *)NULL
#endif
;

;
EXTERN aviInfo   *avifileinfo
#ifdef __DECLARE__
=(aviInfo *)NULL
#endif
;

#endif
