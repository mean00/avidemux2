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
void    A_saveWorkbench (const char *name);
void    A_videoCheck( void);
void	A_setPostproc( void );
void    A_Resync(void);
void    A_addJob(void);
void    A_audioTrack(void);
int     A_Save(const char *name);
int     A_SaveWrapper( char *name);
void    A_parseECMAScript(const char *name);
uint8_t A_autoDrive(Action action);
uint8_t A_TimeShift(void);
void    A_ResetMarkers(void);
void    A_Rewind(void);
void    A_jog(void);
uint8_t A_jumpToTime(uint32_t hh,uint32_t mm,uint32_t ss,uint32_t ms);

#endif