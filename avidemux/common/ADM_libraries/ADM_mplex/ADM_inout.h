//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _ADM_IN_OUT_
#define _ADM_IN_OUT_

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#include "ADM_transfert.h"

class FileOutputStream : public OutputStream
{
public:
    FileOutputStream( const char *filename_pat );
    virtual int  Open( );
    virtual void Close();
    virtual off_t SegmentSize( );
    virtual void NextSegment();
    virtual void Write(uint8_t *data, unsigned int len);

private:
    FILE *strm;
    int strm_fd;
    char filename_pat[MAXPATHLEN];
    char cur_filename[MAXPATHLEN];

};

class IFileBitStream : public IBitStream
{
public:
        IFileBitStream( PacketQueue *inQueue, mplexStreamDescriptor *streamDesc,unsigned int buf_size = BUFFER_SIZE);
        ~IFileBitStream();

private:
        PacketQueue       *queue;
        virtual size_t ReadStreamBytes( uint8_t *buf, size_t number ) ;
        virtual bool EndOfStream() ;
        
};

#endif
//EOF
