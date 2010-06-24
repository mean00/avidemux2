/***************************************************************************
                        Mpeg2 in PS indexer                                            
                             
    VC1: /!\ Escaping not done (yet)

    copyright            : (C) 2005/2009 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_includeFfmpeg.h"
#include "ADM_default.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

#include "dmxTSPacket.h"

#include "avidemutils.h"
#include "ADM_quota.h"
#include "ADM_tsAudioProbe.h"
#include "DIA_working.h"
#include "ADM_tsPatPmt.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"
#include "ADM_clock.h"
#include "ADM_getbits.h"
#include "ADM_indexFile.h"
#define zprintf(...) {}
static const char Structure[4]={'X','T','B','F'}; // X Top Bottom Frame
static const char Type[5]={'X','I','P','B','D'};

static const uint32_t FPS[16]={
                0,                      // 0
                23976,          // 1 (23.976 fps) - FILM
                24000,          // 2 (24.000 fps)
                25000,          // 3 (25.000 fps) - PAL
                29970,          // 4 (29.970 fps) - NTSC
                30000,          // 5 (30.000 fps)
                50000,          // 6 (50.000 fps) - PAL noninterlaced
                59940,          // 7 (59.940 fps) - NTSC noninterlaced
                60000,          // 8 (60.000 fps)
                0,                      // 9
                0,                      // 10
                0,                      // 11
                0,                      // 12
                0,                      // 13
                0,                      // 14
                0                       // 15
        };
static const uint32_t  VC1_ar[16][2] = {  // From VLC
                        { 0, 0}, { 1, 1}, {12,11}, {10,11}, {16,11}, {40,33},
                        {24,11}, {20,11}, {32,11}, {80,33}, {18,11}, {15,11},
                        {64,33}, {160,99},{ 0, 0}, { 0, 0}};

#define VC1_SEQ_SIZE 29
class TSVideo
{
public:
    TSVideo(void) {w=h=fps=interlaced=ar=pid=frameCount=fieldCount=0;extraDataLength=0;}
    uint32_t w;
    uint32_t h;
    uint32_t fps;
    uint32_t interlaced;
    uint32_t ar;
    uint32_t pid;
    uint32_t frameCount;
    uint32_t fieldCount;
    uint32_t extraDataLength;
    uint8_t  extraData[VC1_SEQ_SIZE];
};

typedef enum
{
    idx_startAtImage,
    idx_startAtGopOrSeq
}indexerState;

typedef enum
{
    pictureFrame=3,
    pictureTopField=1, 
    pictureBottomField=2
}pictureStructure;

typedef struct
{
    uint64_t pts,dts; //startAt;
    //uint32_t offset;
    uint32_t frameType;
    pictureStructure picStructure;
    uint32_t nbPics;
    indexerState state;
    tsPacketLinear *pkt;
    int32_t        nextOffset;
    uint64_t beginPts,beginDts;
    uint64_t prevPts,prevDts;
}indexerData;

typedef enum
{
    markStart,
    markEnd,
    markNow
}markType;

class VC1Context
{
public:
        bool advanced;
        bool interlaced;
        bool interpolate;
        VC1Context() {advanced=false;interlaced=false;interpolate=false;}

};


#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
    \class TsIndexer
*/
class TsIndexer
{
protected:
        uint32_t        currentFrameType;
        uint32_t        beginConsuming;
        indexerState    currentIndexState;
        uint64_t        fullSize;
        Clock           ticktock;
        VC1Context      vc1Context;
protected:
        FILE                    *index;
        tsPacketLinearTracker   *pkt;
        listOfTsAudioTracks     *audioTracks;
        DIA_workingBase         *ui;
        void                    updateUI(void);
        bool                    decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength);
        bool                    decodeVC1Seq(int nb, uint8_t *data,TSVideo &video);
        bool                    decodeVC1Pic(int nb, uint8_t *dataBuffer,uint32_t &frameType,uint32_t &frameStructure);
public:
                TsIndexer(listOfTsAudioTracks *tr);
                ~TsIndexer();
        bool    runMpeg2(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runH264(const char *file,ADM_TS_TRACK *videoTrac);
        bool    runVC1(const char *file,ADM_TS_TRACK *videoTrac);
        bool    writeVideo(TSVideo *video,ADM_TS_TRACK_TYPE trkType);
        bool    writeAudio(void);
        bool    writeSystem(const char *filename,bool append);
        bool    Mark(indexerData *data,dmxPacketInfo *s,uint32_t overRead);
        bool    updatePicStructure(TSVideo &video,indexerData &idata, const uint32_t t)
                        {
                                            switch(t)
                                            {
                                                case 3: video.frameCount++;
                                                        idata.picStructure=pictureFrame;
                                                        break;
                                                case 1:  idata.picStructure=pictureTopField;
                                                         video.fieldCount++;
                                                         break;
                                                case 2:  idata.picStructure=pictureBottomField;
                                                         video.fieldCount++;
                                                         break;
                                                default: ADM_warning("frame type 0 met, this is illegal\n");
                                            }
                                            return true;
                        }
};
/**
      \fn TsIndexer 
      \brief main indexing loop for mpeg2 payload
*/
uint8_t   tsIndexer(const char *file)
{
bool r;

    ADM_TS_TRACK *tracks;
    uint32_t nbTracks;
    listOfTsAudioTracks audioTrack;

    if(TS_scanForPrograms(file,&nbTracks,&tracks)==false) 
    {
        printf("[Ts Indexer] Scan of pmt failed\n");
        return false;
    }
    ADM_assert(tracks);
    ADM_assert(nbTracks);
    //
    // Now extract the datas from audio tracks & verify they are here
    tsPacketLinear *p=new tsPacketLinear(0);
    p->open(file,FP_DONT_APPEND);
    for(int i=1;i<nbTracks;i++)
    {
        tsAudioTrackInfo trk;
        trk.esId=tracks[i].trackPid;
        trk.trackType=tracks[i].trackType;
        if(true==tsGetAudioInfo(p,&trk))
        {
              audioTrack.push_back(trk);  
        }
    }
    delete p;
    printf("[TsIndexer] Audio probed, %d found, doing video\n",(int)audioTrack.size());
    //
    TsIndexer *dx=new TsIndexer(&audioTrack);
    switch(tracks[0].trackType)
    {
            case ADM_TS_MPEG2: 
                            r=dx->runMpeg2(file,&(tracks[0]));
                            break;
            case ADM_TS_VC1: 
                            r=dx->runVC1(file,&(tracks[0]));
                            break;
            case ADM_TS_H264: 
                            r=dx->runH264(file,&(tracks[0]));
                            break;
            default:
                        r=0;
                        break;
    }
    delete dx;
    delete [] tracks;
    return r;
}

/**
    \fn TsIndexer
*/
TsIndexer::TsIndexer(listOfTsAudioTracks *trk)
{
    index=NULL;
    pkt=NULL;
    audioTracks=NULL;
    ui=createWorking ("Indexing");
    audioTracks=trk;
    ticktock.reset();
}

/**
    \fn ~TsIndexer
*/
TsIndexer::~TsIndexer()
{
    if(index) qfclose(index);
    if(pkt) delete pkt;
    if(ui) delete ui;
    ui=NULL;
}
/**
    \fn updateUI
*/
void TsIndexer::updateUI(void)
{
        if(ticktock.getElapsedMS()<1000) return;
        ticktock.reset();
        uint64_t p;
        p=pkt->getPos();
        float pos=p;
        pos=pos/(float)fullSize;
        pos*=100;
        ui->update( (uint32_t)pos);
}
/**

*/
static uint32_t unescapeH264(uint32_t len,uint8_t *in, uint8_t *out)
{
  uint32_t outlen=0;
  uint8_t *tail=in+len;
    if(len<3) return 0;
    while(in<tail-3)
    {
      if(!in[0]  && !in[1] && in[2]==3)
      {
        out[0]=0;
        out[1]=0;
        out+=2;
        outlen+=2;
        in+=3; 
      }
      *out++=*in++;
      outlen++;
    }
    // copy last bytes
    uint32_t left=tail-in;
    memcpy(out,in,left);
    outlen+=left;
    return outlen;
    
}
/**
        \fn decodeSEI
        \brief decode SEI to get short ref I
        @param recoveryLength # of recovery frame
        \return true if recovery found
*/
bool TsIndexer::decodeSEI(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength)
{
    GetBitContext s;
    uint8_t *payload=(uint8_t *)alloca(nalSize+16);
    nalSize=unescapeH264(nalSize,org,payload);
    init_get_bits(&s, payload, nalSize*8);
    while( get_bits_count(&s)<(nalSize-4)*8)
    {
        uint32_t sei_type=get_bits(&s,8);
        uint32_t sei_size=get_bits(&s,8);
                if(sei_size==0xff) sei_size=0xff00+get_bits(&s,8);; // should be enough
                zprintf("  [SEI] Type : 0x%x size:%d\n",sei_type,sei_size);
                if(sei_type==6) // Recovery point
                {
                        *recoveryLength=get_ue_golomb(&s);
                        zprintf("[SEI] Recovery :%"LU"\n",*recoveryLength);
                        return true;
                }
                skip_bits(&s,sei_size*8);
    }
    return false;
}

/**
    \fn runH264
    \brief Index H264 stream
*/  
bool TsIndexer::runH264(const char *file,ADM_TS_TRACK *videoTrac)
{
bool    pic_started=false;
bool    seq_found=false;

TSVideo video;
indexerData  data;    
dmxPacketInfo info;
TS_PESpacket SEI_nal(0);
bool result=false;
uint32_t recoveryCount=0xff;

    printf("Starting H264 indexer\n");
    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_H264)
    {
        printf("[Ts Indexer] Only H264 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;

    char *indexName=(char *)alloca(strlen(file)+5);
    sprintf(indexName,"%s.idx2",file);
    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        return false;
    }

    writeSystem(file,true);
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
    int lastRefIdc=0;
    //******************
    // 1 search SPS
    //******************
#define SPS_READ_AHEAD 32
      while(1)
      {
        int startCode=pkt->findStartCode();

        if(startCode&0x80) continue; // Marker missing
        startCode&=0x1f;
        if(startCode!=NAL_SPS) continue;

          // Got SPS!
          uint8_t buffer[60] ; // should be enough
          uint32_t xA,xR;
          // Get info
          pkt->getInfo(&info);
          pkt->read(SPS_READ_AHEAD,buffer);
          if (extractSPSInfo(buffer, SPS_READ_AHEAD, &video.w,&video.h,&video.fps,&xA,&xR))
          {
              
              printf("[TsIndexer] Found video %"LU"x%"LU", fps=%"LU"\n",video.w,video.h,video.fps);
              seq_found=1;
              writeVideo(&video,ADM_TS_H264);
              writeAudio();
              qfprintf(index,"[Data]");
              // Rewind
              pkt->seek(info.startAt,info.offset-5);
              break;              
          };
      }
        if(!seq_found) goto the_end;
    //******************
    // 2 Index
    //******************
      while(1)
      {
        int startCode=pkt->findStartCode();
resume:
        if(!pkt->stillOk()) break;

//  1:0 2:Nal ref idc 5:Nal Type
        if(startCode&0x80) 
        {
            printf("[Ts] Nal Marker missing:%x\n",startCode);
            continue; // Marker missing
        }
        int fullStartCode=startCode;
        int ref=(startCode>>5)&3;

        startCode&=0x1f; // Ignore nal ref IDR
        
        zprintf("[%02x] Nal :0x%x,ref=%d,lastRef=%d at : %d \n",fullStartCode,startCode,ref,lastRefIdc,pkt->getConsumed()-beginConsuming);
        
          // Ignore multiple chunk of the same pic
          if((startCode==NAL_NON_IDR || startCode==NAL_IDR)&&pic_started )  //&& ref==lastRefIdc) 
          {
           // aprintf("Still capturing, ignore\n");
            continue;
          }
                
          switch(startCode)
                  {
                  case NAL_SEI:
                    {
                        // Load the whole NAL
                            SEI_nal.empty();
                            uint32_t code=0xffff+0xffff0000;
                            while((code!=1) && pkt->stillOk())
                            {
                                uint8_t r=pkt->readi8();
                                code=(code<<8)+r;
                                SEI_nal.pushByte(r);
                            }
                            if(!pkt->stillOk()) goto resume;
                            zprintf("[SEI] Nal size :%d\n",SEI_nal.payloadSize);
                            if(SEI_nal.payloadSize>=7)
                                decodeSEI(SEI_nal.payloadSize-4,SEI_nal.payload,&recoveryCount);
                            else printf("[SEI] Too short size+4=%d\n",*(SEI_nal.payload));
                        
                            startCode=pkt->readi8();
                            goto resume;
                    }
                            break;
                  case NAL_AU_DELIMITER:
                          pic_started = false;
                          break;

                  case NAL_SPS:
                            
                          pic_started = false;
                          aprintf("Sps \n");
                          pkt->getInfo(&info);
                          data.frameType=1;
                          Mark(&data,&info,5);
                          data.state=idx_startAtGopOrSeq;
                          recoveryCount=0xff;
                          break;

                  case NAL_IDR:
                    zprintf("KOWABOUNGA\n");
                  case NAL_NON_IDR:
                    {
#define NON_IDR_PRE_READ 8
                      zprintf("Pic start last ref:%d cur ref:%d nb=%d\n",lastRefIdc,ref,data.nbPics);
                      lastRefIdc=ref;
                        
                      uint8_t bufr[NON_IDR_PRE_READ+4];
                      uint8_t header[NON_IDR_PRE_READ+4];
                      GetBitContext s;
                   
                        pkt->read(NON_IDR_PRE_READ,bufr);
                        // unescape...
                        unescapeH264(NON_IDR_PRE_READ,bufr,header);
                        //
                        init_get_bits(&s, header, NON_IDR_PRE_READ*8);
                        int first_mb_in_slice,slice_type;

                        first_mb_in_slice= get_ue_golomb(&s);
                        slice_type= get_ue_golomb_31(&s);
                        if(slice_type>9) 
                        {
                            printf("[TsIndexer] Bad slice type\n");
                        }
                        if(slice_type>4) slice_type-=5;
                        switch(slice_type)
                        {

                            case 0 : data.frameType=2;break; // P
                            case 1 : data.frameType=3;break; // B
                            case 2 : data.frameType=1;break; // I
                            default : data.frameType=2;break; // SP/SI
                        }
                      if(startCode==NAL_IDR) data.frameType=4; // IDR
                      zprintf("[>>>>>>>>] Pic Type %"LU" Recovery %"LU"\n",data.frameType,recoveryCount);
                      if(data.frameType==1 && !recoveryCount) data.frameType=4; //I  + Recovery=0 = IDR!
                      if(data.state==idx_startAtGopOrSeq) 
                      {
                              currentFrameType=data.frameType;;
                              updateUI();
                              
                      }else
                      {
                            pkt->getInfo(&info);
                            Mark(&data,&info,5+NON_IDR_PRE_READ);
                       }
                      data.state=idx_startAtImage;
                      data.nbPics++;
                      pic_started = true;
                      recoveryCount=0xff;
                    }
                  
                    break;
                  default:
                      break;
          }
      } // End while
      result=true;
the_end:
        printf("\n");
        Mark(&data,&info,0);
        qfprintf(index,"\n[End]\n");
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return result; 
}
//***********************************************************************
/**
    \fn runMpeg2
*/  
bool TsIndexer::runMpeg2(const char *file,ADM_TS_TRACK *videoTrac)
{
uint32_t temporal_ref,val;
uint8_t buffer[50*1024];
bool seq_found=false;

TSVideo video;
indexerData  data;    
dmxPacketInfo info;

    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_MPEG2)
    {
        printf("[Ts Indexer] Only Mpeg2 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;

    char *indexName=(char *)alloca(strlen(file)+5);
    sprintf(indexName,"%s.idx2",file);
    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        return false;
    }
    writeSystem(file,true);
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
    int startCode;
#define likely(x) x
#define unlikely(x) x
      while(1)
      {
        startCode=pkt->findStartCode();
        if(!pkt->stillOk()) break;

          switch(startCode)
                  {
                  case 0xB3: // sequence start
                          if(seq_found)
                          {
                                  pkt->getInfo(&info);
                                  data.frameType=1;
                                  Mark(&data,&info,4);
                                  data.state=idx_startAtGopOrSeq;
                                  pkt->forward(8);  // Ignore
                                  continue;
                          }
                          //
                          seq_found=1;
                          val=pkt->readi32();
                          video.w=val>>20;
                          video.w=((video.w+15)&~15);
                          video.h= (((val>>8) & 0xfff)+15)& ~15;

                          video.ar = (val >> 4) & 0xf;
                          video.fps= FPS[val & 0xf];
                          pkt->forward(4);
                          writeVideo(&video,ADM_TS_MPEG2);
                          writeAudio();
                          qfprintf(index,"[Data]");
                          pkt->getInfo(&info);
                          data.frameType=1;
                          Mark(&data,&info,4+8);
                          data.state=idx_startAtGopOrSeq;
                          continue;

                          break;
                    case 0xB5: //  extension
                                { 
                                    uint8_t id=pkt->readi8()>>4;
                                    uint8_t two;
                                    switch(id)
                                    {
                                        case 1: // Sequence extension
                                            val=(val>>3)&1; // gop type progressive, unreliable, not used
                                            break;
                                        case 8: // picture coding extension (mpeg2)
                                        {
                                            // skip motion vector
                                            uint8_t picture_structure;
                                            pkt->forward(1); // 4*4 bits
                                            two=pkt->readi8();
                                            picture_structure=(two)&3;
                                            
                                            //printf("Picture type %02x struct:%x\n",two,picture_structure);
                                            updatePicStructure(video,data,picture_structure);
                                        }
                                        default:break;
                                    }
                                }
                                break;
                  case 0xb8: // GOP
                          // Update ui
                            {
                               updateUI();

                            }

                          if(!seq_found) continue;
                          if(data.state==idx_startAtGopOrSeq) 
                          {         
                                  continue;;
                          }
                          pkt->getInfo(&info);
                          Mark(&data,&info,4);
                          data.state=idx_startAtGopOrSeq;
                          break;
                  case 0x00 : // picture
                        {
                          int type;
                          markType update=markNow;
                          if(!seq_found)
                          { 
                                  continue;
                                  printf("[TsIndexer]No sequence start yet, skipping..\n");
                          }
                          
                          val=pkt->readi16();
                          temporal_ref=val>>6;
                          type=7 & (val>>3);
                          if( type<1 ||  type>3)
                          {
                                  printf("[Indexer]Met illegal pic at %"LLX" + %"LX"\n",
                                                  info.startAt,info.offset);
                                  continue;
                          }
                          
                          
                          if(data.state==idx_startAtGopOrSeq) 
                          {
                                  currentFrameType=type;
                          }else
                            {
                                  data.frameType=type;
                                  pkt->getInfo(&info);
                                  Mark(&data,&info,4+2);


                            }
                            data.state=idx_startAtImage;
                            data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
    
        printf("\n");
        Mark(&data,&info,2);
        qfprintf(index,"\n[End]\n");
        qfprintf(index,"\n# Found %"LU" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %"LU" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %"LU" field pictures\n",video.fieldCount); // Size
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return 1; 
}
/**
    \fn runVC1
    \brief Index VC1 stream
*/  
bool TsIndexer::runVC1(const char *file,ADM_TS_TRACK *videoTrac)
{
uint32_t temporal_ref,val;
uint8_t buffer[50*1024];
bool seq_found=false;

TSVideo video;
indexerData  data;    
dmxPacketInfo info;


    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_VC1)
    {
        printf("[Ts Indexer] Only VC1 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;

    char *indexName=(char *)alloca(strlen(file)+5);
    sprintf(indexName,"%s.idx2",file);
    index=qfopen(indexName,"wt");
    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName);
        return false;
    }
    writeSystem(file,true);
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
    int startCode;
#define likely(x) x
#define unlikely(x) x
      while(1)
      {
        startCode=pkt->findStartCode();
        if(!pkt->stillOk()) break;

          switch(startCode)
                  {
                  case 0x0f: // sequence start
                          uint8_t vc1Seq[VC1_SEQ_SIZE];
                          uint8_t h;
                          if(seq_found)
                          {
                                  pkt->forward(4);  // Ignore
                                  continue;
                          }
                          //
                          
                          h=pkt->readi8();
                          if(!h&0x80) // simple/main profile
                                continue;
                          vc1Seq[0]=h;

                          pkt->read(VC1_SEQ_SIZE-1,vc1Seq+1);
                          if(!decodeVC1Seq(VC1_SEQ_SIZE,vc1Seq,video)) continue;
                          video.extraDataLength=VC1_SEQ_SIZE+8;
                          memcpy(video.extraData+4,vc1Seq,VC1_SEQ_SIZE);
                          video.extraData[0]=0;
                          video.extraData[1]=0;
                          video.extraData[2]=1;
                          video.extraData[3]=0xf;
                          video.extraData[VC1_SEQ_SIZE+4+0]=0;
                          video.extraData[VC1_SEQ_SIZE+4+1]=0;
                          video.extraData[VC1_SEQ_SIZE+4+2]=1;
                          video.extraData[VC1_SEQ_SIZE+4+3]=0xfE;
                          seq_found=1;
                          // Hi Profile
                          printf("[VC1] Found seq start with %d x %d video\n",(int)video.w,(int)video.h);
                          printf("[VC1] FPS : %d\n",(int)video.fps);
                          //video.ar = (val >> 4) & 0xf;
                          //video.fps= FPS[val & 0xf];
                          writeVideo(&video,ADM_TS_VC1);
                          writeAudio();
                          qfprintf(index,"[Data]");
                          pkt->getInfo(&info);
                          data.frameType=1;
                          Mark(&data,&info,8);
                          data.state=idx_startAtGopOrSeq;
                          continue;

                          break;
                    case 0x0D: //  Picture start
                        { 
                          int type;
                          int consumed;
                          uint8_t buffer[4];
                          uint32_t fType,sType;
                          markType update=markNow;
                          if(!seq_found)
                          { 
                                  continue;
                                  printf("[TsIndexer]No sequence start yet, skipping..\n");
                          }      
                          pkt->read(2,buffer);
                          if(!decodeVC1Pic(2,buffer,fType,sType)) continue;
                          updatePicStructure(video,data,sType);
                          if(data.state==idx_startAtGopOrSeq) 
                          {
                                  currentFrameType=fType;
                          }else
                            {
                                  data.frameType=fType;
                                  pkt->getInfo(&info);
                                  Mark(&data,&info,consumed);
                            }
                            data.state=idx_startAtImage;
                            data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
    
        printf("\n");
        Mark(&data,&info,2);
        qfprintf(index,"\n[End]\n");
        qfprintf(index,"\n# Found %"LU" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %"LU" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %"LU" field pictures\n",video.fieldCount); // Size
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return 1; 
}
/**
    \fn   Mark
    \brief update the file

    The offset part is due to the fact that we read 2 bytes from the pic header to know the pic type.
    So when going from a pic to a pic, it is self cancelling.
    If the beginning is not a pic, but a gop start for example, we had to add/remove those.

*/
bool  TsIndexer::Mark(indexerData *data,dmxPacketInfo *info,uint32_t overRead)
{
      
        uint32_t consumed=pkt->getConsumed()-overRead;
        if(data->nbPics)
        {
            int64_t deltaPts,deltaDts;

            if(data->beginPts==-1 || data->prevPts==-1) deltaPts=-1;
                else deltaPts=data->prevPts-data->beginPts;

            if(data->beginDts==-1 || data->prevDts==-1) deltaDts=-1;
                else deltaDts=data->prevDts-data->beginDts;

            qfprintf(index," %c%c:%06"LX":%"LLD":%"LLD,
                                    Type[currentFrameType],
                                    Structure[data->picStructure&3],
                                    consumed-beginConsuming,
                                    deltaPts,deltaDts);
            beginConsuming=consumed;
        }else
        {  // Our first Pic
            beginConsuming=0;
            pkt->setConsumed(overRead);
        }
            
        // If audio, also dump audio
        if(data->frameType==1 || data->frameType==4) // I or IDR
        {
            data->beginPts=info->pts;
            data->beginDts=info->dts;
            if(audioTracks)
            {
                qfprintf(index,"\nAudio bf:%08"LLX" ",info->startAt);
                packetTSStats *s;
                uint32_t na;
                pkt->getStats(&na,&s);      
                ADM_assert(na==audioTracks->size());
                for(int i=0;i<na;i++)
                {   
                    packetTSStats *current=s+i;
                    qfprintf(index,"Pes:%x:%08"LLX":%"LD":%"LLD" ",
                                current->pid,current->startAt,current->startSize,current->startDts);
                }                
            }
            // start a new line
            qfprintf(index,"\nVideo at:%08"LLX":%04"LX" Pts:%08"LLD":%08"LLD" ",info->startAt,info->offset-overRead,info->pts,info->dts);
        }
        currentFrameType=data->frameType;
        data->prevDts=info->dts;
        data->prevPts=info->pts;
    return true;
}

/**
    \fn writeVideo
    \brief Write Video section of index file
*/
bool TsIndexer::writeVideo(TSVideo *video,ADM_TS_TRACK_TYPE trkType)
{
    qfprintf(index,"[Video]\n");
    qfprintf(index,"Width=%d\n",video->w);
    qfprintf(index,"Height=%d\n",video->h);
    qfprintf(index,"Fps=%d\n",video->fps);
    qfprintf(index,"Interlaced=%d\n",video->interlaced);
    qfprintf(index,"AR=%d\n",video->ar);
    qfprintf(index,"Pid=%d\n",video->pid);
    if(video->extraDataLength)
    {
        qfprintf(index,"ExtraData=%d ",video->extraDataLength);
        for(int i=0;i<video->extraDataLength;i++)
            qfprintf(index," %02x",video->extraData[i]);
        qfprintf(index,"\n");
    }
 switch(trkType)
    {
        case ADM_TS_MPEG2: qfprintf(index,"VideoCodec=Mpeg2\n");break;;
        case ADM_TS_H264:  qfprintf(index,"VideoCodec=H264\n");break;
        case ADM_TS_VC1:   qfprintf(index,"VideoCodec=VC1\n");break;
        default: printf("[TsIndexer] Unsupported video codec\n");return false;

    }
    return true;
}
/**
    \fn writeSystem
    \brief Write system part of index file
*/
bool TsIndexer::writeSystem(const char *filename,bool append)
{
    qfprintf(index,"PSD1\n");
    qfprintf(index,"[System]\n");
    qfprintf(index,"Version=%d\n",ADM_INDEX_FILE_VERSION);
    qfprintf(index,"Type=T\n");
    qfprintf(index,"File=%s\n",filename);
    qfprintf(index,"Append=%d\n",append);
    return true;
}
/**
    \fn     writeAudio
    \brief  Write audio headers
*/
bool TsIndexer::writeAudio(void)
{
    if(!audioTracks) return false;
    qfprintf(index,"[Audio]\n");
    qfprintf(index,"Tracks=%d\n",audioTracks->size());
    for(int i=0;i<audioTracks->size();i++)
    {
        char head[30];
        tsAudioTrackInfo *t=&(*audioTracks)[i];
        sprintf(head,"Track%1d",i);
        qfprintf(index,"%s.pid=%x\n",head,t->esId);
        qfprintf(index,"%s.codec=%d\n",head,t->wav.encoding);
        qfprintf(index,"%s.fq=%d\n",head,t->wav.frequency);
        qfprintf(index,"%s.chan=%d\n",head,t->wav.channels);
        qfprintf(index,"%s.br=%d\n",head,t->wav.byterate);
    }
    return true;
}
/**
    \fn decodeVc1Seq
    \brief http://wiki.multimedia.cx/index.php?title=VC-1#Setup_Data_.2F_Sequence_Layer
#warning we should de-escape it!
        Large part of this borrowed from VLC
        Advanced/High profile only
*/
bool TsIndexer::decodeVC1Seq(int nb, uint8_t *data,TSVideo &video)
{
GetBitContext s;
int v;
int consumed;
    vc1Context.advanced=true;
    init_get_bits(&s, data, nb*8);
#define VX(a,b) v=get_bits(&s,a);printf("[VC1] %d "#b"\n",v);consumed+=a;
    VX(2,profile);
    VX(3,level);

    VX(2,chroma_format);
    VX(3,Q_frame_rate_unused);
    VX(5,Q_bit_unused);

    VX(1,postproc_flag);

    VX(12,coded_width);
    video.w=v*2+2;
    VX(12,coded_height);
    video.h=v*2+2;

    VX(1,pulldown_flag);
    VX(1,interlaced_flag);
    vc1Context.interlaced=v;
    VX(1,frame_counter_flag);

    VX(1,interpolation_flag);
    vc1Context.interpolate=v;

    VX(1,reserved_bit);
    VX(1,psf);

    VX(1,display_extension);
    if(v)
    {
         VX(14,display_extension_coded_width);
         VX(14,display_extension_coded_height);
         VX(1,aspect_ratio_flag);
     

         if(v)
         {
                VX(4,aspect_ratio);
                switch(v)
                {
                    case 15: video.ar=(get_bits(&s,8)<<16)+get_bits(&s,8);
                             break;
                    default:
                             video.ar=(VC1_ar[v][0]<<16)+(VC1_ar[v][1]<<16);
                             break;
                }
                printf("[VC1] Aspect ratio %d x %d\n",video.ar>>8,video.ar&0xff);
         }
    }
    VX(1,frame_rate);
    if(v)
    {
            VX(1,frame_rate32_flag);
            if(v)
            {
                VX(16,frame_rate32);
                float f=v;
                f=(f+1)/32;
                f*=1000;
                video.fps=(uint32_t)f;
            }else
            {
                float den,num;
                VX(8,frame_rate_num)
                switch( v )
                    {
                    case 1: num = 24000; break;
                    case 2: num = 25000; break;
                    case 3: num = 30000; break;
                    case 4: num = 50000; break;
                    case 5: num = 60000; break;
                    case 6: num = 48000; break;
                    case 7: num = 72000; break;
                    }
                VX(4,frame_rate_den)
                switch( v )
                    {
                    default:
                    case 1: den = 1000; break;
                    case 2: den = 1001; break;
                    }

                float f=num*1000;
                f/=den;
                video.fps=(uint32_t)f;
            }
    }else
    {
        video.fps=25000;
    }
    if(consumed>nb*8) 
    {
        ADM_error("Stored %d bits, consumed %d\n",nb*8,consumed);
        ADM_assert(0);
    }
    return true;

}
/**
    \fn decodeVC1Pic
    \brief Decode info from VC1 picture
    Borrowed a lot from VLC also

*/
bool TsIndexer::decodeVC1Pic(int nb, uint8_t *dataBuffer,uint32_t &frameType,uint32_t &frameStructure)
{
    GetBitContext s;
    init_get_bits(&s, dataBuffer, nb*8);
    frameStructure=3;
    bool field=false;
    if(vc1Context.interlaced)
    {
        if(get_bits(&s,1))
        {
            if(get_bits(&s,1))
               field=true;
        }
    }
    if(field)
    {
            int fieldType=get_bits(&s,3);
            frameStructure=1; // Top
            switch(fieldType)
            {
                case 0: /* II */
                case 1: /* IP */
                case 2: /* PI */
                    frameType=1;
                    break;
                case 3: /* PP */
                    frameType=2;
                    break;
                case 4: /* BB */
                case 5: /* BBi */
                case 6: /* BiB */
                case 7: /* BiBi */
                    frameType=3;
                    break;

            }
    }else
    {
                frameStructure=3; // frame
                if( !get_bits(&s,1))
                    frameType=2;
                else if( !get_bits(&s,1))
                    frameType=3;
                else if( !get_bits(&s,1))
                    frameType=1;
                else if( !get_bits(&s,1))
                    frameType=3;
                else
                    frameType=2;

    }

    return true;
}
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/

//

//EOF
