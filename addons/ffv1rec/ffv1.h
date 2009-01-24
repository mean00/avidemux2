/*---------------------------------------------
	ffv1video external iface

	GPL

	Mean Jul 2003

----------------------------------------------------*/
#include "default.h"

typedef void *(* adm_fast_memcpy)(void *to, const void *from, size_t len);
extern adm_fast_memcpy myAdmMemcpy;
#define memcpy myAdmMemcpy
/*          Video input iface        */
typedef struct
{
	int		width;
	int		height;
	int 		channel;
	int 		ntsc;
	int 		secam;
	double 		frequency;
	// These are codecs settings
	int		keydist;
	int		me;
	int		quality;
	int		quant; // 00 H263/ 1 MPEG
}v4linfo;

int 		initVideoDev(char *videodevice, v4linfo *info );
void closeVideoDev(void);

void 	captureVideoDev( void );

unsigned long int getVideoFourCC( void );
unsigned long int getVideoData( char **data );

/*          audio input iface        */
typedef struct
{
	long int 	bufferSize;
	long int	frequency;
}audioInfo;

int 		initAudioDev(char *videodevice,audioInfo *info );
void 	captureAudioDev( void *p );
int		audioDevPreinit(char *audiodevice,audioInfo *info );

extern void sighandler(int i);

//#define DP(DSTRING) fprintf(stderr, "%s\n", DSTRING);
#define DP(DSTRING)

/*		video compressor iface   */
int 				FFV1_Compress(unsigned char *in,unsigned char *out,uint32_t  *outlen);
int 				FFV1_Init(v4linfo *info);
int				 FFV1_selectByName( char *name);
void				FFV1_videoForceKeyFrame( void );
/*---------------wriiter thread-----------------*/
void write_process(void  *fname);
void writeInit(v4linfo *info);

/* EOF */
