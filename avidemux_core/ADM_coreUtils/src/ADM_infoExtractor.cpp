/***************************************************************************
                          ADM_infoextractor
                             -------------------
           - extract additionnal info from header (mp4/h263)                  
**************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
//#include "ADM_mp4.h"

#define aprintf(...) {}
#include "ADM_getbits.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"

static void refineH264FrameType(uint8_t *head,uint8_t *tail,uint32_t *flags);
bool ADM_findMpegStartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
/*
    Extract width & height from vol header passed as arg


*/
uint8_t extractMpeg4Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h,uint32_t *time_inc)
{
    // Search startcode
    uint8_t b;
    uint32_t idx=0;
    uint32_t mw,mh;
    uint32_t timeVal;
    
    //mixDump(data,dataSize);
    //printf("\n");
    while(1)
    {
        uint32_t startcode=0xffffffff;
        while(dataSize>2)
        {
            startcode=(startcode<<8)+data[idx];
            idx++;
            dataSize--;
            if((startcode&0xffffff)==1) break;
        }
        if(dataSize>2)
        {
            //printf("Startcodec:%x\n",data[idx]);
            if((data[idx]&0xF0)==0x20) //VOL start
            {
                dataSize--;
                idx++;
#if 0
                printf("VOL Header:\n");

                if(dataSize<16)
                {
                  mixDump(data+idx,dataSize);printf("\n");
                }
                else
                {
                  mixDump(data+idx,16);printf("\n");
                }
#endif
                // Here we go !
                GetBitContext s;
                init_get_bits( &s,data+idx, dataSize*8);
                //
                skip_bits1(&s); // Random access
                skip_bits(&s,8); // Obj type indication
                if(get_bits(&s,1)) // VO od 
                {
                      skip_bits(&s,4); // Ver
                      skip_bits(&s,3);  // Priority
                }
                if(get_bits(&s,4)==15) // custom A/R
                {
                      skip_bits(&s,8);
                      skip_bits(&s,8);
                }
                if(get_bits(&s,1)) // Vol control param
                {
                      skip_bits(&s,2);   //Chroma
                      skip_bits(&s,1);   // Low delay
                      if(get_bits(&s,1)) // VBV Info
                      {
                        skip_bits(&s,16);
                        skip_bits(&s,16);
                        skip_bits(&s,16);
                        skip_bits(&s,15);
                        skip_bits(&s,16);
                      }
                  }
                 skip_bits(&s,2); //  Shape
                 skip_bits(&s,1); //  Marker
                 timeVal=get_bits(&s,16); // Time increment
                 *time_inc = av_log2(timeVal - 1) + 1;
                 if (*time_inc < 1)
                    *time_inc = 1;
                 skip_bits(&s,1); //  Marker
                 if(get_bits(&s,1)) // Fixed vop rate, compute how much bits needed
                 {
                     get_bits(&s, *time_inc);
                 }
                  skip_bits(&s,1); //  Marker
                  mw=get_bits(&s,13);
                  skip_bits(&s,1); //  Marker
                  mh=get_bits(&s,13);
                // /Here we go
                //printf("%d x %d \n",mw,mh);
                *h=mh;
                *w=mw;
                return 1;;
                // Free get bits ?
                // WTF ?
            }
            continue;
        }
        else
        {
            printf("No more startcode\n");
            // Free get bits ?
            return 0;
            
        }
    }
    
    return 0;
}
/**
    \fn extractVopInfo
    \brief extract info from vop : Vop type, module time base, time inc
    
    Warning this function expects data to start AFTER startcode, contrarily to other functions here!
*/

uint8_t extractVopInfo(uint8_t *data, uint32_t len,uint32_t timeincbits,uint32_t *vopType,uint32_t *modulo, uint32_t *time_inc,uint32_t *vopcoded)
{
   GetBitContext s;
   int vop;
   uint32_t vp,tinc;
           init_get_bits( &s,data, len*8);
           vop=get_bits(&s,2);
           switch(vop)
           {
             case 0: vp=AVI_KEY_FRAME;break;
             case 1: vp=0;break;
             case 2: vp=AVI_B_FRAME;break;
             case 3: vp=0;break;  // D FRAME ????
             default:
                printf("Unknown vop type :%d\n",vop);
                return 0;
           }
           /* Read modulo */
           int imodulo=0;
           while (get_bits1(&s) != 0)
                  imodulo++;
           if(!get_bits1(&s))
           {
              printf("Wrong marker1\n");
              return 0; 
           }
           
           /* Read time */
           tinc=get_bits(&s,timeincbits);
           /* Marker */
            if(!get_bits1(&s))
           {
              printf("Wrong marker2\n");
              return 0; 
           }
           /* Vop coded */
           *modulo=imodulo;
           *vopcoded=get_bits1(&s);
           *vopType=vp;
           *time_inc=tinc;
           return 1;
}
/**
      \brief extractH263FLVInfo
      \fn Extract width/height from FLV header
*/
uint8_t extractH263FLVInfo(uint8_t *buffer,uint32_t len,uint32_t *w,uint32_t *h)
{
        GetBitContext gb;
        int format;
        init_get_bits( &gb,buffer, len*8);
        if (get_bits_long(&gb, 17) != 1) {
            printf("[FLV]Wrong FLV1 header\n");
            return 0;
        }
        format = get_bits(&gb, 5);
        if (format != 0 && format != 1) {
            printf("[FLV]Wrong FLV1 header format\n");
            return 0;        }
        
        get_bits(&gb, 8); /* picture timestamp */
        format = get_bits(&gb, 3);
        switch (format) {
        case 0:
            *w = get_bits(&gb, 8);
            *h = get_bits(&gb, 8);
            break;
        case 1:
            *w = get_bits(&gb, 16);
            *h = get_bits(&gb, 16);
            break;
        case 2:
            *w = 352;
            *h = 288;
            break;
        case 3:
            *w = 176;
            *h = 144;
            break;
        case 4:
            *w = 128;
            *h = 96;
            break;
        case 5:
            *w = 320;
            *h = 240;
            break;
        case 6:
            *w = 160;
            *h = 120;
            break;
        default:
             printf("[FLV]Wrong width format\n");
             return 0;
            break;
        }
        return 1;
}
/*
        Extract H263 width & height from header

*/
uint8_t extractH263Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h)
{
uint32_t val;
                GetBitContext s;
                init_get_bits( &s,data, dataSize*8);
                
                 mixDump(data,10);
                 val=get_bits(&s,16);
                 if(val)
                 {
                    printf("incorrect H263 header sync\n");
                    return 0;
                 }
                 val=get_bits(&s,6);
                 if(val!=0x20)
                 {
                    printf("incorrect H263 header sync (2)\n");
                    return 0;
                 }
                 //
                 skip_bits(&s,8); // timestamps in 30 fps tick
                 skip_bits(&s,1); // Marker
                 skip_bits(&s,1); // Id
                 skip_bits(&s,1); // Split
                 skip_bits(&s,1); // Document Camera indicator
                 skip_bits(&s,1); // Full Picture Freeze Release
                 val=get_bits(&s,3);
                 switch(val)
                 {
                   
                    case 1: *w=128;*h=96;return 1;break;
                    case 2: *w=176;*h=144;return 1;break;
                    case 6:
                    case 7:
                            printf("H263+:Todo\n");
                    default:
                        printf("Invalid format\n");return 0;break;
                 }
                 return 0;
}
/**
    \fn unescapeH264
    \brief Remove escape stuff

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
        \fn extractVUIInfo
*/
static uint8_t extractVUIInfo(GetBitContext *s, uint32_t *fps1000, uint32_t *darNum, uint32_t *darDen)
{
	*fps1000 = *darNum = *darDen = 0;

	if (get_bits1(s))
	{
		unsigned int aspect_ratio_information = get_bits(s, 8);

		if (aspect_ratio_information == 255)
		{
			*darNum = get_bits_long(s, 16);
			*darDen = get_bits_long(s, 16);
		}
		else if (aspect_ratio_information < sizeof(pixel_aspect) / sizeof(*pixel_aspect))
		{
			*darNum = pixel_aspect[aspect_ratio_information].num;
			*darDen = pixel_aspect[aspect_ratio_information].den;
		}
	}

	if (get_bits1(s))	// overscan
		get_bits1(s);

	if (get_bits1(s))	// vsp_color
	{
		get_bits(s, 4);

		if (get_bits1(s))
		{
			get_bits(s, 8);
			get_bits(s, 8);
			get_bits(s, 8);
		}
	}

	if (get_bits1(s))	// chroma
	{
		get_ue_golomb(s);
		get_ue_golomb(s);
	}

	if (get_bits1(s))	// timing
	{
		uint32_t timeinc_unit = get_bits_long(s, 32);
		uint32_t timeinc_resolution = get_bits_long(s, 32);
		uint32_t fixed_fps = get_bits1(s);

		if (timeinc_unit > 0 && timeinc_resolution > 0)
			*fps1000 = (uint32_t)(((float)timeinc_resolution / (float)timeinc_unit) * 1000);

		if (fixed_fps)
			*fps1000 /= 2;
	}

	return 1;
}

/**
    \fn extractSPSInfo
    \brief Extract info from H264 SPS
    See 7.3.2.1 of 14496-10
*/
uint8_t extractSPSInfo(uint8_t *data, uint32_t len,uint32_t *wwidth,uint32_t *hheight, uint32_t *fps1000, uint32_t *darNum, uint32_t *darDen)
{
   GetBitContext s;
   
   uint32_t profile,constraint,level,pic_order_cnt_type,w,h, mbh, frame_mbs_only;
   uint8_t buf[len];
   uint32_t outlen;
   uint32_t id,dum;
   
           outlen=unescapeH264(len,data,buf);
           init_get_bits( &s,buf, outlen*8);
            
           profile=get_bits(&s,8);
           constraint=get_bits(&s,8)>>5;
           level=get_bits(&s,8);
           id=get_ue_golomb(&s); // Seq parameter set id           
           printf("[H264]Profile : %u, Level :%u, SPSid:%u\n",profile,level,id);
           if(profile>=100) // ?? Borrowed from H264.C/FFMPEG
           {
              printf("[H264]Warning : High profile\n");
              if(get_ue_golomb(&s) == 3) //chroma_format_idc
                get_bits1(&s);  //residual_color_transform_flag
            get_ue_golomb(&s);  //bit_depth_luma_minus8
            get_ue_golomb(&s);  //bit_depth_chroma_minus8
            get_bits1(&s);
			if (get_bits1(&s))
				get_bits(&s, 8);
           }
           

           dum=get_ue_golomb(&s); // log2_max_frame_num_minus4
           printf("[H264]Log2maxFrame-4:%u\n",dum);
           pic_order_cnt_type=get_ue_golomb(&s);
           printf("[H264]Pic Order Cnt Type:%u\n",pic_order_cnt_type);
           if(!pic_order_cnt_type) // pic_order_cnt_type
           {
              dum=get_ue_golomb(&s); //log2_max_pic_order_cnt_lsb_minus4
              printf("[H264]Log2maxPix-4:%u\n",dum);
           }else
           {
             if(pic_order_cnt_type==1)
             {
                 get_bits1(&s);   //delta_pic_order_always_zero_flag
                 get_se_golomb(&s);   //offset_for_non_ref_pic
                 get_se_golomb(&s);  // offset_for_top_to_bottom_field
                 int i=get_ue_golomb(&s);  //num_ref_frames_in_pic_order_cnt_cycle

                 for(int j=0;j<i;j++)
                 {
                      get_se_golomb(&s);
                 }
             }else 
             {
               printf("Error in SPS\n");
               return 0;
             }
           }
           dum=get_ue_golomb(&s);     //num_ref_frames
           printf("[H264] # of ref frames : %u\n",dum);
           get_bits1(&s);         // gaps_in_frame_num_value_allowed_flag
		   w = get_ue_golomb(&s) + 1;   //pic_width_in_mbs_minus1

		   mbh = get_ue_golomb(&s) + 1;
		   frame_mbs_only = get_bits1(&s);
		   h = (2 - frame_mbs_only) * mbh;   //pic_height_in_mbs_minus1

           printf("[H264] Width in mb -1  :%d\n",w); 
           printf("[H264] Height in mb -1 :%d\n", h);

		   *wwidth = w * 16;
		   *hheight= h * 16;
           
		   if (!frame_mbs_only)
			   get_bits1(&s);

		   get_bits1(&s);

		   if(get_bits1(&s))
		   {
			   get_ue_golomb(&s);
			   get_ue_golomb(&s);
			   get_ue_golomb(&s);
			   get_ue_golomb(&s);
		   }

		   if(get_bits1(&s))
           {
			   extractVUIInfo(&s, fps1000, darNum, darDen);
               printf("[H264] Fps %"LU", a.r. %"LU",%"LU"\n",*fps1000,*darNum,*darDen);
           }else
            {
                printf("[H264] Unknown FPS, setting 25\n");
                *fps1000=25000;
            }
           return 1;
}
/**
      \fn extractH264FrameType
      \brief return frametype in flags (KEY_FRAME or 0). To be used only with  mkv/mp4 nal type (i.e. no startcode)
      
*/
uint8_t extractH264FrameType(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags)
{
  uint8_t *head=buffer, *tail=buffer+len;
  uint8_t stream;
  
  uint32_t val,hnt;  
  
// FIXME :  no startcode only !
  
  while(head+4<tail)
  {
    
              uint32_t length=(head[0]<<24) + (head[1]<<16) +(head[2]<<8)+(head[3]);
              if(length>len||length<6)
              {
                printf("Warning , incomplete nal (%u/%u),(%0x/%0x)\n",length,len,length,len);
                *flags=0;
                return 0;
              }
              head+=4; // Skip nal lenth
              length-=4;
              stream=*(head++)&0x1F;
                switch(stream)
                {
                  case NAL_IDR: 
                                  *flags=AVI_KEY_FRAME;
                                  
                                  return 1;
                                  break; 
                  case NAL_NON_IDR: 
                                  refineH264FrameType(head,tail,flags);
                                  return 1;
                                  break;
                  default:
                          printf("??0x%x\n",stream);
                          head+=length-5;
                          continue;
                }
  }
  printf("No stream\n");
  return 0;
}

/**
      \fn extractH264FrameType_startCode
      \brief return frametype in flags (KEY_FRAME or 0). To be used only with  avi / mpeg TS nal type (i.e. with startcode)
      
*/
uint8_t extractH264FrameType_startCode(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags)
{
  uint8_t *head=buffer, *tail=buffer+len;
  uint8_t stream;
#define NAL_NON_IDR       1
#define NAL_IDR           5
#define NAL_SEI           6

  uint32_t val,hnt;  
  
// FIXME :  no startcode only !
  
  while(head+4<tail)
  {
          // Search startcode
      
                hnt=(head[0]<<24) + (head[1]<<16) +(head[2]<<8)+(head[3]);
                head+=4;
                while((hnt!=1) && head<tail)
                {

                        hnt<<=8;
                        val=*head++;
                        hnt+=val;
                }
                if(head>=tail) break;
                stream=*(head++) &0x1f;
                switch(stream)
                {
                  case NAL_IDR: 
                                  *flags=AVI_KEY_FRAME;
                                 // printf("IDR\n");
                                  return 1;
                                  break; 
                  case NAL_NON_IDR: 
                                  refineH264FrameType(head,tail,flags);
                                  return 1;
                                  break;
                  default:
                          printf("??0x%x\n",stream);
                          continue;
                }
  }
  printf("No stream\n");
  return 0;
}
/**
    \fn refineH264FrameType
    \brief Try to detect B slice, warning the stream is not escaped!
*/
void refineH264FrameType(uint8_t *head,uint8_t *tail,uint32_t *flags)
{
GetBitContext s;
uint32_t sliceType;
            *flags=0;
            init_get_bits(&s,head, (tail-head)*8);
            get_ue_golomb(&s);
            sliceType= get_ue_golomb(&s);
            if(sliceType > 9) 
            {
              printf("Weird Slice %d\n",sliceType);
              return ;
            }
            if(sliceType > 4)
                sliceType -= 5;
            if(sliceType==3) *flags=AVI_B_FRAME;  
}
/**
    \fn ADM_searchVop
    \brief search vop header in a bitstream 
    Used for packed bitstream and also to identify the frametype

*/
uint32_t ADM_searchVop(uint8_t *begin, uint8_t *end,uint32_t *nb, ADM_vopS *vop,uint32_t *timeincbits)
{

	uint32_t off=0;
	uint32_t globalOff=0;
	uint32_t voptype;
	uint8_t code;
        uint32_t w,h,t;
        uint32_t modulo,time_inc,vopcoded,vopType;
	*nb=0;
	while(begin<end-3)
	{
    	if( ADM_findMpegStartCode(begin, end,&code,&off))
    	{
        	if(code==0xb6)
			{
				// Analyse a bit the vop header
				uint8_t coding_type=begin[off];
				coding_type>>=6;
				aprintf("\t at %u %d Img type:%s\n",off,*nb,s_voptype[coding_type]);
				switch(coding_type)
				{
					case 0: voptype=AVI_KEY_FRAME;break;
					case 1: voptype=0;break;
					case 2: voptype=AVI_B_FRAME;break;
					case 3: printf("[Avi] Glouglou\n");voptype=0;break;

				}
        	                vop[*nb].offset=globalOff+off-4;
				vop[*nb].type=voptype;



                                /* Get more info */
                                if( extractVopInfo(begin+off, end-begin-off, *timeincbits,&vopType,&modulo, &time_inc,&vopcoded))
                                {
                                    aprintf(" frame found: vopType:%x modulo:%d time_inc:%d vopcoded:%d\n",vopType,modulo,time_inc,vopcoded);
                                    vop[*nb].modulo=modulo;
                                    vop[*nb].timeInc=time_inc;
                                    vop[*nb].vopCoded=vopcoded;
                                }
                                *nb=(*nb)+1;
                                begin+=off+1;
				globalOff+=off+1;
				continue;

			}
                else if(code==0x20 && off>=4	) // Vol start
                {

                   if(extractMpeg4Info(begin+off-4,end+4-off-begin,&w,&h,timeincbits))
                   {
                      aprintf("Found Vol header : w:%d h:%d timeincbits:%d\n",w,h,*timeincbits);
                   }

                }
        	begin+=off;
        	globalOff+=off;
        	continue;
    	}
    	return 1;
    }
	return 1;
}


//EOF
