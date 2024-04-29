/**
                \fn ADM_memFile.cpp
                \brief temporary file in memory
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
#include "ADM_memFile.h"
#include "DIA_coreToolkit.h"
#include "ADM_assert.h"


struct mfile_t {
    char *filename;
    MFILE * mfile;
};


#define mfile_len 32768
#define mfprintf_max_len 8192
#define mfile_prealloc (mfprintf_max_len*2)
static mfile_t mfile[mfile_len];


uint8_t  memFileInit(void)
{
    memset(mfile,0,sizeof(mfile));  
    return 1;
}

static int getMfdFromPath(const char *path)
{
    for (int i=0; i<mfile_len; i++)
    {
        if (mfile[i].filename != NULL)
        {
            if (strcmp(path, mfile[i].filename) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

static int getMfdFromMFILE(const MFILE * stream)
{
    for (int i=0; i<mfile_len; i++)
    {
        if (mfile[i].filename != NULL)
        {
            if (stream == mfile[i].mfile)
            {
                return i;
            }
        }
    }
    return -1;
}

static int getFirstFree(void)
{
    for (int i=0; i<mfile_len; i++)
    {
        if (mfile[i].filename == NULL)
        {
            return i;
        }
    }
    return -1;
}

MFILE *mfopen(const string &fileName, const char *mode)
{
    return mfopen(fileName.c_str(),mode);
}

MFILE *mfopen(const char *path, const char *mode)
{
    int fd = getMfdFromPath(path);

    if (fd >= 0)
    {
        mfile[fd].mfile->seek = 0;
        return mfile[fd].mfile;
    }
    
    fd = getFirstFree();
    if (fd < 0)
    {
        return NULL;
    }
    mfile[fd].filename = ADM_strdup(path);
    ADM_assert(mfile[fd].filename != NULL);
    mfile[fd].mfile = (MFILE*)malloc(sizeof(MFILE));
    ADM_assert(mfile[fd].mfile != NULL);
    
    mfile[fd].mfile->buf = (char*)malloc(mfprintf_max_len);
    mfile[fd].mfile->allocated = mfprintf_max_len;
    mfile[fd].mfile->length = 0;
    mfile[fd].mfile->seek = 0;
    
    return mfile[fd].mfile;
}

void mfprintf(MFILE *stream, const char *format, ...){
  static char buf [mfprintf_max_len];
  char *p = buf;
  int numbytes;

  va_list ap;
	va_start(ap,format);
	numbytes = vsnprintf(buf,mfprintf_max_len,format,ap);
	va_end(ap);
	if( numbytes == -1 ){
		fprintf(stderr,"\nmfprintf(): size of static buffer needs to be extended.\n");
		ADM_assert(0);
	}
	mfwrite(p,1,numbytes,stream);
}

size_t mfwrite(const void *ptr, size_t size, size_t  nmemb, MFILE *stream)
{
    size_t total = size*nmemb;
    if (total == 0)
    {
        return 0;
    }
    if ((stream->seek+total) > stream->allocated)
    {
        stream->allocated = stream->seek+total;
        stream->allocated += mfprintf_max_len;
        stream->buf = (char*)realloc(stream->buf, stream->allocated);
    }
    ADM_assert(stream->buf!=NULL);
    memcpy(stream->buf + stream->seek, ptr, total);
    if ((stream->seek+total) > stream->length)
    {
        stream->length = stream->seek+total;
    }
    stream->seek += total; 
    return nmemb;
}

int mfseek (MFILE * stream, ssize_t offset, int origin)
{
    if (!stream)
        return -1;
    if (origin==SEEK_SET)
    {
        ssize_t newSeek = offset;
        if (newSeek < 0)
            return -1;
        if (newSeek > stream->length)
            return -1;
        stream->seek = newSeek;
        return 0;
    }
    if (origin==SEEK_CUR)
    {
        ssize_t newSeek = stream->seek + offset;
        if (newSeek < 0)
            return -1;
        if (newSeek > stream->length)
            return -1;
        stream->seek = newSeek;
        return 0;
    }
    return -1;
}

char * mfgets (char * str, int num, MFILE * stream)
{
    if (!stream)
        return NULL;
    if (stream->seek >= stream->length)
    {
        return NULL;
    }
    
    int len=0;
    while(num--)
    {
        if (stream->seek >= stream->length)
            break;
        str[len] = stream->buf[stream->seek];
        len++;
        stream->seek++;
        if (str[len-1]=='\n')
            break;
    }
    str[len] = 0;
    return str;
}

int mfclose(MFILE *stream)
{
    return 0;   // dummy close
}

ADM_COREUTILS6_EXPORT void mfcleanup(const std::string &name)
{
    mfcleanup(name.c_str());
}

ADM_COREUTILS6_EXPORT void mfcleanup(const char * name)
{
    int fd = getMfdFromPath(name);
    if (fd < 0)
    {
        printf("[mfcleanup] already destroyed memFile \"%s\"\n",name);
        return;
    }
 
    printf("[mfcleanup] destroying memFile \"%s\"\n",name);
    
    ADM_dezalloc(mfile[fd].filename);
    free(mfile[fd].mfile->buf);
    free(mfile[fd].mfile);
    mfile[fd].filename = NULL;
    mfile[fd].mfile = NULL;    
}

