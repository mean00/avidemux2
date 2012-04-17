#ifndef ADM_GUI_UI_H
#define ADM_GUI_UI_H

//#include "ADM_editor/ADM_outputfmt.h"

void UI_updateFrameCount(uint32_t curFrame);
void UI_setFrameCount(uint32_t curFrame,uint32_t total);

//void UI_updateTimeCount(uint32_t curFrame, uint32_t fps);
//void UI_setTimeCount(uint32_t curFrame,uint32_t total, uint32_t fps);
void UI_setCurrentTime(uint64_t pts);
void UI_setTotalTime(uint64_t curTime);
double 	UI_readScale( void );
void 	UI_setScale( double  val );
void 	UI_setFrameType( uint32_t frametype,uint32_t qp);
void 	UI_setMarkers(uint64_t a, uint64_t b );
void 	UI_setTitle(const char *name);
int32_t UI_readJog(void);

void UI_setAProcessToggleStatus( uint8_t status );
void UI_setVProcessToggleStatus( uint8_t status );

void UI_iconify( void );
void UI_deiconify( void );

int UI_readCurFrame( void );
int UI_readCurTime(uint16_t &hh, uint16_t &mm, uint16_t &ss, uint16_t &ms);
void UI_JumpDone(void);

int    UI_getCurrentPreview(void);
void   UI_setCurrentPreview(int ne);

int 	UI_getCurrentACodec(void);
int 	UI_getCurrentVCodec(void);
void UI_setAudioCodec( int i);
void UI_setVideoCodec( int i);

/* We deal index wise here...*/
int      UI_GetCurrentFormat( void );
void 	 UI_SetCurrentFormat( uint32_t fmt );

void UI_toogleSide(void);
void UI_toogleMain(void);


void UI_updateRecentMenu( void );

uint8_t UI_arrow_enabled(void);
uint8_t UI_arrow_disabled(void);

void UI_refreshCustomMenu(void);

bool UI_setVUMeter( uint32_t volume[6]); // Volume between 0 and 255.
bool UI_setDecoderName(const char *name);
bool UI_hasOpenGl(void);
void UI_closeGui(void);
#endif
// EOF
