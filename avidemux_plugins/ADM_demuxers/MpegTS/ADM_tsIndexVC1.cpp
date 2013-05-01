/***************************************************************************
    \brief TS indexer, VC1 video
    \author mean fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_tsIndex.h"

static const uint32_t  VC1_ar[16][2] = {  // From VLC
                        { 0, 0}, { 1, 1}, {12,11}, {10,11}, {16,11}, {40,33},
                        {24,11}, {20,11}, {32,11}, {80,33}, {18,11}, {15,11},
                        {64,33}, {160,99},{ 0, 0}, { 0, 0}};


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

    beginConsuming=0;
    listOfUnits.clear();

    if(!videoTrac) return false;
    if(videoTrac[0].trackType!=ADM_TS_VC1)
    {
        printf("[Ts Indexer] Only VC1 video supported\n");
        return false;
    }
    video.pid=videoTrac[0].trackPid;

    memset(&data,0,sizeof(data));
    data.picStructure=pictureFrame;
    
    string indexName=string(file);
    indexName=indexName+string(".idx2");
    index=qfopen(indexName,"wt");

    if(!index)
    {
        printf("[PsIndex] Cannot create %s\n",indexName.c_str());
        return false;
    }
    writeSystem(file,false);
    pkt=new tsPacketLinearTracker(videoTrac->trackPid, audioTracks);

    FP_TYPE append=FP_APPEND;
    pkt->open(file,append);
    data.pkt=pkt;
    fullSize=pkt->getSize();
    int startCode;
    decodingImage=false;
#define likely(x) x
#define unlikely(x) x
      while(1)
      {
        startCode=pkt->findStartCode();
        if(!pkt->stillOk()) break;

          switch(startCode)
                  {
                  case 0x0f: // sequence start
                      
                          if(seq_found)
                          {
                              pkt->getInfo(&thisUnit.packetInfo);
                              thisUnit.consumedSoFar=pkt->getConsumed();
                              addUnit(data,unitTypeSps,thisUnit,4);
                              decodingImage=false;
                              break;
                          }
                          // Verify it is high/advanced profile
                          {
                          int seqSize=0;
                          tsGetBits bits(pkt);
                          if(!bits.peekBits(1)) continue; // simple/main profile

                          if(!decodeVC1Seq(bits,video)) continue;

                          seqSize=bits.getConsumed();
                          video.extraDataLength=seqSize+4+1;
                          memcpy(video.extraData+4,bits.data,seqSize);
                            // Add info so that ffmpeg is happy
                          video.extraData[0]=0;
                          video.extraData[1]=0;
                          video.extraData[2]=1;
                          video.extraData[3]=0xf;
                          video.extraData[seqSize+4+0]=0;
                          seq_found=1;
                          // Hi Profile
                          printf("[VC1] Found seq start with %d x %d video\n",(int)video.w,(int)video.h);
                          printf("[VC1] FPS : %d\n",(int)video.fps);
                          printf("[VC1] sequence header is %d bytes\n",(int)seqSize);
                          writeVideo(&video,ADM_TS_VC1);
                          writeAudio();
                          qfprintf(index,"[Data]");
                          
                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          addUnit(data,unitTypeSps,thisUnit,seqSize+4);
                          decodingImage=false;
                          
                          continue;
                          }
                          break;
                    case 0x0D: //  Picture start
                        { 
                          int type;
                          uint8_t buffer[4];
                          uint32_t fType,sType;
                          if(!seq_found)
                          { 
                                  continue;
                                  printf("[TsIndexer]No sequence start yet, skipping..\n");
                          }      
                          pkt->getInfo(&thisUnit.packetInfo);
                          thisUnit.consumedSoFar=pkt->getConsumed();
                          
                          tsGetBits bits(pkt);
                          if(!decodeVC1Pic(bits,fType,sType)) continue;
                          thisUnit.imageType=fType;
                          updatePicStructure(video,sType);
                          addUnit(data,unitTypePic,thisUnit,4);
                          decodingImage=true;
                          data.nbPics++;
                        }
                          break;
                  default:
                    break;
                  }
      }
    
        printf("\n");
//        Mark(&data,&info,2);
        qfprintf(index,"\n[End]\n");
        qfprintf(index,"\n# Found %"PRIu32" images \n",data.nbPics); // Size
        qfprintf(index,"# Found %"PRIu32" frame pictures\n",video.frameCount); // Size
        qfprintf(index,"# Found %"PRIu32" field pictures\n",video.fieldCount); // Size
        qfclose(index);
        index=NULL;
        audioTracks=NULL;
        delete pkt;
        pkt=NULL;
        return 1; 
}
/**
    \fn decodeVc1Seq
    \brief http://wiki.multimedia.cx/index.php?title=VC-1#Setup_Data_.2F_Sequence_Layer
#warning we should de-escape it!
        Large part of this borrowed from VLC
        Advanced/High profile only
*/
bool TsIndexer::decodeVC1Seq(tsGetBits &bits,TSVideo &video)
{

int v;
int consumed;
    vc1Context.advanced=true;

#define VX(a,b) v=bits.getBits(a);printf("[VC1] %d "#b"\n",v);consumed+=a;
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
                    case 15: video.ar=(bits.getBits(8)<<16)+bits.getBits(8);
                             break;
                    default:
                             video.ar=(VC1_ar[v][0]<<16)+(VC1_ar[v][1]<<16);
                             break;
                }
                printf("[VC1] Aspect ratio %d x %d\n",video.ar>>8,video.ar&0xff);
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
            //
            VX(1,color_flag);
            if(v){
                    VX(8,color_prim);
                    VX(8,transfer_char);
                    VX(8,matrix_coef);
                }
    }
    VX(1,hrd_param_flag);
    int leaky=0;
    if(v)
    {
        VX(5,hrd_num_leaky_buckets);
        leaky=v;
        VX(4,bitrate_exponent);
        VX(4,buffer_size_exponent);
        for(int i = 0; i < leaky; i++) 
        {
                bits.getBits(16);
                bits.getBits(16);
        }
    }
    // Now we need an entry point
    bits.flush();
    uint8_t a[4];
    uint8_t entryPoint[4]={0,0,1,0x0E};
    for(int i=0;i<4;i++) a[i]=bits.getBits(8);
    for(int i=0;i<4;i++) printf("%02x ",a[i]);
    printf(" as marker\n");
    if(memcmp(a,entryPoint,4))
    {
        ADM_warning("Bad entry point");
        return false;
    }
    VX(6,ep_flags);
    VX(1,extended_mv);
    int extended_mv=v;
    VX(6,ep_flags2);

    for(int i=0;i<leaky;i++)
            bits.getBits(8);
    VX(1,ep_coded_dimension);
    if(v)
    {
         VX(12,ep_coded_width);
         VX(12,ep_coded_height);
    }
    if(extended_mv) VX(1,dmv);
    VX(1,range_mappy_flags);
    if(v) VX(3,mappy_flags);
    VX(1,range_mappuv_flags);
    if(v) VX(3,mappuv_flags);
    return true;

}
/**
    \fn decodeVC1Pic
    \brief Decode info from VC1 picture
    Borrowed a lot from VLC also

*/
bool TsIndexer::decodeVC1Pic(tsGetBits &bits,uint32_t &frameType,uint32_t &frameStructure)
{
    frameStructure=3;
    bool field=false;
    if(vc1Context.interlaced)
    {
        if(bits.getBits(1))
        {
            if(bits.getBits(1))
               field=true;
        }
    }
    if(field)
    {
            int fieldType=bits.getBits(3);
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
                if( !bits.getBits(1))
                    frameType=2;
                else if( !bits.getBits(1))
                    frameType=3;
                else if( !bits.getBits(1))
                    frameType=1;
                else if( !bits.getBits(1))
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
