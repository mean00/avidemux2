 /**
    \file   A_function.h
    \brief  Declaration A_function
*/
#ifndef A_FUNCTION_H
#define A_FUNCTION_H

int     A_delete(uint32_t start, uint32_t end);
void    A_externalAudioTrack( void );
uint8_t A_rebuildKeyFrame (void);
void    A_openBrokenAvi (const char *name);
int     A_openAvi2 (const char *name, uint8_t mode);
int     A_appendAvi (const char *name);
void    A_videoCheck( void);
void	A_setPostproc( void );
void    A_Resync(void);
void    A_addJob(void);
void    A_audioTrack(void);

bool    A_parseECMAScript(const char *name);
bool    A_parseTinyPyScript(const char *name);

//uint8_t A_autoDrive(Action action);
uint8_t A_TimeShift(void);
void    A_ResetMarkers(void);
void    A_Rewind(void);
void    A_jog(void);
uint8_t A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);
int 			A_openAvi		(const char *name);
void 			A_saveAudio	(char *name);
void 			A_saveAudioDecoded	(char *name);
void 			A_saveAVI		(char *name);
void 			A_playAvi		(void);
void            A_queueJob      (void);
int  A_saveAudioCopy (const char *name);
int  A_saveJpg (const char *name);
int  A_saveBunchJpg(const char *name);
int  A_saveImg (const char *name);
int  ADM_saveRaw (const char *name);
void A_saveJsProject (const char *name);
void A_savePyProject (const char *name);
int  A_audioSave(const char *name);
int  A_SaveWrapper(const char *name);
int  A_saveAudioProcessed (const char *name);
int  A_Save(const char *name);

#endif