#pragma once

#define CLEAR_FRAME_TYPE 0xFFFFFFFF

void UI_updateFrameCount(uint32_t curFrame);
void UI_setFrameCount(uint32_t curFrame,uint32_t total);

void UI_setCurrentTime(uint64_t pts);
void UI_setTotalTime(uint64_t curTime);
void UI_setSegments(uint32_t numOfSegs, uint64_t * segPts);
double 	UI_readScale( void );
void 	UI_setScale( double  val );
void 	UI_setFrameType( uint32_t frametype,uint32_t qp);
void 	UI_setMarkers(uint64_t a, uint64_t b );
void 	UI_setTitle(const char *name);
int32_t UI_readJog(void);

void UI_setAProcessToggleStatus( uint8_t status );
void UI_setVProcessToggleStatus( uint8_t status );

void    UI_iconify( void );
void    UI_deiconify( void );

void    UI_JumpDone(void);

int     UI_getCurrentPreview(void);
void    UI_setCurrentPreview(int ne);

int 	UI_getCurrentACodec(void);
int 	UI_getCurrentVCodec(void);
void    UI_setAudioCodec( int i);
void    UI_setVideoCodec( int i);

/* We deal index wise here...*/
int      UI_GetCurrentFormat( void );
void 	 UI_SetCurrentFormat( uint32_t fmt );

void     UI_toogleSide(void);
void     UI_toogleMain(void);


void UI_updateRecentMenu( void );
void UI_updateRecentProjectMenu();

uint8_t UI_arrow_enabled(void);
uint8_t UI_arrow_disabled(void);

void UI_refreshCustomMenu(void);
void UI_applySettings(void);

bool UI_setVUMeter( int32_t volume[8]); // Volume as integer dBFS (valid range -100 to +3), mark inactive channel with +255 (> 3)
bool UI_setVolume(void);
bool UI_setDecoderName(const char *name);
bool UI_setDisplayName(const char *name);
bool UI_hasOpenGl(void);
void UI_closeGui(void);
bool UI_getTimeShift(int *onoff,int *value);
bool UI_setTimeShift(int onoff,int value);
void UI_setAudioTrackCount( int nb );

void UI_notifyInfo(const char *message, int timeoutMs);
void UI_notifyWarning(const char *message, int timeoutMs);
void UI_notifyError(const char *message, int timeoutMs);
void UI_notifyPlaybackLag(uint32_t lag, int updateTimeMs);

/* We need to know whether auto-repeat is firing */
bool UI_navigationButtonsPressed(void);

bool UI_askAvisynthPort(uint32_t &port);

bool UI_reset(void);

void UI_tweaks(const char * op, const char * paramS, int paramN);
// EOF
