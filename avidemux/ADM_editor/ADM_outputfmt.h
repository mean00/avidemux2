//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// (c) Mean, fixounet@free.fr
//
// Copyright: See COPYING file that comes with this distribution
// 2002-2004
//
#ifndef ADM_OUT_FMT
#define ADM_OUT_FMT

typedef enum 
{
	ADM_AVI=0,
	ADM_AVI_DUAL,
	ADM_AVI_PAK,
	ADM_AVI_UNP,
	ADM_PS,
	ADM_TS,
	ADM_ES,
	ADM_MP4,
	ADM_OGM,
	ADM_PSP,
	ADM_FLV,
        ADM_MATROSKA,
	ADM_DUMMY,
	ADM_FORMAT_MAX,
}ADM_OUT_FORMAT;

typedef struct 
{
  ADM_OUT_FORMAT format;
  const char *text;
}ADM_FORMAT_DESC;
/**
 * 	This is used to fill-in the menus in GUIs
 */
const ADM_FORMAT_DESC ADM_allOutputFormat[]=
{
  {ADM_AVI,QT_TR_NOOP("AVI")},
  {ADM_AVI_DUAL,QT_TR_NOOP("AVI, dual audio")},
  {ADM_AVI_PAK,QT_TR_NOOP("AVI, pack VOP")},
  {ADM_AVI_UNP,QT_TR_NOOP("AVI, unpack VOP")},  
  {ADM_PS,QT_TR_NOOP("MPEG-PS (A+V)")},
  {ADM_TS,QT_TR_NOOP("MPEG-TS (A+V)")},
  {ADM_ES,QT_TR_NOOP("MPEG video")},
  {ADM_MP4,QT_TR_NOOP("MP4")},
  {ADM_OGM,QT_TR_NOOP("OGM")},
  {ADM_PSP,QT_TR_NOOP("PSP")},
  {ADM_FLV,QT_TR_NOOP("FLV")},
  {ADM_MATROSKA,QT_TR_NOOP("MKV")},
  {ADM_DUMMY,QT_TR_NOOP("DUMMY")}
};

#endif
//EOF
