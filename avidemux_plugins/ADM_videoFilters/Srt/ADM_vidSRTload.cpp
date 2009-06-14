/***************************************************************************
                          ADM_vidload.cpp  -  description
                             -------------------

	Load the subtitles either in srt or sub format

	The structure is
		uint32_t 		startTime in ms from beginning
		uint32_t 		endTime  in ms from beginning

	All text are stored as utf16 after loading


    begin                : Thu Aug 09 2003
    copyright            : (C) 2003 by mean
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

#include <math.h>
#include <iconv.h>
#include <errno.h>

//#define __STDC_ISO_10646__

#include "ADM_default.h"

#include "ADM_videoFilter.h"

#include "ADM_vidFont.h"
#include "ADM_vidSRT.h"

#define aprintf(...) {}

#define ADM_RAW 1024

static iconv_t myConv=(iconv_t)-1;;
static int ADM_SubAtoi(uint16_t *in);

static uint8_t	ADM_utfInit( char *charset );
static uint8_t ADM_utfConv(ADM_GLYPH_T *out, char *in,uint32_t nbIn, uint32_t *nbOut);
static uint8_t ADM_utfEnd( void);




uint8_t	ADMVideoSubtitle::loadFont (void)
{

  if (!_font->initFreeType ((char *)_conf->_fontname))
    {
      printf ("\n Free type init failed for font %s!", _conf->_fontname);
    }
  else
    {
      _font->fontSetSize (_conf->_fontsize);
    }

  return 1;
}

//
//      Load subtitles in .sub format i.e. {xx}{yy} |   |
//
// {3610}{3656}Pripraven�uv�a ich|v naom dome s primeranou ctou.
//__________________________________________________________________
uint8_t  ADMVideoSubtitle::loadSubTitle (void)
{

  char string[500];
  uint32_t current_line = 0;
  // first cound how many line
  _line = 0;
   // init iconv...
  if(! ADM_utfInit(_conf->_charset))
  {
  	printf("Problem initializing iconv, aborting\n");
	return 0;

  }
  while (fgets (string, 200, _fd))
    {
      _line++;
    }
  printf ("\n subs : %ld lines\n", _line);
  // rewind
  fseek (_fd, 0, SEEK_SET);
  //
  _subs = new subLine[_line];
  if (!_subs)
    return 0;
  memset (_subs, 0, sizeof (subLine) * _line);



//
  for (uint32_t i = 0; i < _line; i++)
    {
      fgets (string, ADM_RAW, _fd);
      //printf("\n in : %s ",string);
      if (string[0] != '{')
	continue;
      subParse ((subLine *) & (_subs[current_line]), string);
      current_line++;
    }

  // the effective number of line we have
  _line = current_line;
  return 1;

 ADM_utfEnd( );
}

//___________________________________________________________________
//      In case of sub file, take one line and convert it to unicode
//	we also extract the start/end and split it to N lines
//	Does not work if input is UTF16. UTF8 is ok.
//___________________________________________________________________
uint8_t	ADMVideoSubtitle::subParse (subLine * in, char *string)
{


  static ADM_GLYPH_T 	final[ADM_RAW];	// convert to unicode
  uint32_t   	startindex, endindex,textindex;
  uint32_t 	j;
  float 	f;

  uint32_t 	finallen=0;
  uint32_t	alpha;

  j = 1;


  ADM_utfConv(final,string,strlen(string),&finallen);

  // ignore { }
  while (ADM_ASC(final[j]) != '}' && j < finallen)
    j++;

  startindex =1;
  j += 2;			// skip }{
  endindex =j;
  while (ADM_ASC(final[j]) != '}' && j < finallen)
    j++;
  // The text starts here
  textindex = j + 1;

  if (j >= finallen - 1)
    {
      printf ("***ERR: Suspicious line !!!\n");
      return 0;

    }

  // convert frame -> time in ms
  // skip utf marker if present
  alpha= ADM_SubAtoi (&final[1]);
  f=(float)alpha;
  aprintf("%s start:%d",string,alpha);
  f = f * 1000000. / _info.fps1000;
  in->startTime = (uint32_t) floor (f);

  alpha = ADM_SubAtoi (&final[endindex]);
  f=(float)alpha;
  aprintf(" end:%d\n",alpha);
  f = f * 1000000. / _info.fps1000;

  in->endTime = (uint32_t) floor (f);

  uint32_t lllines=0;
  finallen-=textindex;
  if(finallen==0)
  	{
		printf("Empty line\n");
		in->nbLine=0;
		return 1;
	}
  // Count the number of | found
  for(j=0;j<finallen;j++)
  {
  	if(ADM_ASC(final[j+textindex])=='|') lllines++;
  }

  aprintf("%lu -->%lu\n",in->startTime,in->endTime);
  in->nbLine=lllines+1;
  in->string=new ADM_GLYPH_T *[in->nbLine];
  in->lineSize=new uint32_t[in->nbLine];

  for(uint32_t i=0;i<in->nbLine;i++)
  {
  	in->string[i]=new ADM_GLYPH_T[finallen]; 	// yes, we overshot
	in->lineSize[i]=0;
  }

  uint32_t curline=0, curindex=0;
  for(uint32_t i=0;i<finallen;i++)
  {
  	if(ADM_ASC(final[i+textindex])=='|')
	{
		in->lineSize[curline]=curindex;
		curindex=0;
		curline++;
	}
	else
	{
		in->string[curline][curindex++]=final[i+textindex];
	}

  }
  if(curindex)
  	in->lineSize[curline]=curindex;
  return 1;
}

//
//      Load subtitles in .srt format i.e. {xx}{yy} |   |
//__________________________________________________________________
uint8_t
ADMVideoSubtitle::loadSRT (void)
{


  static ADM_GLYPH_T 	final[ADM_RAW];	// convert to unicode
  ADM_GLYPH_T  		temp[SRT_MAX_LINE][ADM_RAW];
  uint32_t		tempSize[SRT_MAX_LINE];
  char			string[ADM_RAW];


  uint32_t line;
  uint32_t len;
  // Init iconv
   uint32_t 	finallen=0;
  subLine	*current;

  if(! ADM_utfInit(_conf->_charset))
  {
  	printf("Problem initializing iconv, aborting\n");
	return 0;

  }

  // first cound how many line
  line = 0;
  _line = 0;
  while (fgets (string, 300, _fd))
    line++;
  printf ("\n subs : %ld lines\n", line);
  // rewind
  fseek (_fd, 0, SEEK_SET);
  //
  _subs = new subLine[line];

  if (!_subs)
    return 0;

  memset (_subs, 0, sizeof (subLine) * line);

  // read and allocate

  uint32_t j;
  int state = 0;
  int stored=0;

  for (uint32_t i = 0; i < line; i++)
    {
	current=&_subs[_line];
	fgets (string, ADM_RAW, _fd);
	ADM_utfConv(final,string,strlen(string),&finallen);
	// Purge cr/lf
	switch (state)
	{
	case 0:		// waiting for number
                  if(!_line && (final[0]&0xfefe)==0xfefe)
                  {
                      j=ADM_SubAtoi (final+1);
                  }
                  else
		      j = ADM_SubAtoi (final);
		if (j == _line + 1)
		{
			stored=0;
			state = 1;
		}
		break;
	case 1:		// waiting for time
		{
			uint32_t sh, sm, ss, ms;
			uint32_t dh, dm, ds, md;

			ADM_GLYPH_T *cur=&final[0];

	  		sh=ADM_SubAtoi(cur);
			cur+=3;
			sm=ADM_SubAtoi(cur);
			cur+=3;
			ss=ADM_SubAtoi(cur);
			cur+=3;
			ms=ADM_SubAtoi(cur);
			cur+=3;

			cur+=5;

			dh=ADM_SubAtoi(cur);
			cur+=3;
			dm=ADM_SubAtoi(cur);
			cur+=3;
			ds=ADM_SubAtoi(cur);
			cur+=3;
			md=ADM_SubAtoi(cur);

			_subs[_line].startTime = ms + 1000 * (ss + sm * 60 + sh * 3600);
			_subs[_line].endTime = md + 1000 * (ds + dm * 60 + dh * 3600);
			state = 2;

			aprintf("%d %d %d %d / %d %d %d %d>%s<\n",sh,sm,ss,ms,dh,dm,ds,md,string);

		}

		break;
	case 2:
	   // looking for text
	   // We append each line to text with a | to separate the lines
	  {
	  	if (finallen < 2)
	    	{
	   		// Finished
	      		_line++;
	      		state = 0;
			current->nbLine=stored;
			current->lineSize=new uint32_t[stored];
			current->string=new ADM_GLYPH_T *[stored];
			for(uint32_t i=0;i<stored;i++)
			{
				current->lineSize[i]=tempSize[i];
				current->string[i]=new ADM_GLYPH_T[tempSize[i]];
				memcpy(current->string[i],temp[i],sizeof(ADM_GLYPH_T)*tempSize[i]);
			}
	      		break;
	    	}
	  	else
	    	{
			if(stored>=SRT_MAX_LINE)
			{
				printf("sub:Too much lines, ignoring..\n");
				continue;
			}
	      		// We add it
			memcpy(temp[stored],final,finallen*sizeof(ADM_GLYPH_T));
			tempSize[stored]=finallen;
			stored++;
	    	}
	  }
	  break;

	}

    }
  ADM_utfEnd( );
  aprintf(">> Sub: %d subs stored and loaded\n",_line);
  return 1;

}
// Sort of atoi for utf16
// Very basic
int ADM_SubAtoi(uint16_t *in)
{
	int result=0;
	int d;
	do
	{
		d=ADM_ASC(*in);
		in++;
		if(d>='0' && d<='9')
		{
			result*=10;
			result+=d-'0';
		}
		else
			return result;
	}while(1);


}
//	Init iconv, so that we can go utf16
//
uint8_t	ADM_utfInit( char *charset )
{
	myConv=iconv_open("UTF-16",charset); //"WINDOWS-1251");
	if((long int)myConv==-1)
	{
		printf("\n Error initializing iconv...\n");
		return  0;
	}
	return 1;
}
//
//	Convert the incoming string to utf16
//	While doing it, remove the utf16 makers from input/output
//	(fffe)
//
uint8_t ADM_utfConv(ADM_GLYPH_T *out, char *in,uint32_t nbIn, uint32_t *nbOut)
{
size_t sin,sout,sz;
char *cout,*cin;
uint32_t done=0;

	memset(out,0,ADM_RAW*sizeof(ADM_GLYPH_T));
	sin=strlen(in);
	sout=ADM_RAW;
	cin=in;
	cout=(char *)(out);

	if((uint8_t)in[0]==0xff && (uint8_t)in[1]==0xfe)
	{
		 cin+=2;
		 sin-=2;
	}
	if(!sin)
	{
		*nbOut=0;
		return 1;
	}
#ifdef  ICONV_NEED_CONST
  	sz=iconv(myConv,(const char **)&cin,&sin,&cout,&sout);
#else
	sz=iconv(myConv,&cin,&sin,&cout,&sout);
#endif
	if(sz==-1)
	{
  		printf("Iconv error:%s\n:%s:\n",strerror(errno),in);
		//out[0]=' ';
  	}
	// Get the amound of utf16...
	done=(ADM_RAW-sout)>>1;

#if defined(__WIN32) || defined(ADM_BSD_FAMILY)
	// For Win32 and BSD we swap le and be

       ADM_GLYPH_T glyph;
       for(uint32_t w=0;w<done;w++)
       {
               glyph=out[w];
               out[w]=((glyph&0xff)<<8)+(glyph>>8);
       }
#endif

	if(done)
	{
		if(0xfeff==out[0])
		{
			//aprintf("Removing utf tag\n");
			done--;
			memmove(&out[0],&out[1],done*sizeof(ADM_GLYPH_T));

		}
	}

	while(done &&( ADM_ASC(out[done-1])==0x0a || ADM_ASC(out[done-1]==0x0d))) done--;
	*nbOut=done;
	return 1;

}
// Close iconv
//

uint8_t ADM_utfEnd( void)
{
  // Purge iconv
  if((long int)myConv!=-1)
	{
		iconv_close(myConv);
		myConv=(iconv_t)-1;
	}
}



