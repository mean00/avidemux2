//#include "unistd.h"
#include "ADM_toolkitGtk.h"


#include "ADM_toolkit/toolkit.hxx"

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_video/ADM_genvideo.hxx"
#include "ADM_filter/video_filters.h"

#include "ADM_toolkit/toolkit.hxx"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_video/ADM_genvideo.hxx"
#include "ADM_filter/video_filters.h"


#include "ADM_toolkit/filesel.h"

#include "ADM_colorspace/colorspace.h"

#include "ADM_video/ADM_vobsubinfo.h"

#include "ADM_video/ADM_vidVobSub.h"
#include "ADM_leftturn.h"
#include "DIA_enter.h"
#include "adm_sup.h"

static uint32_t pal[16]={ 0x00000, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                          0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                          0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                          0xFFFFFF, 0x303030, 0xFFFFFF, 0xFFFFFF
};

SUPIndexArray::SUPIndexArray(uint32_t nb)
{
    indexCeil=nb;
    nbIndex=0;
    index=new SUPIndex[nb];
}
SUPIndexArray::~SUPIndexArray()
{
  if(index) delete [] index;
  index=NULL; 
}

uint32_t SUPIndexArray::insert(uint32_t pos, uint32_t size, uint32_t pts)
{
  if(nbIndex==indexCeil-1) // grow
  {
      SUPIndex *sup=new SUPIndex[indexCeil*2];
      memcpy(sup,index,sizeof(SUPIndex)*indexCeil);
      delete [] index;
      index=sup;
      indexCeil*=2;
  }
  SUPIndex *idx=&(index[nbIndex]);
  idx->pos=pos;
  idx->size=size;
  idx->pts=pts;
  nbIndex++;
  return 1;
}

//**************************************************************

ADMVideoSupSub::ADMVideoSupSub(char *supfile,uint32_t ix)
{
  _fd=fopen(supfile,"r");
  ADM_assert(_fd);
  index=new  SUPIndexArray(50);
  bitmap=NULL;
  parse();
  setPalette(pal);
  currentImage=0;
}
ADMVideoSupSub::~ADMVideoSupSub()
{
  if(_fd) fclose(_fd);
  _fd=NULL;
  if(index) delete index;
  index=NULL; 
  if(bitmap) delete bitmap;
  bitmap=NULL;
}

uint8_t ADMVideoSupSub::parse(void)
{
  uint32_t fileSize;
  uint32_t blockSize,pos;
  uint8_t PTS[8];
  
  fseeko(_fd,0,SEEK_END);
  fileSize=ftello(_fd); 
  fseeko(_fd,0,SEEK_SET);
  
  while(ftello(_fd)<fileSize-12)
  {
    if(fgetc(_fd)!='S') goto fail;
    if(fgetc(_fd)!='P') goto fail;
    // Read timestamp
      fread(PTS,8,1,_fd);
        uint64_t pts;
        uint32_t pts1=(PTS[0])+(PTS[1]<<8)+(PTS[2]<<16)+(PTS[3]<<24);
        uint32_t pts2=(PTS[4])+(PTS[5]<<8)+(PTS[6]<<16)+(PTS[7]<<24);
        pts=pts1+(((uint64_t)pts2)<<32);
        pos=ftello(_fd);
        fread(PTS,2,1,_fd);
        blockSize=(PTS[0]<<8)+PTS[1];
        index->insert(pos,blockSize,pts);
        fseeko(_fd,blockSize-2,SEEK_CUR);
  }
fail:
  fseeko(_fd,0,SEEK_SET);
  printf("[SUP] Found %u images\n",index->nbIndex);
  return 1;

      
}

uint32_t ADMVideoSupSub::getNbImage(void)
{
  return index->nbIndex; 
}

vobSubBitmap *ADMVideoSupSub::getBitmap(
          uint32_t nb,uint32_t *start, uint32_t *end,uint32_t *first,uint32_t *last)
{
  uint32_t size,pos;
  if(currentImage>index->nbIndex) 
  {
      printf("[SUP] Exceeding # images\n");
      return NULL;
  }
  /* Read incoming data */
  SUPIndex *idx=&(index->index[currentImage]);
      
  uint8_t *data=new uint8_t[idx->size];
  fseeko(_fd,idx->pos,SEEK_SET);
  fread(data,idx->size,1,_fd);
  
  
  if(decode(idx->size,data))
  {
    // Do some stats
    if(bitmap)
    {
      int col[16];
      uint32_t sum=0;
      memset(col,0,sizeof(int)*16);
      uint8_t *ptr=bitmap->_bitmap;
      for(int y=0;y<bitmap->_height*bitmap->_width;y++)
      {
         col[*ptr++]++;
         sum++;
      }
      printf("<< STATS >>\n");
      for(int i=0;i<16;i++)
      {
        printf("Col %u %u Percent %u\n",i,col[i],(100*col[i])/sum);
        
      }
    }
    
    
    bitmap->buildYUV(_YUVPalette); 
  }
  
  delete [] data;
  if(bitmap)
  {
    *first=0;
    *last=bitmap->_height-1; 
  }
  currentImage++;
  return bitmap; 
}
#define aprintf printf
#define READ16() (data[_curOffset]<<8)+data[_curOffset+1]; _curOffset+=2;
uint8_t ADMVideoSupSub::decode(uint32_t size, uint8_t *data)
{
  uint32_t odd,even,_subSize,_dataSize,next=0,date,command,dum;
  
  uint32_t _curOffset=0,doneA=0,doneB=0;
        odd=even=0;
        
        _subSize=(data[0]<<8)+data[1];
        if(!_subSize)
        {
            printf("Vobsub: error reading\n");
            return 0;
        }
        
        aprintf("Vobsub: data len =%d\n",_subSize);
        _curOffset=2;
        if(_subSize<4)
        {
          printf("[handleSub] Packet too short!\n");
          return 1; 
        }
        _dataSize=(data[2]<<8)+data[3];
        aprintf("data block=%lu\n",_dataSize);
        if(_dataSize<=4)
        {
            printf("Vobsub: data block too small\n");
            return 0;       
        }
        if(_dataSize-4>=_subSize)
        {
          printf("DataSize too large\n");
          return 0; 
        }
        _curOffset=_dataSize;
        while(2)
        {
                if(_curOffset>_subSize-5) break;
                date=READ16();
                next=READ16();
                if(next==_curOffset-4) break;            // end of command
                
                while(_curOffset<next)
                {
                      
                      
                        command=data[_curOffset++];
                        aprintf("vobsub:Command : %d date:%d next:%d cur:%lu\n",command,date,next,_curOffset);
                        int left=next-_curOffset;
                        switch(command)
                        {
                                case 00:
                                        break;
                                case 01: // start date
                                        break;
                                case 02: // stop date

                                        break;
                                case 03: // Pallette 4 nibble= 16 bits
                                         if(left<2)
                                         {
                                            printf("Command 3: Palette: Not enough bytes left\n");
                                            return 1; 
                                         }
                                         dum=READ16();
                                        _colors[0]=dum>>12;
                                        _colors[1]=0xf & (dum>>8);
                                        _colors[2]=0xf & (dum>>4);
                                        _colors[3]=0xf & (dum);
                                        printf("[SUP] Palette %x %x %x %x\n",_colors[0],_colors[1],_colors[2],_colors[3]);
                                        break;          
                                case 0xff:
                                        break;
                                case 04: // alpha channel
                                         //4 nibble= 16 bits
                                        if(left<2)
                                         {
                                            printf("Command 4: Alpha: Not enough bytes left\n");
                                            return 1; 
                                         }

                                        dum=READ16();
                                        _alpha[0]=dum>>12;
                                        _alpha[1]=0xf & (dum>>8);
                                        _alpha[2]=0xf & (dum>>4);
                                        _alpha[3]=0xf & (dum);
                                        printf("[SUP] Alpha %x %x %x %x\n",_alpha[0],_alpha[1],_alpha[2],_alpha[3]);
                                        break;
                                case 05:
                                        // Coordinates 12 bits per entry X1/X2/Y1/Y2
                                        // 48 bits total / 6 bytes
                                        {
                                                uint16_t a,b,c;
                                                uint32_t nx1,nx2,ny1,ny2;
                                                if(left<6)
                                                {
                                                    printf("Command 5: Coord: Not enough bytes left\n");
                                                    return 1; 
                                                }
                                                if(doneA) return 1;
                                                doneA++;
                                                a=READ16();
                                                b=READ16();
                                                c=READ16();
                                                nx1=a>>4;
                                                nx2=((a&0xf)<<8)+(b>>8);
                                                ny1=((b&0xf)<<4)+(c>>12);
                                                ny2=c&0xfff;
                                                
                                                aprintf("vobsuv: x1:%d x2:%d y1:%d y2:%d\n",nx1,nx2,ny1,ny2);
                                                
                                                if(bitmap && ((nx2+1-nx1)==bitmap->_width) && ((ny2+1-ny1)==bitmap->_height))
                                                {       // Reuse old bitmap
                                                        bitmap->clear();
                                                }
                                                else
                                                {
                                                  if(bitmap)
                                                        delete bitmap;
                                                  bitmap=NULL;
                                                  _subW=nx2+1-nx1;
                                                  _subH=ny2+1-ny1;
                                                  bitmap=new vobSubBitmap(_subW,_subH);
                                                }
                                                                        
                                        }
                                        break;
                                case 06: // RLE offset 
                                        // 2*16 bits : odd offset, even offset
                                        {
                                                if(doneB) return 1;
                                                doneB++;
                                                if(left<4)
                                                {
                                                    printf("Command 6: RLE: Not enough bytes left\n");
                                                    return 1; 
                                                }
                                                odd=READ16();
                                                even=READ16();
 
                                        }
                                        break;   
                                default:                                                     
                                        printf("Unknown command:%d\n",command);
                                        return 0;
                                  
                        } //End switch command     
                }// end while
        }
        /*****/
        if(bitmap && odd && even) 
        {
                bitmap->clear();
                decodeRLE(odd,0,even,data,_dataSize);
                decodeRLE(even,1,0,data,_dataSize);
        }
    return 1;
}
uint8_t ADMVideoSupSub::decodeRLE(uint32_t off,uint32_t start,uint32_t end,uint8_t *data,uint32_t _dataSize)
{
   if(!bitmap) return 0;
        uint32_t _curOffset=0;
        uint32_t oldoffset=_curOffset;
        uint32_t stride=_subW;
        uint32_t x,y;
        uint8_t *ptr=bitmap->_bitmap;
        uint8_t *alpha=bitmap->_alphaMask;
        uint32_t a,b;
        int     nibbleparity=0;
        int     nibble=0;
        
        int run,color;
        
#define SKIPNIBBLE        {nibbleparity=0;}
#define readbyte() data[_curOffset++]
#define NEXTNIBBLE(x) if(nibbleparity){ x=nibble&0xf;nibbleparity=0;}else {nibble=readbyte();nibbleparity=1;x=nibble>>4;}
       
        _curOffset=off;
        aprintf("Vobsub: Rle at offset :%d datasize:%d (stride:%d)\n",off,_dataSize,stride);
        if(!ptr)
        {
                printf("Vobsub:No infos yet RLE data...\n");
                 return 1;
        }
        x=0;
        y=0;
        while(
               (_curOffset<_dataSize)
            && (y<(_subH>>1)) 
            && ((!end) || (_curOffset<end))
        )
        {
               NEXTNIBBLE(a);
               if(a<4)
               {
                 a<<=4;
                 NEXTNIBBLE(b);
                 a|=b;
                 if(a<0x10)
                 {
                        a<<=4;
                        NEXTNIBBLE(b);
                        a|=b;
                        if(a<0x40)
                        {
                                a<<=4;
                                NEXTNIBBLE(b);
                                a|=b;
                                if(a<0x100)
                                {
                                        a|=(stride-x)<<2;
                                }
                        }
                 }
              }
              run=a>>2;
              color=3-(a&0x3);
             // aprintf("Vobsub: Run:%d color:%d\n",run,color);
              if((run>stride-x) || !run)
                run=stride-x;
              
              //memset(ptr,color,run);
              memset(ptr,_colors[color],run);
              memset(alpha,_alpha[color],run);
              if(run!=stride) bitmap->setDirty(y*2+start);
              x+=run;
              ptr+=run;
              alpha+=run;
              //  aprintf("x:%d y:%d\n",x,y);
              if(x>=stride)
              {
                        
                     y++;
                     x=0;
                     ptr=bitmap->_bitmap+(y*2+start)*stride;
                     alpha=bitmap->_alphaMask+(y*2+start)*stride;
                     SKIPNIBBLE;
              }
        }
        aprintf("vobsub End :%d y:%d\n",_curOffset,y); 
        _curOffset=oldoffset;
  
  return 1;
}
uint8_t ADMVideoSupSub::setPalette( uint32_t *palette )
{
uint8_t r,g,b,a;
uint8_t y;
int8_t u,v;
uint32_t value;
        for(int i=0;i<16;i++)
        {
               value=palette[i];
               r=(value>>16)&0xff;
               g=(value>>8)&0xff; 
               b=(value)&0xff;
               
               COL_RgbToYuv(b,  g,  r, &y, &u,&v);
               
                _YUVPalette[i]=y;
        }
        return 1;

}
//EOF

