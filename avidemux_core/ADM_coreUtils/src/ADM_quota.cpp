/**
                \fn ADM_quota.cpp
                \brief handle out of space error when writing to files
*/
#define ADM_LEGACY_PROGGY
#include <errno.h>
#include <string>
#include <stdarg.h>

#ifdef _WIN32
#	include <io.h>
#else
#	include <unistd.h>
#endif

using std::string;

#include "ADM_default.h"
#include "ADM_quota.h"
#include "DIA_coreToolkit.h"
#undef free


struct qfile_t {
        const char *filename;
        unsigned int ignore;
};
#define qfile_len 32768 //2^15=13....
#define qfprintf_buf_len 8192
#define msg_len 512
static qfile_t qfile[qfile_len];

#include "DIA_coreToolkit.h"


uint8_t  quotaInit(void)
{
            memset(qfile,0,sizeof(qfile));  
            return 1;
}

/* why here?: don't use mean's malloc rewrites for all of the xml2 library */
#include "ADM_assert.h"

FILE *qfopen(const string &fileName, const char *mode)
{
    return qfopen(fileName.c_str(),mode);
}
/* store open filenames and it's current "ignore"-status */
FILE *qfopen(const char *path, const char *mode){
    // Mean:Should be the first funtion to be called
    // The qfile array may or may not be initialized with 0
    // We will trigger an assert in the malloc if we send dummy
    
  FILE * FD = NULL;
  int fd;
	while( !FD ){
		FD = ADM_fopen (path,mode);
		if( !FD && (errno == ENOSPC 
#ifndef __MINGW32__
|| errno == EDQUOT
#endif
) ){
		  char msg[msg_len];
		  	fprintf(stderr,"qfopen(): can't open \"%s\": %s\n", path,
				       (errno==ENOSPC?"filesystem full":"quota exceeded"));
		  	ADM_assert(snprintf(msg,msg_len,"can't open \"%s\": %s\n%s\n",
						        path,
							(errno==ENOSPC?"filesystem full":"quota exceeded"),
							"Please free up some space and press RETRY to try again.")!=-1);
			GUI_Error_HIG("Error","msg");
			/* same behaviour for IGNORE and RETRY */
			continue;
		}
		if( !FD ){
		  char msg[msg_len];
			ADM_assert(snprintf(msg,msg_len,"can't open \"%s\": %u (%s)\n", path, errno, strerror(errno))!=-1);
			fprintf(stderr,"qfopen(): %s",msg);
			GUI_Error_HIG(msg,NULL);
			return NULL;
		}
	}
	/* keep filename for messages and ignore status */
	if( (fd=fileno(FD)) == -1 ){
		fprintf(stderr,"\nqfprintf(): bad stream argument\n");
		ADM_assert(0);
	}
	if( qfile[fd].filename )
		ADM_dealloc(qfile[fd].filename);
	qfile[fd].filename = ADM_strdup(path);
	qfile[fd].ignore = 0;
	return FD;
}

void qfprintf(FILE *stream, const char *format, ...){
  static char buf [qfprintf_buf_len];
  char *p = buf;
  int numbytes;
  int fd = fileno(stream);

  va_list ap;
	va_start(ap,format);
	numbytes = vsnprintf(buf,qfprintf_buf_len,format,ap);
	va_end(ap);
	if( numbytes == -1 ){
		fprintf(stderr,"\nqfprintf(): size of static buffer needs to be extended.\n");
		ADM_assert(0);
	}
	if( fd == -1 ){
		fprintf(stderr,"\nqfprintf(): bad stream argument\n");
		ADM_assert(0);
	}
	qwrite(fd,p,numbytes);
}

size_t qfwrite(const void *ptr, size_t size, size_t  nmemb, FILE *stream){
  int fd = fileno(stream);
	if( fd == -1 ){
		fprintf(stderr,"\nqfwrite(): bad stream argument\n");
		ADM_assert(0);
	}
	return qwrite(fd,ptr,size*nmemb);
}

ssize_t qwrite(int fd, const void *buf, size_t numbytes){
  const char *p=(const char *)buf;
  char msg[msg_len];
  ssize_t ret = 0;
	while(1){
	 int rc = write(fd,p,numbytes);
		if( rc == numbytes ){
			ret+=rc;
			return ret;
		}
		if( rc > 0 ){
			p+=rc;
			numbytes-=rc;
			ret+=rc;
			continue;
		}
		if( rc == -1 && (errno == ENOSPC 
#ifndef __MINGW32__
|| errno == EDQUOT
#endif
) ){
		  uint8_t rc;
			if( qfile[fd].ignore )
				return -1;
			fprintf(stderr,"qwrite(): can't write to file \"%s\": %s\n",
				       (qfile[fd].filename?qfile[fd].filename:"__unknown__"),
			               (errno==ENOSPC?"filesystem full":"quota exceeded"));
			ADM_assert(snprintf(msg,msg_len,"can't write to file \"%s\": %s\n%s\n",
			                                (qfile[fd].filename?qfile[fd].filename:"__unknown__"),
			                                (errno==ENOSPC?"filesystem full":"quota exceeded"),
			                                "Please free up some space and press RETRY to try again.")!=-1);
			rc = GUI_Alternate(msg,"Ignore","Retry");
			if( rc == 0 /* ignore */ ){
				qfile[fd].ignore = 1;
				return -1;
			}
			continue;
		}
		ADM_assert(snprintf(msg,msg_len,"can't write to file \"%s\": %u (%s)\n",
					        (qfile[fd].filename?qfile[fd].filename:"__unknown__"),
						errno, strerror(errno))!=-1);
		fprintf(stderr,"qwrite(): %s",msg);
		GUI_Error_HIG(msg,NULL);
		return -1;
	}
}

int qfclose(FILE *stream){
  int fd = fileno(stream);
	if( fd == -1 ){
		fprintf(stderr,"\nqfclose(): bad stream argument\n");
		ADM_assert(0);
	}
        if( qfile[fd].filename ){
		ADM_dealloc(qfile[fd].filename);
		qfile[fd].filename = NULL;
	}
	qfile[fd].ignore = 0;
	return( fclose(stream) );
}

