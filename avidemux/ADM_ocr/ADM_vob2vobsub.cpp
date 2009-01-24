/***************************************************************************
                         Vob2vobsub
                      -------------------
          Convert a set of vobs to a vobsub file
          * missing : 
                size 720*576 
                palette
                languages
                Multiple sub           
                      
     IFO reading from mplayer, everything else from me
                      
    copyright            : (C) 2005 by mean
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
#include "config.h"

#include "ADM_default.h"
#include <math.h>

#include <time.h>
#include <sys/time.h>

#include "DIA_coreToolkit.h"


#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"

#include "DIA_fileSel.h"
#include "ADM_inputs/ADM_mpegdemuxer/dmx_demuxer.h"
#include "ADM_inputs/ADM_mpegdemuxer/dmx_demuxerPS.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_working.h"

#include "ADM_video/ADM_vidMisc.h"

#define ADM_VOBSUB_NO_PADDING
static int vobsub_parse_ifo(const char *const name, 
                     uint32_t *palette, 
                     uint32_t *width, uint32_t *height, 
		             char *language);
static uint8_t dumpHeader(FILE *fd,int index,uint32_t w,uint32_t h,uint32_t *palette);
static void padd(int nb,FILE *fd);

static uint8_t  *padder;

#define PADDER_SIZE 0x800

#define MAX_LINE        5000
#define MAX_BUFFER      (1*1024)
#define MAX_LANGUAGE    10

#define MIN_WRAP_VALUE (90*1000*60*1) // 1 mn

typedef struct oneLine
{
    uint64_t pts;
    uint32_t start;
}oneLine;

class OneTrack
{
protected:         
            char        language[2];   
            uint8_t     *base;
            uint32_t    limit;
            uint32_t    index;
            oneLine     *lines;
            uint32_t    nbLines;
            int         runCode;
            uint64_t    currentPTS;
            int         firstOne;
            
            uint8_t     addData(uint8_t *data,uint32_t len);
            uint8_t     addLine(uint64_t pts,uint32_t start);
            uint8_t     grow(void);
public:    
            uint8_t     run(uint16_t twofirst,uint8_t *data,uint32_t size,uint32_t usableSize,uint64_t pts);
            uint8_t     setLang(char *lang);
            uint8_t     dump(uint32_t index,FILE *fdIdx, FILE *fdSub,uint32_t *out);
                        OneTrack(void);
                        ~OneTrack();
};

OneTrack::OneTrack(void)
{
    
    base=new uint8_t[  MAX_BUFFER];
    limit= MAX_BUFFER;
    index=0;
    lines=new oneLine[MAX_LINE];
    nbLines=0;  
    runCode=0;
    
}
uint8_t OneTrack::setLang(char *lang)
{
    language[0]=lang[0];
    language[1]=lang[1];
    return 1;
}
OneTrack::~OneTrack()
{
    delete [] base;
    delete [] lines;
}
 uint8_t     OneTrack::addData(uint8_t *data,uint32_t len)
{
    while(len+index>limit) grow();
    
    memcpy(base+index,data,len);
    index+=len;
    return 1;   
}
uint8_t     OneTrack::grow(void)
{
    uint8_t *nw;
    nw=new uint8_t[2*limit];
    memcpy(nw,base,limit);
    delete [] base;
    base=nw;
    limit*=2;
    return 1;   
    
}
 uint8_t     OneTrack::addLine(uint64_t pts,uint32_t start)
{
    if(nbLines>=MAX_LINE) return 0;
    lines[nbLines].pts=pts;
    lines[nbLines].start=start;
        
    nbLines++;
    return 1;   
}
uint8_t     OneTrack::dump(uint32_t number,FILE *fdIdx, FILE *fdSub,uint32_t *out)
{
    uint16_t hh,mm,ss,ms;
    uint32_t timestamp;
    uint32_t original,position;
    original=ftello(fdSub); // Current position, we start from here
      
    if(!index) return 1;
  
    fwrite(base,index,1,fdSub); // Append our datas
    
    
    fprintf(fdIdx,"# English\n");
    fprintf(fdIdx,"id: %c%c, index: %d\n",language[0],language[1],number);
    fprintf(fdIdx,"# Decomment next line to activate alternative name in DirectVobSub / Windows Media Player 6.x\n");
    fprintf(fdIdx,"# alt: English\n");
    fprintf(fdIdx,"# Vob/Cell ID: 1, 1 (PTS: 0)\n");
    (*out)++;
    for(int i=0;i<nbLines;i++) // We shift PTS & position by 1 to workaround the display bug
    {   
        if(lines[i].pts!=ADM_NO_PTS)
        {     
            timestamp=(uint32_t)floor(lines[i].pts/90.);
            ms2time(timestamp,&hh,&mm,&ss,&ms);
            position=lines[i].start;
            //printf("Stream :%d position :%x offset:%x total:%x\n",i,position,original,original+position);
            position+=original;          
            fprintf(fdIdx,"timestamp: %02d:%02d:%02d:%03d, filepos: %08x\n",hh,mm,ss,ms,position); 
        }
        else
        {
                printf("Sub %d, skipped line at %d\n",number,i);
        }
    }
    return 1;
}
uint8_t     OneTrack::run(uint16_t twofirst,uint8_t *data,uint32_t size,uint32_t usableSize,uint64_t pts)
{
uint32_t padding;
    
    if(!runCode) // new line
    {
        currentPTS=pts;
        runCode=twofirst-usableSize;
        addLine(pts,index);
#if 0
        {
        uint16_t hh,mm,ss,ms;
        uint32_t timestamp;

              timestamp=pts/90;
              ms2time(timestamp,&hh,&mm,&ss,&ms);
              printf("Line : %03u  at %02d:%02d:%02d \n",nbLines,hh,mm,ss);
        }
#endif
        addData(data,size);
        if(runCode<0) runCode=0;
        return 1; 
    }
    if(currentPTS==ADM_NO_PTS && pts!=ADM_NO_PTS)
        currentPTS=lines[nbLines-1].pts=pts;
    runCode-=usableSize;
    if(runCode<0)
    {
         printf("Overrun %d\n",runCode);
         runCode=0;
    }
    addData(data,size);
    return 1;
    
} 
/*
    nameVob     : path + name of the 1st .vob file
    nameVobSub  : path+name of the .idx file to write
    nameIfo     : path+name of the ifo file


*/
//**********************************************
uint8_t ADM_vob2vobsub(char *nameVob, char *nameVobSub, char *nameIfo)
{
   dmx_demuxerPS *demuxer=NULL;
   DIA_working *working=NULL;
   MPEG_TRACK track;
   FILE *indexFile=NULL;
   FILE *indexSub=NULL;
   uint32_t palette[16],width,height;
   uint64_t abs,rel,size,pts;
   int blockSize;
   uint8_t *data,stream;
   char *subname;
   double percent;
   uint32_t packetLen,usedLen,read;
   OneTrack allIndex[MAX_LANGUAGE];
   char language[MAX_LANGUAGE*4];
#ifdef  TEST_V2V  
   nameIfo="d:/Crime/VTS_01_0.IFO";
   nameVobSub="toto.idx";
#endif
   
        printf("v2v: Ifo:%s Vob:%s Vsub:%s\n",nameIfo,nameVob,nameVobSub);

   memset(language,0,sizeof(language));
   memset(palette,0,sizeof(uint32_t)*16);
   if(!vobsub_parse_ifo(nameIfo,palette,&width,&height,language))
   {
     GUI_Error_HIG(QT_TR_NOOP("Ifo error"),QT_TR_NOOP("Error reading ifo file, aborting."));   
        return 0;
   } 
   printf("Ifo: %d x %d\n",width,height);                 
   
   indexFile=fopen(nameVobSub,"wt");
   if(!indexFile)
   {
     GUI_Error_HIG(QT_TR_NOOP("Cannot write .idx"),NULL);              
        return 0;
    }
   subname=ADM_strdup(nameVobSub);
   size=strlen(subname);
   subname[size-3]='s';
   subname[size-2]='u';
   subname[size-1]='b';
   indexSub=fopen(subname,"wb");
   ADM_dealloc(subname);
    if(!indexSub)
    {
        fclose(indexFile);
        GUI_Error_HIG(QT_TR_NOOP("Cannot write .sub"),NULL);
        return 0;
    }
   for(int i=0;i<MAX_LANGUAGE;i++)
   {
         allIndex[i].setLang(language+i*3);  
   }
   track.pes=0x20;
   track.pid=track.pes;
   demuxer=new  dmx_demuxerPS(1,&track,1);
   if(!demuxer->open(nameVob))
   {
     GUI_Error_HIG(QT_TR_NOOP("Problem opening the mpeg files"),NULL);
        delete demuxer;
        fclose(indexFile);
        fclose(indexSub);
        return 0;   
   }
   
   
   size=demuxer->getSize();
   
    int display=0;
    
   dumpHeader(indexFile,0,width,height,palette);
   working=new DIA_working(QT_TR_NOOP("Generating VobSub file"));
   
   //*** Main Loop ***
   uint32_t startPts=0,lastPts=0;
   uint16_t hh,mm,ss,ms;
   uint32_t timestamp;
   while(1)
   {
       if(!demuxer->forceRefill(&stream)) goto _abt;
       demuxer->getPos(&abs,&rel);
       display++;
       if(display>20)
       {
        working->update(abs>>10,size>>10);
        display=0;
       }
#ifdef TEST_V2V       
       //if(abs>200*1024*1024) break;
#endif       
       if(stream>=0x20 && stream<0x20+MAX_LANGUAGE)
       {
            demuxer->getPacketInfo(&data,&packetLen,&usedLen,&pts);
            if(pts!=ADM_NO_PTS)
            {
                        // Wrap around ?
                        if(lastPts)
                        {
                                if(pts<lastPts)
                                {
                                        if(lastPts-pts>MIN_WRAP_VALUE)
                                        {
                                                
                                                printf("Wrapping at %u ",lastPts);
                                                startPts+=lastPts;
                                                timestamp=startPts/90;
                                                ms2time(timestamp,&hh,&mm,&ss,&ms);
                                                printf("%02d:%02d:%02d \n",hh,mm,ss);
                                        }
                                }
                        }

                        lastPts=pts;
                        pts+=startPts;
            }
#if 0
            if(pts!=ADM_NO_PTS)
            {
              timestamp=pts/90;
              ms2time(timestamp,&hh,&mm,&ss,&ms);
              printf("%02d:%02d:%02d \n",hh,mm,ss);
            }
#endif
            blockSize=demuxer->read16i();
            allIndex[stream-0x20].run(blockSize,data,packetLen,usedLen, pts)  ;
       }
    }
  //*** /Main Loop ***     
  _abt:
  uint32_t out=0,padding;
  for(int i=0;i<MAX_LANGUAGE;i++)
  {
      allIndex[i].dump(i,indexFile, indexSub,&out);  
      padding=ftello(indexSub);
      printf("Padding : was %x ",padding);
      padding=padding+PADDER_SIZE-1;
      padding&=0xffffffff^(PADDER_SIZE-1);  
      fseeko(indexSub,padding,SEEK_SET);  
      printf(" -> %x\n",padding);
  }
  
  
  printf("scan done %"LLU"/%"LLU"\n",abs,size); 
  delete working;
  delete demuxer;     
  fclose(indexFile);
  fclose(indexSub);
  return 1;
    
}
/*
Borrowed from Mplayer
*/

extern void mixDump(uint8_t *p,uint32_t l);
int vobsub_parse_ifo(const char *const name, 
                     uint32_t *palette, 
                     uint32_t *width, uint32_t *height, 
		             char *language)
{
    int ret = 0;
    FILE *fd = fopen(name, "rb");
    if(!fd) return 0;
    
	// parse IFO header
	unsigned char block[0x800];
	const char *const ifo_magic = "DVDVIDEO-VTS";
	if (fread(block, sizeof(block), 1, fd) != 1) 
	{
    	fclose(fd);
    	return 0;
    }
    if(memcmp(block,ifo_magic,strlen(ifo_magic+1)))
    {
        printf("Bad ifo magic\n");
        fclose(fd);
    	return 0;
    }
	    unsigned long pgci_sector = block[0xcc] << 24 | block[0xcd] << 16
		| block[0xce] << 8 | block[0xcf];
	    int standard = (block[0x200] & 0x30) >> 4;
	    int resolution = (block[0x201] & 0x0c) >> 2;
	    *height = standard ? 576 : 480;
	    *width = 0;
	    switch (resolution) {
	    case 0x0:
		*width = 720;
		break;
	    case 0x1:
		*width = 704;
		break;
	    case 0x2:
		*width = 352;
		break;
	    case 0x3:
		*width = 352;
		*height /= 2;
		break;
	    default:
		printf("Vobsub: Unknown resolution %d \n", resolution);
	    }
	    unsigned  int tmp=0;
	    for(int lg=0;lg<MAX_LANGUAGE;lg++)
	    {
		    tmp= 0x256 + lg * 6 + 2;
		    //printf("Lang :%d %c%c\n",lg,block[tmp],block[tmp+1]);//tmp[0],tmp[1]);
		    language[lg*3+0]=block[tmp];
		    language[lg*3+1]=block[tmp+1];
            language[lg*3+2]=0;
		}
	    if (fseeko(fd, pgci_sector * sizeof(block), SEEK_SET)
		|| fread(block, sizeof(block), 1, fd) != 1)
		 {}
		 else
		 {
	    
		        unsigned long idx,r,g,b,y,u,v;
		        unsigned long pgc_offset = block[0xc] << 24 | block[0xd] << 16
		                 | block[0xe] << 8 | block[0xf];
		        // 8+32*4+8*2+4+dvd_time_t+user_ops_t
		        // 9c+dvd_time+user_ops_t=9c+4+8=0xA8  
		        //mixDump(block,0x800);      
		        for (idx = 0; idx < 16; ++idx) 
		        {
  		        
		            unsigned char *p = block + pgc_offset + 0xa4 + 4 * idx; 
#if 1
                        r=p[1];g=p[2];b=p[3];
                       // palette[idx] = r << 16 | g << 8 | b;
                        palette[idx] = r << 16 | r << 8 | r;
#else		            
		                y=p[1];u=p[2];v=p[3];
		                float rr,bb,gg;
	                    float yy=y,uu=u-128,vv=v-128;

	                    rr=	yy+			 	1.402*vv;
	                    gg= yy+ 	-0.344*uu+  	-0.714*vv;
	                    bb=	yy+ 	1.772*uu 	 		;

	#define CLIP(x) if(x>255) x=255; else if (x<0) x=0;x=x+0.49;
		                CLIP(rr);CLIP(gg);CLIP(bb);
		                r=(unsigned long int)rr;
		                g=(unsigned long int)gg;
		                b=(unsigned long int)bb;
		                //palette[idx] = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
		                palette[idx] = r << 16 | g << 8 | b;
#endif		                
		        }
		        ret= 1;
	     }
_ifo_abt:		
	    fclose(fd);
    
    return ret;
}

/**********************************************************************


***********************************************************************/
static uint8_t dumpHeader(FILE *fd,int index,uint32_t w,uint32_t h,uint32_t *palette)
{
    fprintf(fd,"# VobSub index file, v7 (do not modify this line!)\n");
    fprintf(fd,"# \n");
    fprintf(fd,"# To repair desyncronization, you can insert gaps this way:\n");
    fprintf(fd,"# (it usually happens after vob id changes)\n");
    fprintf(fd,"# \n");
    fprintf(fd,"#	 delay: [sign]hh:mm:ss:ms\n");
    fprintf(fd,"# \n");
    fprintf(fd,"# Where:\n");
    fprintf(fd,"#	 [sign]: +, - (optional)\n");
    fprintf(fd,"#	 hh: hours (0 <= hh)\n");
    fprintf(fd,"#	 mm/ss: minutes/seconds (0 <= mm/ss <= 59)\n");
    fprintf(fd,"#	 ms: milliseconds (0 <= ms <= 999)\n");
    fprintf(fd,"# \n");
    fprintf(fd,"#	 Note: You can't position a sub before the previous with a negative value.\n");
    fprintf(fd,"# \n");
    fprintf(fd,"# You can also modify timestamps or delete a few subs you don't like.\n");
    fprintf(fd,"# Just make sure they stay in increasing order.\n");
    fprintf(fd,"\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Settings\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Original frame size\n");
    fprintf(fd,"size: %dx%d\n",w,h);
    fprintf(fd,"\n");
    fprintf(fd,"# Origin, relative to the upper-left corner, can be overloaded by aligment\n");
    fprintf(fd,"org: 0, 0\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Image scaling (hor,ver), origin is at the upper-left corner or at the alignment coord (x, y)\n");
    fprintf(fd,"scale: 100%%, 100%%\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Alpha blending\n");
    fprintf(fd,"alpha: 100%%\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Smoothing for very blocky images (use OLD for no filtering)\n");
    fprintf(fd,"smooth: OFF\n");
    fprintf(fd,"\n");
    fprintf(fd,"# In millisecs\n");
    fprintf(fd,"fadein/out: 50, 50\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Force subtitle placement relative to (org.x, org.y)\n");
    fprintf(fd,"align: OFF at LEFT TOP\n");
    fprintf(fd,"\n");
    fprintf(fd,"# For correcting non-progressive desync. (in millisecs or hh:mm:ss:ms)\n");
    fprintf(fd,"# Note: Not effective in DirectVobSub, use \"delay: ... \" instead.\n");
    fprintf(fd,"time offset: 0\n");
    fprintf(fd,"\n");
    fprintf(fd,"# ON: displays only forced subtitles, OFF: shows everything\n");
    fprintf(fd,"forced subs: OFF\n");
    fprintf(fd,"\n");
    fprintf(fd,"# The original palette of the DVD\n");
    fprintf(fd,"palette:");
    for(int k=0;k<16;k++) // 131313, efefef, efefef, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff, ffffff\n");
    {
        if(k) fprintf(fd,", ");
        fprintf(fd,"%06x",palette[k]);
    }
    fprintf(fd,"\n");
    fprintf(fd,"# Custom colors (transp idxs and the four colors)\n");
    fprintf(fd,"custom colors: OFF, tridx: 0000, colors: 000008, 300030, 000030, 9332c8\n");
    fprintf(fd,"\n");
    fprintf(fd,"# Language index in use\n");
    fprintf(fd,"langidx: %d\n",0);
    fprintf(fd,"\n");
    return 1;  
}
