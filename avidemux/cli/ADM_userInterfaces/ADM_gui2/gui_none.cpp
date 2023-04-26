// C++ Interface: 
//
// Description: 
//
//
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_misc.h"

#include "GUI_ui.h"
#include "ADM_colorspace.h"
#include "ADM_muxerProto.h"

static int cliFormat=-1;
static int audioCodec=0;
static int videoCodec=0;
 

//**************************************************
int 	UI_getCurrentACodec(void)
{
  return audioCodec; 
}
void UI_setAudioCodec( int i)
{
  audioCodec=i;
}

//**************************************************
#if 0
uint8_t 	UI_SetCurrentFormat( ADM_OUT_FORMAT fmt )
{
format=fmt;
}

ADM_OUT_FORMAT 	UI_GetCurrentFormat( void )
{
  return format;
}
#endif
//**************************************************
int 	UI_getCurrentVCodec(void)
{
  return videoCodec; 
}
void UI_setVideoCodec( int i)
{
  videoCodec=i;
}
//**************************************************
void UI_setAProcessToggleStatus( uint8_t status )
{}
void UI_setVProcessToggleStatus( uint8_t status )
{}
//**************************************************
void UI_refreshCustomMenu(void) {}

void UI_applySettings(void)
{}

int    UI_getCurrentPreview(void)
{
  return 0; 
}
void   UI_setCurrentPreview(int ne)
{
}

//**************************************************
void UI_updateFrameCount(uint32_t curFrame)
{}
void UI_setFrameCount(uint32_t curFrame,uint32_t total)
{}

void UI_updateTimeCount(uint32_t curFrame, uint32_t fps)
{}

double 	UI_readScale( void )
{
  return 0;
}
void 	UI_setScale( double  val )
{}
void 	UI_setFrameType( uint32_t frametype,uint32_t qp)
{}
void 	UI_setTitle(const char *name)
{}






void UI_iconify( void )
{}
void UI_deiconify( void )
{}

int UI_readCurFrame( void )
{
    return 0;
}

int UI_readCurTime(uint16_t &hh, uint16_t &mm, uint16_t &ss, uint16_t &ms) { return 0; }

void UI_JumpDone(void)
{}


void UI_toogleSide(void)
{}
void UI_toogleMain(void)
{}

bool UI_getTimeShift(int *onoff,int *value)
{
  *onoff=0;
  *value=0;
  return 1;
}
bool UI_setTimeShift(int onoff,int value)
{
  return 1;
}

void UI_updateRecentMenu( void ) {}
void UI_updateRecentProjectMenu() {}

uint8_t UI_arrow_enabled(void)
{
  return 1;
}
uint8_t UI_arrow_disabled(void)
{
  return 1;
}

int UI_GetCurrentFormat( void )
{
    if(cliFormat == -1)
    {
        ADM_warning("Output format not specified or invalid. Trying MKV as fallback.\n");
        return ADM_MuxerIndexFromName("MKV");
    }
    return cliFormat;
}
void UI_SetCurrentFormat( uint32_t f )
{
	cliFormat=f;
}
void UI_setCurrentTime(uint64_t x)
{

}
void UI_setMarkers(uint64_t a,uint64_t b)
{
}
void UI_setTotalTime(uint64_t t)
{
}
void UI_setSegments(uint32_t numOfSegs, uint64_t * segPts)
{
}
bool UI_setVUMeter(int32_t volume[8])
{
    return true;
}
bool UI_setVolume(void)
{
    return true;
}
bool UI_askAvisynthPort(uint32_t &port)
{
	return false;
}
bool  	UI_reset(void)
{
    return true;
}
bool UI_navigationButtonsPressed(void)
{
    return false;
}
bool UI_setDisplayName(char const*)
{
        return true;
}
void UI_setNeedsResizingFlag(bool resize)
{}
void UI_setBlockZoomChangesFlag(bool block)
{}
void UI_resetZoomThreshold(void)
{}
void UI_setZoomToFitIntoWindow(void)
{}
void UI_displayZoomLevel(void)
{}
void UI_getMaximumPreviewSize(uint32_t *availWidth, uint32_t *availHeight)
{
    *availWidth = 0;
    *availHeight = 0;
}

void UI_notifyInfo(const char *message, int timeoutMs)
{}
void UI_notifyWarning(const char *message, int timeoutMs)
{}
void UI_notifyError(const char *message, int timeoutMs)
{}
// EOF
