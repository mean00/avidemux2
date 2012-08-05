/** *************************************************************************
        \file  avilist_avi
        \brief avi extension to avilist


    copyright            : (C) 2002/2012 by mean
                           (C) Feb 2005 by GMV: ODML write support
    GPL V2.0
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>
#include "ADM_muxer.h"

#include "fourcc.h"
#include "avilist_avi.h"

#define aprintf printf

#define w32(x) write32(hdr.x);
#define w16(x) write16(hdr.x);
/**
    \class ADMMemioAvi
    \brief Serialize structure into memory prior to writing them
*/
class ADMMemioAvi : public ADMMemio
{
public:
              ADMMemioAvi(int sz) : ADMMemio(sz) {}
    virtual ~ADMMemioAvi() {}
    bool    writeMainHeaderStruct(const MainAVIHeader &hdr);
    bool    writeStreamHeaderStruct(const AVIStreamHeader &hdr);
    bool    writeBihStruct(const ADM_BITMAPINFOHEADER &hdr);
    bool    writeWavStruct(const WAVHeader &hdr);
};


#ifndef ADM_CPU_X86 //-----------------------------------------------------
bool    ADMMemioAvi::writeMainHeaderStruct(const MainAVIHeader &hdr)
{
    w32(dwMicroSecPerFrame);	// frame display rate (or 0L)
    w32(dwMaxBytesPerSec);	// max. transfer rate
    w32(dwPaddingGranularity);	// pad to multiples of this
					// size; normally 2K.
    w32(dwFlags);		// the ever-present flags
    w32(dwTotalFrames);		// # frames in file
    w32(dwInitialFrames);
    w32(dwStreams);
    w32(dwSuggestedBufferSize);

    w32(dwWidth);
    w32(dwHeight);

   
    write32(0);
    write32(0);
    write32(0);
    write32(0);
    return true;
}
bool    ADMMemioAvi::writeStreamHeaderStruct(const AVIStreamHeader &hdr)
{
    w32(	fccType);
	w32(	fccHandler);
	w32(	dwFlags);	/* Contains AVITF_* flags */
	w16(	wPriority);	/* dwPriority - splited for audio */
	w16(	wLanguage);
	w32(	dwInitialFrames);
	w32(	dwScale);
	w32(	dwRate);		/* dwRate / dwScale == samples/second */
	w32(	dwStart);
	w32(	dwLength);	/* In units above... */
	w32(	dwSuggestedBufferSize);
	w32(	dwQuality);
	w32(	dwSampleSize);
	w16( rcFrame.left);
    w16( rcFrame.top);
    w16( rcFrame.right);
    w16( rcFrame.bottom);
    return true;
}
bool    ADMMemioAvi::writeBihStruct(const ADM_BITMAPINFOHEADER &hdr)
{
    w32( 	biSize);
    w32(  	biWidth);
    w32(  	biHeight);
    w16( 	biPlanes);
    w16( 	biBitCount);
    w32( 	biCompression);
    w32( 	biSizeImage);
    w32(  	biXPelsPerMeter);
    w32(  	biYPelsPerMeter);
    w32( 	biClrUsed);
    w32( 	biClrImportant);
    return true;
}
bool    ADMMemioAvi::writeWavStruct(const WAVHeader &hdr)
{
    w16(	encoding);	
    w16(	channels);					/* 1 = mono, 2 = stereo */
    w32(	frequency);				/* One of 11025, 22050, or 44100 48000 Hz */
    w32(	byterate);					/* Average bytes per second */
    w16(	blockalign);				/* Bytes per sample block */
    w16(	bitspersample);		/* One of 8, 1 */
    return true;
}
#else // -------------------------------------------------------
bool    ADMMemioAvi::writeMainHeaderStruct(const MainAVIHeader &hdr)
{
    write(sizeof(hdr),(const uint8_t *)&hdr);
    return true;
}
bool    ADMMemioAvi::writeStreamHeaderStruct(const AVIStreamHeader &hdr)
{
    write(sizeof(hdr),(const uint8_t *)&hdr);
    return true;
}
bool    ADMMemioAvi::writeBihStruct(const ADM_BITMAPINFOHEADER &hdr)
{
    write(sizeof(hdr),(const uint8_t *)&hdr);
    return true;
}
bool    ADMMemioAvi::writeWavStruct(const WAVHeader &hdr)
{
    write(sizeof(hdr),(const uint8_t *)&hdr);
    return true;
}
#endif  //------------------------------------------------------------------

bool AviListAvi::WriteMem(const ADMMemio &mem)
{
    Write( mem.getBuffer(),mem.size());
    return true;
}

/**
     \fn writeStrh
     \brief write a complete strh chunk
*/
bool  AviListAvi::writeStrh(const AVIStreamHeader &hdr)
{
    ADMMemioAvi mem(sizeof(AVIStreamHeader));
    mem.writeStreamHeaderStruct(hdr);
    WriteChunkMem("strh",mem);
    return true;
}

/**
    \fn writeStrfBih
*/
bool  AviListAvi::writeStrfBih(const ADM_BITMAPINFOHEADER &bih, int extraLen, uint8_t *extraData)
{

    int toWrite=sizeof(bih)+extraLen;
    ADMMemioAvi mem(toWrite);
    mem.writeBihStruct(bih);
    if(extraLen)
        mem.write(extraLen,extraData);
    WriteChunkMem("strf",mem);
    return true;
}

/**
    \fn writeStrfBih
*/
bool  AviListAvi::writeStrfWav(const WAVHeader &wav, int extraLen, uint8_t *extraData)
{
    int toWrite=sizeof(wav)+extraLen;
    ADMMemioAvi mem(toWrite);
    mem.writeWavStruct(wav);
    if(extraLen)
        mem.write(extraLen,extraData);
    WriteChunkMem("strf",mem);
    return true;
}
/**

*/
 bool    AviListAvi::writeMainHeaderStruct(const MainAVIHeader &hdr)
{
    ADMMemioAvi mem(sizeof(hdr));
    mem.writeMainHeaderStruct(hdr);
    WriteMem(mem);
    return true;
}
/**

*/
bool    AviListAvi::writeStreamHeaderStruct(const AVIStreamHeader &hdr)
{
    ADMMemioAvi mem(sizeof(hdr));
    mem.writeStreamHeaderStruct(hdr);
    WriteMem(mem);
    return true;

}
/**
    \fn writeDummyChunk
    \brief write a placeholder dummy chunk
*/
bool  AviListAvi::writeDummyChunk(int size, uint64_t *pos)
{
	// save file position
		*pos=Tell();
		aprintf("[ODML]write dummy chunk at file position %"PRIu64" with data size %"PRIu32"\n",*pos, size);
		// generate dummy data
		uint8_t* dummy=(uint8_t*)ADM_alloc (size);
		memset(dummy,0,size);
		// write dummy chunk
		WriteChunk ( (const uint8_t  *)"JUNK", size, dummy);
		// clean up
		ADM_dealloc (dummy);
        return true;
}
/**
    \fn fill
    \brief fill a chunk till the payload = maxSize
*/
bool  AviListAvi::fill(uint32_t maxSize)
{
    uint64_t pos=Tell();          // Current position
    uint64_t start=TellBegin()+8; // Payload start
    uint64_t end=start+maxSize;   // Next chunk
    if(pos>end) // overwrite!
    {
        ADM_error("Chunk already bigger than filler ( %d, filler=%d)\n",(int)pos-start,maxSize);
        return false;
    }
    if(pos==end)
    {
        ADM_info("Chunk already right size, nothing to do\n");
        return true;
    }
    int toFill=(int)(end-pos);
    printf("Current pos=%"PRIu64", next chunk at %"PRIu64", filling with %d\n",pos,end,toFill);
    uint8_t *buffer=new uint8_t[toFill];
    memset(buffer,0,toFill);
    Write(buffer,toFill);
    delete [] buffer;
    return true;
}
/**
    \fn EndAndPaddTilleSizeMatchesAviListAvi
    \brief Warning we assume everything is even
*/
bool  AviListAvi::EndAndPaddTilleSizeMatches(int sizeFilled)
{
    uint64_t pos=Tell();            // Current position
    uint64_t start=TellBegin()+8;   // Payload start
    uint64_t end=start+sizeFilled;  // Next chunk
    if(pos&1)
        ADM_backTrack("[AVI]CHUNK is at a even position",__LINE__,__FILE__);
    if(pos+8>end)
    {
        ADM_error("No space to add junk chunk ( %d, filler=%d)\n",(int)pos-start,sizeFilled);
        if(end<=pos)
        {
            ADM_error("CHUNK OVERFLOW ( %d, filler=%d)\n",(int)pos-start,sizeFilled);
            ADM_error("CHUNK OVERFLOW ( %d, filler=%d)\n",(int)pos-start,sizeFilled);
            ADM_error("CHUNK OVERFLOW ( %d, filler=%d)\n",(int)pos-start,sizeFilled);
            ADM_error("CHUNK OVERFLOW ( %d, filler=%d)\n",(int)pos-start,sizeFilled);       
            ADM_backTrack("CHUNK overflow",__LINE__,__FILE__);
            return false;
        }

        int left=(int)(end-pos);
        for(int i=0;i<left;i++) Write8(0); // we dont have enough space to put a junk()
        End();
        return true;
 
    }
    End();
    uint64_t left=end-8-pos;
    AviListAvi junk("JUNK",getFile());
    junk.Begin();
    for(int i=0;i<left;i++)  // Inefficient, but can be fairly large
            junk.Write8(0);
    junk.End();
    return true;

}

/**
    \fn writeChunkMem
    \brief 
*/
bool  AviListAvi::WriteChunkMem(const char *name,ADMMemio &mem)
{
        return WriteChunk((const uint8_t *)name,mem.size(),mem.getBuffer());
}

// EOF


