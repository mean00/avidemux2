#ifndef DMX_DMX_TS
#define DMX_DMX_TS

#include "ADM_coreDemuxerMpeg6_export.h"
 
#include "dmx_demuxer.h"
#include "dmx_identify.h"
#define MAX_TS_BUFFER (2*1024*1024) // should be safe enough, 2 MB for a full compressed frame,even for HDTV
#define MAX_TS_STREAM 50       // should be enough too :)


#define TS_PACKET_SIZE       188
#define TS2_PACKET_SIZE      192 // .m2ts
#define TS_SYNC_BYTE         0x47
#define TS_UNIT_START        0x40
#define TS_UNIT_PAYLOAD_ONLY 0x10
#define TS_UNIT_PAYLOAD_AF   0x30
#define TS_UNBOUND_SIZE      0x10000000
#define TS_ALL_PID           0x2000
/*
        A bit of explanation here.
        The demuxer will take each packet and lookup what it is
        If it is the seeked pes,it will copy its content into _pesBuffer
        If not it will keep statical info and continue

        The pid is like that :
                For simple pes-pid it is the pes_pid (0xC0, 0xE0,...)
                For data stored in private stream 1 (LPCM, AC3,...), the value is 0xFF00 + sub stream id
                        with sub stream id : 0-8 for AC3, A0-A8 for LPCM, 


*/
class ADM_COREDEMUXER6_EXPORT dmx_demuxerTS: public dmx_demuxer
 {
          protected : 
                  uint64_t      stampAbs;
                  uint32_t      consumed;
                  fileParser    *parser;
                  uint32_t       myPid;           // pid: high part =0xff if private stream, 00 if not
                  uint32_t      isPsi;
                  uint8_t       *_pesBuffer;
                  uint32_t      TS_PacketSize;
                  uint32_t      _pesBufferIndex; // current position in pesBuffer
                  uint64_t      _pesBufferStart;
                  uint32_t      _pesBufferLen;
                  uint64_t      _pesPTS;
                  uint64_t      _pesDTS;

                  uint32_t      packMode;
                  uint32_t      packLen;


                  uint64_t      _oldPesStart;    // Contains info for previous packet of same pid
                  uint32_t      _oldPesLen;      // useful when need to go back (after video startcode)
                  uint64_t      _oldPTS;
                  uint64_t      _oldDTS;
                  uint32_t      _probeSize;      // If not nul, we will only seek to this
                  uint32_t      maxPid;

                  uint64_t      seen[256];                                  
                  uint8_t       allPid[0x2000];    
                  uint8_t       *tracked   ;
                  uint32_t      nbTracked;
                
                  uint8_t       refill(void);
                  uint8_t       getInfoPES(uint32_t *consumed,uint64_t *dts,uint64_t *pts,uint8_t *stream,
                                        uint8_t *substream, uint32_t *lenPes);
                  uint8_t       getInfoPSI(uint32_t *oconsumed,uint32_t *olen);
                  uint8_t       updateTracker(uint32_t trackerPid,uint32_t nbData);
                  
          public:
                           dmx_demuxerTS(uint32_t nb,MPEG_TRACK *tracks,uint32_t psi,DMX_TYPE muxType) ;
                virtual    ~dmx_demuxerTS();             
                
                     uint8_t      open(const char *name);
                 
                  fileParser      *getParser(void) {return parser;}
                  uint8_t         forward(uint32_t f);
                  uint8_t         stamp(void); 
                  uint8_t         readPacket(uint32_t *opid,uint32_t *oleft, uint32_t *isPayloadStart,
                                        uint64_t *ostart,uint32_t *occ);

                  uint64_t        elapsed(void);
                
                  uint8_t         getPos( uint64_t *abs,uint64_t *rel);
                  uint8_t         setPos( uint64_t abs,uint64_t  rel);
                
                  uint64_t        getSize( void) { return _size;}          
                  uint8_t         setProbeSize(uint32_t probe);                
                
                  uint32_t        read(uint8_t *w,uint32_t len);
                  uint8_t         sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts, uint64_t *dts);
                  uint8_t         syncH264( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts);
                  uint8_t         hasAudio(void) { return 1;} // MAYBE has audio
                  uint8_t         getStats(uint64_t *stat);
                  uint8_t         changePid(uint32_t newpid,uint32_t newpes);
virtual           uint8_t         readPes(uint8_t *data, uint32_t *pesBlockLen, uint32_t *dts,uint32_t *pts);
// Inlined
uint8_t         read8i(void)
{
        uint8_t r;
        
        if(_pesBufferIndex<_pesBufferLen)
        {
                consumed++;
                r=_pesBuffer[_pesBufferIndex++];
                return r;
        }

        read(&r,1);
        return r;
}
uint16_t         read16i(void)
{
 uint16_t r;
uint8_t p[2];
        
        if(_pesBufferIndex+2<=_pesBufferLen)
        {
                consumed+=2;
                r=((_pesBuffer[_pesBufferIndex])<<8)+(_pesBuffer[_pesBufferIndex+1]);
                _pesBufferIndex+=2;
                return r;
        }

        read(p,2);
        r=(p[0]<<8)+p[1];
        return r;
}
uint32_t         read32i(void)                 
{
uint32_t r;
uint8_t  *p;
uint8_t b[4];        
        if(_pesBufferIndex+4<=_pesBufferLen)
        {
                consumed+=4;
                p=&(_pesBuffer[_pesBufferIndex]);
                _pesBufferIndex+=4;
        }
        else
        {
                read(b,4);
                p=b;
        }
        r=(p[0]<<24)+(p[1]<<16)+(p[2]<<8)+(p[3]);
        return r;
}
                
                  
};      
        

#endif
