 /**
    \file   A_function.h
    \brief  Declaration A_function
*/
#ifndef A_FUNCTION_H
#define A_FUNCTION_H

#include "IScriptEngine.h"

int     A_delete(uint32_t start, uint32_t end);
void    A_externalAudioTrack(const char *trackIndex, const char *filename );
uint8_t A_rebuildKeyFrame (void);
void    A_openBrokenAvi (const char *name);
int     A_openVideo2 (const char *name, uint8_t mode);
int     A_appendVideo (const char *name);
void    A_videoCheck( void);
void	A_setPostproc( void );
void	A_setHDRConfig( void );
void    A_Resync(void);
void    A_addJob(void);
void    A_audioTrack(void);
void    A_setAudioLang(const char *trackIndex, const char *langueName);

bool    A_parseScript(IScriptEngine *engine, const char *name);
bool    A_runPythonScript(const std::string &name);
void    A_saveScript(IScriptEngine* engine, const char* name);
void    A_saveDefaultSettings(); // Save default settings
bool    A_loadDefaultSettings(void);
bool    A_saveSession(void);
bool    A_checkSavedSession(bool load);
//uint8_t A_autoDrive(Action action);
bool    A_TimeShift(void);
void    A_ResetMarkers(void);
void    A_Rewind(void);
void    A_jog(void);
bool    A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);
int 	A_openVideo		(const char *name);
int     A_saveAudio	(const char *name);
void 	A_saveAudioDecoded	(const char *name);
void 	A_saveAVI		(const char *name);
void 	A_playAvi		(void);
void    A_queueJob      (void);
int  A_saveAudioCopy (const char *name);
bool A_saveJpg (const char *name);
int  A_saveBunchJpg(const char *name);
int  A_saveBunchPng(const char *name);
bool A_saveImg (const char *name);
bool A_savePng(const char *name);
int  ADM_saveRaw (const char *name);
int  A_audioSave(const char *name);
int  A_SaveWrapper(const char *name);
int  A_saveAudioProcessed (const char *name);
int  A_Save(const char *name);

void A_queueJob(const char *jobName,const char *outputFile);

void A_set_avisynth_port(char *port_number_as_text);
bool A_getCommandLinePort(uint32_t &port);

#endif
