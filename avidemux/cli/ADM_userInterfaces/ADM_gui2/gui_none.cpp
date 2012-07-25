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

static uint32_t cliFormat=0;
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

uint8_t UI_getTimeShift(int *onoff,int *value)
{
  *onoff=0;
  *value=0;
  return 1;
}
uint8_t UI_setTimeShift(int onoff,int value)
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
bool UI_setVUMeter(uint32_t volume[6])
{
    return true;
}
// EOF
