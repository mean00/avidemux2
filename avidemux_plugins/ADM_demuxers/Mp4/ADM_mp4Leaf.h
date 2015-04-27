/*
http://developer.apple.com/library/mac/#documentation/QuickTime/QTFF/QTFFChap2/qtff2.html#//apple_ref/doc/uid/TP40000939-CH204-33299
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
MKMP4LEAF('moof',MOOF,1),
MKMP4LEAF('trak',TRACK,1),
MKMP4LEAF('mdia',MDIA,1),
MKMP4LEAF('minf',MINF,1),
MKMP4LEAF('dinf',DINF,1),
MKMP4LEAF('stbl',STBL,1),
MKMP4LEAF('traf',TRAF,1),
/* Regular atoms (header) */
MKMP4LEAF('tfhd',TFHD,0),
MKMP4LEAF('tfdt',TFDT,0),
MKMP4LEAF('trun',TRUN,0),
MKMP4LEAF('mfhd',MFHD,0),
MKMP4LEAF('mvhd',MVHD,0),
MKMP4LEAF('tkhd',TKHD,0),
MKMP4LEAF('mdhd',MDHD,0),
MKMP4LEAF('hdlr',HDLR,0),

/*   stbl atom            */
MKMP4LEAF('stsd',STSD,0),
MKMP4LEAF('stts',STTS,0),
MKMP4LEAF('sidx',SIDX,0),
MKMP4LEAF('stsc',STSC,0),
MKMP4LEAF('stsz',STSZ,0),
MKMP4LEAF('stco',STCO,0),
MKMP4LEAF('co64',STCO64,0),
MKMP4LEAF('stsh',STSH,0),
MKMP4LEAF('stss',STSS,0),
/* Edit list */
MKMP4LEAF('elst',ELST,0), // http://wiki.multimedia.cx/index.php?title=QuickTime_container#elst
MKMP4LEAF('edts',EDTS,1),


MKMP4LEAF('ctts',CTTS,0),

/* Data */
MKMP4LEAF('mdat',MDAT,0),



MKMP4LEAF('dumm',DUMMY,0)
    
#endif

//EOF
