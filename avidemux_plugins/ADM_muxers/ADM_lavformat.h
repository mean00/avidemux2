//
// C++ Interface: ADM_lavformat
//
// Description: 
//
//	iface to libavformat	
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ADM_LAVFORMAT_H
#define ADM_LAVFORMAT_H
#include "ADM_bitstream.h"
typedef enum
{
        MUXER_NONE=0,
        MUXER_DVD,
		MUXER_VCD,
		MUXER_SVCD,
        MUXER_TS,
        MUXER_MP4,
        MUXER_PSP,
        MUXER_FLV,
        MUXER_MATROSKA,
        MUXER_DUMMY
}ADM_MUXER_TYPE;

/*
class ADMMpegMuxer
{
protected:
                uint32_t _frameNo;
                uint32_t _fps1000;
                uint32_t _audioByterate;
                uint32_t _audioFq;
                uint32_t _total; // deprecated, only for muxts
                uint64_t _lastAudioDts;
                uint32_t _running;
                ADM_MUXER_TYPE _type;
                uint8_t  _restamp;
                ADM_MUXER_TYPE _muxerType;

public:
        virtual uint8_t open(const char *filename,uint32_t inbitrate, ADM_MUXER_TYPE type, 
                                aviInfo *info, WAVHeader *audioheader)=0;
        virtual uint8_t writeAudioPacket(uint32_t len, uint8_t *buf)=0;
        virtual uint8_t writeVideoPacket(ADMBitstream *bitstream )=0;
        virtual uint8_t forceRestamp(void)=0;
        virtual uint8_t close( void )=0;
        virtual uint8_t audioEmpty( void)=0;
        virtual uint8_t needAudio(void)=0;
        virtual uint8_t audioEof(void) {return 1;}

                ADMMpegMuxer(void) {};
                virtual ~ADMMpegMuxer(void) {};

};
class lavMuxer : public ADMMpegMuxer
{
private:
                uint64_t  sample2time_us( uint32_t sample );
		

public:
		lavMuxer(void );
		~lavMuxer(  );
	virtual uint8_t open(const char *filename,uint32_t inbitrate, ADM_MUXER_TYPE type, aviInfo *info, WAVHeader *audioheader);
        virtual uint8_t open(const char *filename,uint32_t inbitrate, ADM_MUXER_TYPE type, aviInfo *info,uint32_t videoExtraDataSize,
                        uint8_t *videoExtraData, WAVHeader *audioheader,uint32_t audioextraSize,uint8_t *audioextraData);
        
        virtual uint8_t writeAudioPacket(uint32_t len, uint8_t *buf,uint32_t sample);
        virtual uint8_t writeAudioPacket(uint32_t len, uint8_t *buf) { ADM_assert(0);return 1;}
        virtual uint8_t writeVideoPacket(ADMBitstream *bitstream );
        virtual uint8_t forceRestamp(void);
        virtual uint8_t close( void );
        virtual uint8_t audioEmpty( void);
        virtual uint8_t needAudio(void);

	

};

class mplexMuxer : public ADMMpegMuxer
{
protected:
               
public:
                mplexMuxer(void );
                ~mplexMuxer(  );
        virtual uint8_t open(const char *filename,uint32_t inbitrate, ADM_MUXER_TYPE type, aviInfo *info, WAVHeader *audioheader);
        virtual uint8_t writeVideoPacket(ADMBitstream *bitstream );
        virtual uint8_t writeAudioPacket(uint32_t len, uint8_t *buf);
        virtual uint8_t forceRestamp(void);
        virtual uint8_t close( void );
        virtual uint8_t audioEmpty( void);
        virtual uint8_t needAudio(void);
        virtual uint8_t audioEof(void);
        virtual uint8_t videoEof(void);

};*/
#define TS_PACKET_SIZE 188

typedef struct
{
    uint64_t pts;
    uint8_t  packet[TS_PACKET_SIZE];
}entryPacket;

typedef struct
{
    uint32_t pid;
    uint32_t counter;
    uint32_t tableId;
    uint32_t sectionId;
}channel;
/*
class tsMuxer : public ADMMpegMuxer
{
protected:
        uint32_t    audioPacket;
        entryPacket *packetPipe;
        uint32_t    packetHead,packetTail;
        FILE        *outFile;     
        uint32_t    audioPid,videoPid;
        uint32_t    _curPTS;
        uint32_t    nbPacket;
        uint32_t    packetSincePAT;
        uint64_t    lastPCR;
        uint8_t     *audioBuffer;
        uint32_t    audioFill;
        uint8_t     *pesBuffer;
        
        entryPacket *getPacket( void);   
        uint8_t     writeSection( uint32_t pid,channel *chan, 
                                uint8_t *data, uint32_t len) ;
        uint8_t     writePmt( void); 
        uint8_t     writePat( void);                       
        uint8_t     writePacket(uint8_t *data, uint32_t len, uint64_t pcr,channel *chan,uint8_t start);
        uint8_t     writePacketPad(uint8_t *data, uint32_t len, uint64_t pcr,channel *chan,uint8_t start);
        uint64_t    audioTime( uint32_t time)   ;            // Time is us
        uint64_t    videoTime( uint32_t frameno);   // Time in us                    
        uint8_t     flushPackets(uint8_t r);
        uint8_t     writeAudioPacket2(void);
        uint8_t     pes2ts(channel *chan,uint64_t pcr,uint8_t tim );
        uint8_t     writeVideoPacket2(uint32_t len, uint8_t *buf,uint32_t frameno,uint32_t displayframe );
        channel     audioChannel,videoChannel,pmt,pat;
        aviInfo     _info;
        WAVHeader   _wavHeader;
             
public:
                tsMuxer(void );
                ~tsMuxer(  );
        virtual uint8_t open(const char *filename,uint32_t inbitrate, ADM_MUXER_TYPE type, aviInfo *info, WAVHeader *audioheader);
        virtual uint8_t writeVideoPacket(ADMBitstream *bitstream );
        virtual uint8_t writeAudioPacket(uint32_t len, uint8_t *buf);
        virtual uint8_t forceRestamp(void);
        virtual uint8_t close( void );
        virtual uint8_t audioEmpty( void);
        virtual uint8_t needAudio(void);

};
*/
#endif
//EOF

