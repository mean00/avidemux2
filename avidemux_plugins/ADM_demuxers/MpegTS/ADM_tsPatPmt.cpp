/** *************************************************************************
    \file ADM_tsPatPmt.cpp
    \brief Analyze pat & pmt
    copyright            : (C) 2007 by mean
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

#include "ADM_default.h"
#include "ADM_ts.h"
#include "ADM_demuxerInternal.h"
#include "ADM_tsPatPmt.h"

typedef vector <ADM_TS_TRACK> listOfTsTracks;

/**
    \fn scanPmt
*/
static bool  TS_scanPmt(tsPacket *t,uint32_t pid,listOfTsTracks *list);
static bool  decodeProgrameDescriptor(uint8_t *r, uint32_t maxlen,std::string &language);
static bool  decodeRegistration(int size,uint8_t *data);
static ADM_TS_TRACK_TYPE EsType(uint32_t type,const char **str);
/**
    \class TrackTypeDescriptor
*/
class TrackTypeDescriptor
{
public:
    int                 type;
    ADM_TS_TRACK_TYPE   trackType;
    const char          *desc;
    static TrackTypeDescriptor *find(int t);
};
TrackTypeDescriptor TrackTypes[]=
{
    {0x002,ADM_TS_MPEG2,     "Mpeg2 Video"},
    {0x003,ADM_TS_MPEG_AUDIO,"Mpeg1 Audio"},
    {0x004,ADM_TS_MPEG_AUDIO,"Mpeg2 Audio"},
    {0x005,ADM_TS_UNKNOWN,   "Registration"},
    {0x01b,ADM_TS_H264,      "H264 Video"},
    {0x081,ADM_TS_AC3,       "AC3 Audio"},
    {0x0ea,ADM_TS_VC1,       "VC1 Video"},
    {0x006,ADM_TS_UNKNOWN,   "Private Stream"},
    {0xfff,ADM_TS_UNKNOWN,   "Unknown"}   // Last one must be "unknown!"
};



/**
    \fn find
*/
TrackTypeDescriptor *TrackTypeDescriptor::find(int t)
{
    int size=sizeof(TrackTypes)/sizeof(TrackTypeDescriptor);
    for(int i=0;i<size;i++)
    {
        TrackTypeDescriptor *tp=TrackTypes+i;
        if(tp->type==t) return tp;
    }
    return TrackTypes+size-1;
}
/**
    \fn scanForPrograms
    \brief Lookup PAT & PMT to get tracks
*/
bool TS_scanForPrograms(const char *file,uint32_t *nbTracks, ADM_TS_TRACK **outTracks)
{
    uint32_t len;
    TS_PSIpacketInfo psi;
    listOfTsTracks   list;
    vector <uint32_t>listOfPmt;
    ADM_TS_TRACK *tracks=NULL;
    *outTracks=NULL;
    *nbTracks=0;
    uint32_t nb=0;

    tsPacket *t=new tsPacket();
    t->open(file,FP_PROBE);
    // 1 search the pat...
    if(t->getNextPSI(0,&psi)==true)
    {
        uint8_t *r=psi.payload;
        len=psi.payloadSize;
        while(len>=4)
        {
            uint32_t prg=((0x1F&r[0])<<8)+r[1];
            uint32_t pid=((0x1F&r[2])<<8)+r[3];
            r+=4;
            len-=4;
            printf("[TsDemuxer] Pat : Prg:%d Pid: 0x%04x\n",prg,pid);
            if(prg) // if prg==0, it is network Pid, dont need it
                listOfPmt.push_back(pid);
        }
        int size=listOfPmt.size();
        if(size)
        {
            for(int i=0;i<size;i++) // First PMT is PCR lock ?
            {
                printf("<<< PMT : %d/%d>>>\n",i,size);
                uint32_t pid=listOfPmt[i];
                t->setPos(0); // Make sure we can have the very first one
                TS_scanPmt(t,pid,&list);
            }

        }
    }
    // List now contains a list of elementary tracks

    // TODO: Look deeper to see if there is actual content
    // Some channels advertsise languages that they may have
    // but dont actually exists in the file
    // Allocate output files, max size=list.size
    
    printf("[TS Demuxer] Found %u interesting tracks\n",(unsigned int)list.size());
    
    // Search the video track...
    bool result=false;
    int videoIndex=-1;
    for(int i=0;i<list.size();i++)
    {
        ADM_TS_TRACK_TYPE type=list[i].trackType;
        if(type==ADM_TS_MPEG2 || type==ADM_TS_H264 || type==ADM_TS_VC1)
        {
            videoIndex=i;
            break;
        }
    }
    if(videoIndex==-1)
    {
        printf("[Ts Demuxer] Cannot find a video track\n");
        goto _failTs;
    }
    {
        //
 
        // After here we cannot fail (normally...)
        tracks=new ADM_TS_TRACK[list.size()];
        *outTracks=tracks;
        // Copy video track
        tracks[0]=list[videoIndex];
        // and remove it from the list
        list.erase(list.begin()+videoIndex);
        nb++;
        // Also add audio tracks we know of
        for(int i=0;i<list.size();i++)
        {
            ADM_TS_TRACK_TYPE type=list[i].trackType;
            if(type==ADM_TS_MPEG_AUDIO || type==ADM_TS_AC3 || type==ADM_TS_AAC_ADTS ||type==ADM_TS_AAC_LATM
                        || type==ADM_TS_EAC3)
            {
                TSpacketInfo pkt;
                t->setPos(0);
                if(true==t->getNextPacket_NoHeader(list[i].trackPid,&pkt,false))
                    tracks[nb++]=list[i];
                else        
                    printf("[TS Demuxer] Track %i pid 0x%x does not seem to be there\n",i,list[i].trackPid);
            }
        }
        *nbTracks=nb;
        result=true;
    }
_failTs:
    delete t;
    // Delete the list
    if(list.size())
        list.erase(list.begin(),list.end()-1);
    if(result==true)
    {
        printf("[T Demuxer] Kept %d tracks\n",*nbTracks);
    }
    printf("[TS Demuxer] Probed...\n");
    
    return result;
}
/**
    \fn TS_scanPmt
    \brief Lookup one PMT and returns content
*/
bool TS_scanPmt(tsPacket *t,uint32_t pid,listOfTsTracks *list)
{
    ADM_TS_TRACK trk;
    trk.language=ADM_UNKNOWN_LANGUAGE;
    uint32_t len;
    TS_PSIpacketInfo psi;
    uint8_t *r=psi.payload;
    trk.trackType=ADM_TS_UNKNOWN;
    trk.trackPid=pid;
    printf("[TsDemuxer] Looking for PMT : 0x%x\n",pid);
    if(t->getNextPSI(pid,&psi)==true)
     {
        len=psi.payloadSize;
        // We should be protected by CRC here
        int packLen=len;
        printf("[TsDemuxer] PCR 0x%x, len=%d\n",(r[0]<<8)+r[1],packLen);
        r+=2;  
        int programInfoLength=(r[0]<<8)+r[1];
        programInfoLength&=0x3ff;
        r+=2;
        packLen-=4;
        // Program Descriptor
        printf("[PMT]--Decoding Program info--\n");
        if(programInfoLength && programInfoLength<=packLen)
        {
            decodeProgrameDescriptor(r, programInfoLength,trk.language);
            packLen-=programInfoLength;
            r+=programInfoLength;
        }
        printf("[PMT]            Left : %d bytes\n",packLen);
        printf("[PMT]--Decoding ES Descriptor--\n");
        // Es Type Descriptor
        while(packLen>4)
        {
            int type=r[0];
            int pid=(r[1]<<8)+r[2];
            int size=(r[3]<<8)+r[4];

            size&=0xfff;
            pid&=0x1fff;
            uint8_t *base=r+5;
            r+=size+5;
            packLen-=5+size;
            printf("[PMT]          Type=0x%x pid=%x size=%d\n",type,pid,size);
            const char *str;
            
            //
            // extract tags to get additional infos/properties
            //
            uint8_t *head=base;
            uint8_t *tail=r;
            while(head<tail)
            {
               uint8_t tag=head[0];
               uint8_t tag_len=head[1];
               printf("[PMT]     Tag 0x%x , len %d, ",tag,tag_len);
               for(int i=0;i<tag_len;i++) printf(" %02x",head[2+i]);
               printf("\n");
               switch(tag)
               {
               case 0xa:  // dvb language
               {
                   if(tag_len<2) break; // too short
                   char lan[16];
                   for(int i=0;i<tag_len;i++)
                   {
                       lan[i]=head[2+i];
                   }
                   lan[tag_len]=0;
                   trk.language=std::string(lan);
                   break;                        
               }
               case 0x7A: if(type==6) type=0x84;break; 
               case 0x6A: if(type==6) type=0x81;break; // AC3
                   default:break;
               }
               head+=2+tag_len;
            }
            
            if(type==0xea)
            {


            }
            ADM_TS_TRACK_TYPE trackType=EsType(type,&str);;
            if(trackType!=ADM_TS_UNKNOWN) 
            {
                    ADM_TS_TRACK trk2;
                    trk2.trackPid=pid;
                    trk2.trackType=trackType;
                    trk2.language=trk.language;
                    printf("[PMT]  Adding pid 0x%x (%d) , type %s, language=%s\n",pid,pid,str,trk2.language.c_str());
                    list->push_back(trk2);
            }else
                printf("[PMT]              -> %s\n",str);
            
           
        }
        
        printf("[PMT] Left :%d bytes\n",packLen);
        if(trk.trackType!=ADM_TS_UNKNOWN) list->push_back(trk);
        return true;
       }
    return false;
}
/**
    \fn EsType
    \brief returns type and string attached to a ES type
*/
ADM_TS_TRACK_TYPE EsType(uint32_t type,const char **str)
{
    *str="????";
    switch(type)
    {
                case 0x01:case 2: *str= "Mpeg Video";return ADM_TS_MPEG2;break;
                case 0x03:case 4: *str= "Mpeg Audio";return ADM_TS_MPEG_AUDIO;break;
                case 0xF:      *str= "Mpeg AAC ADTS";return ADM_TS_AAC_ADTS;break;
                case 0x11:     *str= "Mpeg AAC LATM";return ADM_TS_AAC_LATM;break;
                case 0x1B:     *str= "H264 Video";return ADM_TS_H264;break;
                case 0x81:     *str= "AC3 (Not sure)";return ADM_TS_AC3;break;
                //case 0x82:     *str= "DTS (Not sure)";return ADM_TS_DTS;break;
                case 0x83:     *str= "TrueHD AC3  (BluRay)";return ADM_TS_AC3;break;
                case 0x84:     *str= "E-AC3 (Not sure)";return ADM_TS_EAC3;break;
                case 0x87:     *str= "E-AC3 (Not sure)";return ADM_TS_EAC3;break;
                case 0xea:     *str= "VC1 (Not sure)";return ADM_TS_VC1;break;
                case 0x90:     *str= "Presentation graphics (BluRay)";return ADM_TS_UNKNOWN;break;
                
                //   case 0x10:        *streamType=ADM_STREAM_MPEG4;return "MP4 Video";
                default : break;
    }
    return ADM_TS_UNKNOWN;
}

/**
    \fn decodeProgrameDescriptor
    \brief decodeProgramDescriptor

    http://www.coolstf.com/tsreader/descriptors.html

*/
bool   decodeProgrameDescriptor(uint8_t *r, uint32_t maxlen,std::string &language)
{
        printf("[PMT] Program Info Len: %d\n",maxlen); 
        int packLen=maxlen;
        packLen-=(2+4);
        while(packLen>4)
        {
                int streamType,streamPid,esInfoLength;
                streamType=r[0];
                printf("[PMT] StreamType=%d\n",streamType);
                if(streamType==5) // descriptor
                {
                    int descriptorLength=r[1];
                    if(descriptorLength+1>packLen) 
                    {
                        ADM_warning("[PMT             Registration length bigger than section\n");
                        return true;
                    }
                    printf("\t[PMT] Registration FCC %c%c%c%c \n",r[2],r[3],r[4],r[5]);
                    uint8_t *tail=r+2+descriptorLength;
                    packLen-=1+descriptorLength;
                    decodeRegistration(descriptorLength,r);
                    r=tail;
                    continue;
                }
                
                streamPid=(r[1]<<8)+r[2]&0x1fff;
                esInfoLength=((r[3]<<8)+r[4])&0xfff;
                r+=5;
                packLen-=5;
                packLen-=esInfoLength;
                TrackTypeDescriptor *td=TrackTypeDescriptor::find(streamType);
                printf("\t[PMT]           StreamType: 0x%x,<<%s>>\n",streamType,td->desc);    
                printf("\t[PMT]           Pid:        0x%x\n",streamPid);
                printf("\t[PMT]           Es Info Length: %d (0x%x)\n",esInfoLength,esInfoLength);
                if(packLen<1) 
                {
                    ADM_warning("[PMT             ES Info length bigger than section\n");
                    return true;
                }
                //trk.trackType=td->trackType;
                //trk.trackPid=streamPid;
                uint8_t *p=r,*pend=r+esInfoLength;
                r+=esInfoLength;
                while(p<pend)
                {
                    uint32_t tag,taglen;
                    tag=p[0];
                    taglen=p[1];
                    printf("\t[PMT]           Tag :%x len:%d =",tag,taglen);
                    switch(tag) //http://www.coolstf.com/tsreader/descriptors.html
                    {
                        
                        case 0x05: printf("Registration Descriptor :%c%c%c%c",p[2],p[3],p[4],p[5]);break;
                        case 0x0a: 
                        {
                            printf("Language descriptor :%c%c%c",p[2],p[3],p[4]);
                            char lang[4]={(char)p[2],(char)p[3],(char)p[4],0};
                            language=std::string(lang);
                            break;
                        }
                        case 0x11: printf("STD\n");break;
                        case 0x1e: printf("SL descriptor (H264 AAC ?)");break;
                        case 0x45: printf("VBI data\n");break;
                        case 0x52: printf("Stream identifier");break;
                        case 0x56: printf("Teletext");break;
                        case 0x59: printf("DVB subtitles");break;
                        case 0x7b: printf("DTS Descriptor");break;
                        case 0x7a: printf("EAC3 Descriptor");break;
                        case 0x6a: printf("AC3 Descriptor");break;
                        default : printf("unknown");break;
                    }
                    printf("\n");
                    p+=2+taglen;
                }
                // 
                
        }
        return true;
}
/**
    \fn decodeRegistration
    \brief Decode registration descriptor subtag as used for VC1
*/
bool decodeRegistration(int size,uint8_t *data)
{
    uint8_t *end=data+size;
    
    while(data<end)
    {
        int tag=data[0];data++;
        printf("[PMT] Registration, found tag %d\n",tag);
        switch(tag)
        {
            case 0:    break; // sub desc
            case 0xff: break;
            case 1: // profile & level desc
                            printf("[PMT] Profile/level :%d",*data++);
                            break;
            case 2: // Alignment
                            printf("[PMT] Alignement    :%d",*data++);
                            break;
            case 3: // Buffer size
                            printf("[PMT] Buffer size\n");
                            data+=3;
                            break;
            default: 
                        printf("Unknown registration tag :%d\n",tag);
                        return true;

        }
    }
    return true;
}
//EOF
