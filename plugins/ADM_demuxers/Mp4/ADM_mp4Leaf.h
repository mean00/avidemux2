/*
http://developer.apple.com/documentation/QuickTime/QTFF/QTFFChap2/chapter_3_section_5.html#//apple_ref/doc/uid/DontLinkBookID_69-CH204-BBCJEIIA
*/

#if !defined(ADM_MPEGLEAF_H) || defined(ADMMP4_TAB_LEAF)

#ifndef ADM_MPEGLEAF_H
#define ADM_MPEGLEAF_H
#endif

#ifndef ADMMP4_TAB_LEAF
#define MKMP4LEAF(a,b,c) ADM_MP4_##b
#else
#undef MKMP4LEAF
#define MKMP4LEAF(a,b,c) {(uint32_t)a,ADM_MP4_##b,c}
#endif

/* Container atom */
MKMP4LEAF('moov',MOOV,1),
MKMP4LEAF('trak',TRACK,1),
MKMP4LEAF('mdia',MDIA,1),
MKMP4LEAF('minf',MINF,1),
MKMP4LEAF('dinf',DINF,1),
MKMP4LEAF('stbl',STBL,1),

/* Regular atoms (header) */

MKMP4LEAF('mvhd',MVHD,0),
MKMP4LEAF('tkhd',TKHD,0),
MKMP4LEAF('mdhd',MDHD,0),
MKMP4LEAF('hdlr',HDLR,0),

/*   stbl atom            */
MKMP4LEAF('stsd',STSD,0),
MKMP4LEAF('stts',STTS,0),
MKMP4LEAF('stsc',STSC,0),
MKMP4LEAF('stsz',STSZ,0),
MKMP4LEAF('stco',STCO,0),
MKMP4LEAF('co64',STCO64,0),
MKMP4LEAF('stsh',STSH,0),
MKMP4LEAF('stss',STSS,0),




MKMP4LEAF('ctts',CTTS,0),

/* Data */
MKMP4LEAF('mdat',MDAT,0),



MKMP4LEAF('dumm',DUMMY,0)
    
#endif

//EOF
