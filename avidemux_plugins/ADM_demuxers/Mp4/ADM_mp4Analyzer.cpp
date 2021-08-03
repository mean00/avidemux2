/***************************************************************************

    copyright            : (C) 2007/2015 by mean
    email                : fixounet@free.fr
 * https://arashafiei.wordpress.com/2012/11/13/quick-dash/
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

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_mp4.h"
#include "DIA_coreToolkit.h"
#include "ADM_coreUtils.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_mp4Tree.h"
#include "ADM_vidMisc.h"
#include "ADM_iso639.h"
#include "ADM_aacinfo.h"

extern "C" {
#include "libavutil/pixfmt.h"
}

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

#define TRACK_OTHER 0
#define TRACK_AUDIO 1
#define TRACK_VIDEO 2

// 14496-1 / 8.2.1
typedef enum
{
    Tag_InitialObjDesc    =0x02,
    Tag_ES_Desc           =0x03,
    Tag_DecConfigDesc     =0x04,
    Tag_DecSpecificInfo   =0x05
}MP4_Tag;

/**
    \fn langCodeToIso639
    \brief Stolen from libavcodec/isom.c, original code by François Revol and quink-black.
*/
static std::string langCodeToIso639(uint16_t code)
{
    int i;
    const char macLangTab[][4] =
    {
        "eng",    /*   0 English */
        "fra",    /*   1 French */
        "ger",    /*   2 German */
        "ita",    /*   3 Italian */
        "dut",    /*   4 Dutch */
        "sve",    /*   5 Swedish */
        "spa",    /*   6 Spanish */
        "dan",    /*   7 Danish */
        "por",    /*   8 Portuguese */
        "nor",    /*   9 Norwegian */
        "heb",    /*  10 Hebrew */
        "jpn",    /*  11 Japanese */
        "ara",    /*  12 Arabic */
        "fin",    /*  13 Finnish */
        "gre",    /*  14 Greek */
        "ice",    /*  15 Icelandic */
        "mlt",    /*  16 Maltese */
        "tur",    /*  17 Turkish */
        "hr ",    /*  18 Croatian */
        "chi",    /*  19 Traditional Chinese */
        "urd",    /*  20 Urdu */
        "hin",    /*  21 Hindi */
        "tha",    /*  22 Thai */
        "kor",    /*  23 Korean */
        "lit",    /*  24 Lithuanian */
        "pol",    /*  25 Polish */
        "hun",    /*  26 Hungarian */
        "est",    /*  27 Estonian */
        "lav",    /*  28 Latvian */
           "",    /*  29 Sami */
        "fo ",    /*  30 Faroese */
           "",    /*  31 Farsi */
        "rus",    /*  32 Russian */
        "chi",    /*  33 Simplified Chinese */
           "",    /*  34 Flemish */
        "iri",    /*  35 Irish */
        "alb",    /*  36 Albanian */
        "ron",    /*  37 Romanian */
        "ces",    /*  38 Czech */
        "slk",    /*  39 Slovak */
        "slv",    /*  40 Slovenian */
        "yid",    /*  41 Yiddish */
        "sr ",    /*  42 Serbian */
        "mac",    /*  43 Macedonian */
        "bul",    /*  44 Bulgarian */
        "ukr",    /*  45 Ukrainian */
        "bel",    /*  46 Belarusian */
        "uzb",    /*  47 Uzbek */
        "kaz",    /*  48 Kazakh */
        "aze",    /*  49 Azerbaijani */
        "aze",    /*  50 AzerbaijanAr */
        "arm",    /*  51 Armenian */
        "geo",    /*  52 Georgian */
        "mol",    /*  53 Moldavian */
        "kir",    /*  54 Kirghiz */
        "tgk",    /*  55 Tajiki */
        "tuk",    /*  56 Turkmen */
        "mon",    /*  57 Mongolian */
           "",    /*  58 MongolianCyr */
        "pus",    /*  59 Pashto */
        "kur",    /*  60 Kurdish */
        "kas",    /*  61 Kashmiri */
        "snd",    /*  62 Sindhi */
        "tib",    /*  63 Tibetan */
        "nep",    /*  64 Nepali */
        "san",    /*  65 Sanskrit */
        "mar",    /*  66 Marathi */
        "ben",    /*  67 Bengali */
        "asm",    /*  68 Assamese */
        "guj",    /*  69 Gujarati */
        "pa ",    /*  70 Punjabi */
        "ori",    /*  71 Oriya */
        "mal",    /*  72 Malayalam */
        "kan",    /*  73 Kannada */
        "tam",    /*  74 Tamil */
        "tel",    /*  75 Telugu */
           "",    /*  76 Sinhala */
        "bur",    /*  77 Burmese */
        "khm",    /*  78 Khmer */
        "lao",    /*  79 Lao */
        "vie",    /*  80 Vietnamese */
        "ind",    /*  81 Indonesian */
        "tgl",    /*  82 Tagalog */
        "may",    /*  83 MalayRoman */
        "may",    /*  84 MalayArabic */
        "amh",    /*  85 Amharic */
        "tir",    /*  86 Galla */
        "orm",    /*  87 Oromo */
        "som",    /*  88 Somali */
        "swa",    /*  89 Swahili */
           "",    /*  90 Kinyarwanda */
        "run",    /*  91 Rundi */
           "",    /*  92 Nyanja */
        "mlg",    /*  93 Malagasy */
        "epo",    /*  94 Esperanto */
           "",    /*  95  */
           "",    /*  96  */
           "",    /*  97  */
           "",    /*  98  */
           "",    /*  99  */
           "",    /* 100  */
           "",    /* 101  */
           "",    /* 102  */
           "",    /* 103  */
           "",    /* 104  */
           "",    /* 105  */
           "",    /* 106  */
           "",    /* 107  */
           "",    /* 108  */
           "",    /* 109  */
           "",    /* 110  */
           "",    /* 111  */
           "",    /* 112  */
           "",    /* 113  */
           "",    /* 114  */
           "",    /* 115  */
           "",    /* 116  */
           "",    /* 117  */
           "",    /* 118  */
           "",    /* 119  */
           "",    /* 120  */
           "",    /* 121  */
           "",    /* 122  */
           "",    /* 123  */
           "",    /* 124  */
           "",    /* 125  */
           "",    /* 126  */
           "",    /* 127  */
        "wel",    /* 128 Welsh */
        "baq",    /* 129 Basque */
        "cat",    /* 130 Catalan */
        "lat",    /* 131 Latin */
        "que",    /* 132 Quechua */
        "grn",    /* 133 Guarani */
        "aym",    /* 134 Aymara */
        "tat",    /* 135 Tatar */
        "uig",    /* 136 Uighur */
        "dzo",    /* 137 Dzongkha */
        "jav",    /* 138 JavaneseRom */
    };

    char out[4] = {0};
    if(code >= 0x400 && code != 0x7fff) // packed ISO
    {
        for(i = 2; i >= 0; i--)
        {
            out[i] = 0x60 + (code & 0x1f);
            code >>= 5;
        }
        return std::string(out);
    }
    // else Macintosh Language Codes are used
    uint32_t nb = sizeof(macLangTab) / sizeof(macLangTab[0]);
    if(code >= nb)
        return ADM_UNKNOWN_LANGUAGE;
    const char *lang = macLangTab[code];
    if(!lang[0]) return ADM_UNKNOWN_LANGUAGE;
    int ix = ADM_getIndexForIso639(lang);
    if(ix < 0) return ADM_UNKNOWN_LANGUAGE;
    const ADM_iso639_t *list = ADM_getLanguageList();
    lang = list[ix].iso639_2;
    return std::string(lang);
}

//extern char* ms2timedisplay(uint32_t ms);

/**
    \fn refineAudio
    \brief update track descriptor with additional info. For example # of channels...
*/
bool MP4Header::refineAudio(WAVHeader *header,uint32_t extraLen,uint8_t *extraData)
{
    if(header->encoding!=WAV_AAC || extraLen<2)
        return true;

    ADM_info("Audio track is AAC, checking it...\n");

    AacAudioInfo info;
    if(!ADM_getAacInfoFromConfig(extraLen,extraData,info))
    {
        ADM_warning("Can't get # of channels from AAC extradata.\n");
        return false;
    }

    if(header->channels!=info.channels)
    {
        ADM_warning("Channel mismatch, mp4 says %d, AAC says %d, updating...\n",header->channels,info.channels);
        header->channels=info.channels;
    }

    if(!info.frequency)
    {
        ADM_warning("Invalid sampling frequency = 0\n");
        return false;
    }

    if(header->frequency!=info.frequency)
    {
        ADM_warning("Sample rate mismatch, mp4 says %d, AAC says %d, updating...\n",header->frequency,info.frequency);
        header->frequency=info.frequency;
    }
    return true;
}
/**
      \fn    LookupMainAtoms
      \brief Search main atoms to ease job for other part
*/
uint8_t MP4Header::lookupMainAtoms(void *ztom)
{
    adm_atom *tom=(adm_atom *)ztom;
    adm_atom *moov,*moof=NULL;
    bool success=true;
    ADMAtoms id;
    uint32_t container;

    ADM_info("Analyzing file and atoms\n");

    if(!ADM_mp4SimpleSearchAtom(tom, ADM_MP4_MOOV,&moov))
    {
        ADM_warning("Cannot locate moov atom\n");
        return 0;
    }
    ADM_assert(moov);
    while(!moov->isDone())
    {
        adm_atom son(moov);
        if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
        {
            aprintf("[Lookup]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
        }else
        {
            switch(id)
            {
                case ADM_MP4_MVHD: parseMvhd(&son);break;
                case ADM_MP4_MVEX:
                    {
                        ADM_info("Found mvex at position %u of size %u\n",son.getStartPos(),son.getRemainingSize());
                        parseTrex(&son);
                    }
                    break;
                case ADM_MP4_TRACK:
                    if(!parseTrack(&son))
                    {
                        ADM_info("Parse Track failed\n");
                        success=false;
                    }
                    break;
                default:
                    aprintf("atom %s not handled\n",fourCC::tostringBE(son.getFCC()));
                    break;
            }
        }
        son.skipAtom();
    }
    delete moov;
    if(!success)
    {
        if(!ADM_mp4SimpleSearchAtom(tom, ADM_MP4_MOOF,&moof))
        {
            ADM_info("Cannot find all needed atoms\n");
        }else
        {
            ADM_info("It is a Dash/fragmented file\n");
            _flavor=Mp4Dash;
            int moofFound=1;
            while(1)
            {
                parseMoof(*moof);
                delete moof;
                moof=NULL;
                if(!ADM_mp4SimpleSearchAtom(tom, ADM_MP4_MOOF,&moof))
                {
                    break;
                }
                moofFound++;
            }
            ADM_info("Found %d fragments\n",moofFound);
        }
    }

    ADM_info("Done finding main atoms\n");
    return success;
}
/**
    \fn parseMvhd
    \brief Parse mvhd header
*/
void MP4Header::parseMvhd(void *ztom)
{
    adm_atom *tom = (adm_atom*)ztom;
    int version = tom->read();

    tom->skipBytes(3); // flags

    if (version == 1)
        tom->skipBytes(16);
    else
        tom->skipBytes(8);

    int scale = tom->read32();
    uint64_t duration = (version == 1) ? tom->read64() : tom->read32();

    _movieScale = scale;

    ADM_info("Warning: movie scale is %d\n", (int)_movieScale);

    if (_movieScale)
    {
        duration = 1000 * duration; // In ms
        duration /= _movieScale;
    }
    else
        _movieScale = 1000;

    //printf("Movie duration: %s\n", ms2timedisplay(duration));
    _videoScale=_movieScale;
    _tracks[0].scale=_videoScale;
    _movieDuration = duration;
}

/**
    \fn parseTrex
    \brief Some iso5 files specify dts increment via trex box only.
*/
uint8_t MP4Header::parseTrex(void *ztom)
{
    adm_atom *tom=(adm_atom *)ztom;
    ADMAtoms id;
    uint32_t container;
    uint32_t trackId=0;
    while(!tom->isDone())
    {
        adm_atom son(tom);
        if(!ADM_mp4SearchAtomName(son.getFCC(),&id,&container))
        {
            aprintf("[parseTrex] Unknown atom %s\n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
            continue;
        }
        if(id!=ADM_MP4_TREX) continue;
        if(nbTrex>=_3GP_MAX_TRACKS)
        {
            ADM_warning("Number of trex boxes exceeds max supported.\n");
            nbTrex=_3GP_MAX_TRACKS;
            break;
        }
        mp4TrexInfo *trx=new mp4TrexInfo;
        ADM_info("Found trex, reading it.\n");
        son.skipBytes(4); // version and flags
        trx->trackID=son.read32();
        trx->sampleDesc=son.read32(); // stsd id
        trx->defaultDuration=son.read32();
        trx->defaultSize=son.read32();
        trx->defaultFlags=son.read32();
#define DUMPX(a) printf("trex %u: "#a" = %u\n",nbTrex,trx->a);
        DUMPX(trackID)
        DUMPX(sampleDesc)
        DUMPX(defaultDuration)
        DUMPX(defaultSize)
        DUMPX(defaultFlags)
        _trexData[nbTrex++]=trx;
        son.skipAtom();
        return 1;
    }
    ADM_info("trex box not found.\n");
    return 0;
}
/**
      \fn parseTrack
      \brief Parse track header
*/
uint8_t MP4Header::parseTrack(void *ztom)
{
    adm_atom *tom=(adm_atom *)ztom;
    ADMAtoms id;
    uint32_t container;
    uint32_t w,h;
    uint32_t trackType=TRACK_OTHER;
    uint32_t trackId=0;
    _currentDelay=0;
    _currentStartOffset=0;

    ADM_info("Parsing Track\n");
    while(!tom->isDone())
    {
        adm_atom son(tom);
        if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
        {
            aprintf("[Track]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
            continue;
        }
        aprintf("\tProcessing atom %s \n",fourCC::tostringBE(son.getFCC()));
        switch(id)
        {
            case ADM_MP4_TKHD:
            {
                int version = son.read();

                son.skipBytes(3);

                if(version == 1)
                    tom->skipBytes(16);
                else
                    tom->skipBytes(8);

                trackId = son.read32();
                aprintf("[parseTrack] Track Id: %" PRIu32"\n", trackId);
                son.skipBytes(4);

                uint64_t duration = (version == 1) ? son.read64() : son.read32();

                aprintf( "Duration: %" PRIu32" (ms)\n", (duration * 1000) / _videoScale);
                son.skipBytes(8);
                son.skipBytes(8);
                son.skipBytes(36);

                w = son.read32() >> 16;
                h = son.read32() >> 16;

                aprintf("tkhd: %ld %ld\n", w, h);
                break;
            }
            case ADM_MP4_MDIA:
            {
                bool skipTrack=!!_videoFound;
                if(!parseMdia(&son,&trackType,&trackId))
                    return false;
                if(trackType==TRACK_VIDEO && skipTrack)
                {
                    ADM_warning("Skipping video track %u\n",trackId);
                    tom->skipAtom();
                    return 1;
                }
                break;
            }
            case ADM_MP4_EDTS:
            {
                ADM_info("EDTS atom found\n");
                parseEdts(&son,trackType);
                break;
            }
            default:
                ADM_info("Unprocessed atom :%s\n",fourCC::tostringBE(son.getFCC()));
        }
        son.skipAtom();
    }
    return 1;
}
/**
      \fn parseMdia
      \brief Parse mdia header
*/
uint8_t MP4Header::parseMdia(void *ztom,uint32_t *trackType,uint32_t *trackId)
{
    adm_atom *tom=(adm_atom *)ztom;
    ADMAtoms id;
    uint32_t container;
    uint32_t trackScale=_videoScale;
    uint64_t trackDuration=0;
    *trackType=TRACK_OTHER;
    std::string language = ADM_UNKNOWN_LANGUAGE;
    uint8_t r=0;

    ADM_info("<<Parsing Mdia>>\n");

    while(!tom->isDone())
    {
        adm_atom son(tom);
        if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
        {
            aprintf("[MDIA]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
            continue;
        }
        switch(id)
        {
            case ADM_MP4_MDHD:
            {
                int version = son.read();

                son.skipBytes(3); // flags

                if(version == 1)
                    son.skipBytes(16);
                else
                    son.skipBytes(8);

                trackScale = son.read32();

                aprintf( "MDHD, Trackscale in mdhd: %u\n", trackScale);

                if(!trackScale)
                    trackScale = 600; // default

                uint64_t duration = (version == 1) ? son.read64() : son.read32();

                aprintf( "MDHD, duration in mdhd: %u (unscaled)\n", duration);
                duration = (duration * 1000.) / trackScale;
                aprintf( "MDHD, duration in mdhd: %u (scaled ms)\n", duration);
                trackDuration = duration;
                //printf("MDHD, Track duration: %s, trackScale: %u\n", ms2timedisplay((1000 * duration) / trackScale), trackScale);
                uint16_t langcode = son.read16();
                language = langCodeToIso639(langcode);
                printf("[mdhd] Language: %s (code: %d)\n",language.c_str(),langcode);

                break;
            }
            case ADM_MP4_HDLR:
            {
                uint32_t type;

                son.read32();
                son.read32();
                type=son.read32();
                printf("[HDLR]\n");
                switch(type)
                {
                    default:
                        *trackType=TRACK_OTHER;
                        ADM_info("Found other type track\n");
                        break;
                    case MKFCCR('v','i','d','e'): // 'vide'
                        *trackType=TRACK_VIDEO;
                        if(_videoFound)
                        { // Ignore all subsequent video tracks, but keep track type set so that the caller can do the same.
                            ADM_warning("Multiple video tracks are not supported, skipping.\n");
                            tom->skipAtom();
                            return 1;
                        }
                        _tracks[0].delay=_currentDelay;
                        _tracks[0].startOffset=_currentStartOffset;
                        ADM_info("hdlr video found \n ");
                        _movieDuration=trackDuration;
                        _videoScale=trackScale;
                        _tracks[0].scale=_videoScale;
                        _tracks[0].id=*trackId;
                        _tracks[0].language = language;
                        break;
                    case MKFCCR('s','o','u','n'): // 'soun'
                        _tracks[1+nbAudioTrack].delay=_currentDelay;
                        _tracks[1+nbAudioTrack].startOffset=_currentStartOffset;
                        _tracks[1+nbAudioTrack].language = language;
                        _tracks[1+nbAudioTrack].id=*trackId;
                        if(!*trackId)
                            ADM_warning("Invalid track ID for audio track %d\n",1+nbAudioTrack);
                        *trackType=TRACK_AUDIO;
                        ADM_info("hdlr audio found \n ");
                        break;
                    case MKFCCR('u','r','l',' '): // 'url '
                    {
                        int s;
                        son.read32();
                        son.read32();
                        son.read32();
                        s=son.read();
                        char *str=new char[s+1];
                        son.readPayload((uint8_t *)str,s);
                        str[s]=0;
                        ADM_info("Url : <%s>\n",str);
                        delete [] str;
                        break;
                    }
                }
                break;
            }
            case ADM_MP4_MINF:
            {
                // We are only interested in stbl
                while(!son.isDone())
                {
                    adm_atom grandson(&son);
                    if(!ADM_mp4SearchAtomName(grandson.getFCC(), &id,&container))
                    {
                        aprintf("[MINF]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
                        grandson.skipAtom();
                        continue;
                    }
                    if(id==ADM_MP4_STBL)
                    {
                        if(!parseStbl(&grandson,*trackType,trackScale))
                        {
                            ADM_info("STBL failed\n");
                            return 0;
                        }
                        r=1;
                    }
                    grandson.skipAtom();
                }
                break;
            }
            default:
                aprintf("** atom  NOT HANDLED [%s] \n",fourCC::tostringBE(son.getFCC()));
        }
        son.skipAtom();
    }
    return r;
}
/**
 * \fn parseElst
 * \brief Parse edit list atom. We manage only one case : when video does not start at 
 *              0, we delay all others tracks by the amount indicated
 * @param tom
 * @return 
 */
uint8_t MP4Header::parseElst(void *ztom, int64_t *delay, int64_t *skip)
{
    uint32_t playbackSpeed;
    adm_atom *tom=(adm_atom *)ztom;
    int version=tom->read();
    tom->skipBytes(3);
    uint32_t nb=tom->read32();
    int64_t *editDuration=new int64_t[nb];
    int64_t *mediaTime=new int64_t[nb];
    int64_t dlay=0;
    int64_t adv=0;
    
    ADM_info("[ELST] Found %" PRIu32" entries in list, version=%d\n",nb,version);
    for(int i=0;i<nb;i++)
    {
        if(1==version)
        {
            editDuration[i]=(int64_t)tom->read64();
            mediaTime[i]=(int64_t)tom->read64();
        }else
        {
            editDuration[i]=(int32_t)tom->read32();
            mediaTime[i]=(int32_t)tom->read32();
        }
            playbackSpeed=tom->read32();
            ADM_info("Duration : %d, mediaTime:%d speed=%d \n",(int)editDuration[i],(int)mediaTime[i],(int)playbackSpeed);
    }

    switch(nb)
    {
        case 1:
            if(mediaTime[0]>0)
            {
                adv=mediaTime[0];
            }
            break;
        case 2:
            if(mediaTime[0]==-1)
            {
                dlay=editDuration[0];
                adv=mediaTime[1];
            }
            break;
        default:break;
    }
    ADM_info("delay = %" PRId64" in movie scale units, skip to time %" PRId64" in track scale units.\n",dlay,adv);
    
    delete [] editDuration;
    delete [] mediaTime;
    *delay=dlay;
    *skip=adv;
    return 1;
}
/**
        \fn parseEdts
        \brief parse sample table. this is the most important function.
*/
uint8_t MP4Header::parseEdts(void *ztom,uint32_t trackType)
{
    adm_atom *tom=(adm_atom *)ztom;
    ADMAtoms id;
    uint32_t container;

    ADM_info("Parsing Edts, trackType=%d\n",trackType);
    while(!tom->isDone())
    {
        adm_atom son(tom);
        if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
        {
            aprintf("[EDTS]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
            continue;
        }
        switch(id)
        {
            case ADM_MP4_ELST:
            {
                ADM_info("ELST atom found\n");
                parseElst(&son,&_currentDelay,&_currentStartOffset);
                son.skipAtom();
                break;
            }
            default:
                aprintf("** atom  NOT HANDLED [%s] \n",fourCC::tostringBE(son.getFCC()));
        }
    }

    tom->skipAtom();
    return 1;
}
/**
        \fn parseStbl
        \brief parse sample table. this is the most important function.
*/
uint8_t MP4Header::parseStbl(void *ztom,uint32_t trackType,uint32_t trackScale)
{
    adm_atom *tom=(adm_atom *)ztom;
    ADMAtoms id;
    uint32_t container;
    MPsampleinfo info;

    ADM_info("<<Parsing Stbl>>\n");
    while(!tom->isDone())
    {
        adm_atom son(tom);
        if(!ADM_mp4SearchAtomName(son.getFCC(), &id,&container))
        {
            aprintf("[STBL]Found atom %s unknown\n",fourCC::tostringBE(son.getFCC()));
            son.skipAtom();
            continue;
        }
        switch(id)
        {
            case ADM_MP4_STSS: // Sync sample atom (i.e. keyframes)
            {
                son.read32();
                info.nbSync=son.read32();
                ADM_info("Stss:%u\n",info.nbSync);
                if(info.nbSync)
                {
                    info.Sync=new uint32_t[info.nbSync];
                    for(int i=0;i<info.nbSync;i++)
                    {
                        info.Sync[i]=son.read32();
                    }
                }
                break;
            }
            case ADM_MP4_STTS:
            {
                ADM_info("stts:%" PRIu32"\n",son.read32()); // version & flags
                info.nbStts=son.read32();
                ADM_info("Time stts atom found (%" PRIu32")\n",info.nbStts);
                ADM_info("Using myscale %" PRIu32"\n",trackScale);
                info.SttsN=new uint32_t[info.nbStts];
                info.SttsC=new uint32_t[info.nbStts];
                //double dur;
                for(int i=0;i<info.nbStts;i++)
                {
                    info.SttsN[i]=son.read32();
                    info.SttsC[i]=son.read32();
                    aprintf("stts: count:%u size:%u (unscaled)\n",info.SttsN[i],info.SttsC[i]);
                    //dur*=1000.*1000.;; // us
                    //dur/=myScale;
                }
                break;
            }
            case ADM_MP4_STSC:
            {
                son.read32();
                info.nbSc=son.read32();
                info.Sc=new uint32_t[info.nbSc];
                info.Sn=new uint32_t[info.nbSc];
                for(int j=0;j<info.nbSc;j++)
                {
                    info.Sc[j]=son.read32();
                    info.Sn[j]=son.read32();
                    son.read32();
                    aprintf("\t sc  %d : sc start:%u sc count: %u\n",j,info.Sc[j],info.Sn[j]);
                }
                break;
            }
            case ADM_MP4_STSZ:
            {
                uint32_t n;
                son.read32();
                n=son.read32();
                info.nbSz=son.read32();
                info.SzIndentical=0;
                ADM_info("%" PRIu32" frames /%" PRIu32" nbsz..\n",n,info.nbSz);
                if(n)
                {
                    aprintf("\t\t%" PRIu32" frames of the same size %" PRIu32" , n=%" PRIu32"\n",
                    info.nbSz,info.SzIndentical,n);
                    info.SzIndentical=n;
                    info.Sz=NULL;
                }else
                {
                    info.Sz=new uint32_t[info.nbSz];
                    for(int j=0;j<info.nbSz;j++)
                    {
                        info.Sz[j]=son.read32();
                    }
                }
                break;
            }
            case ADM_MP4_CTTS: // Composition time to sample
            {
                uint32_t n,i,k;

                ADM_info("ctts:%" PRIu32"\n",son.read32()); // version & flags
                n=son.read32();
                if(n==1) // all the same , ignore
                {
                    break;
                }
                aprintf("Found %d ctts elements\n",n);
                uint32_t *values=new uint32_t [n];
                uint32_t *count=new uint32_t [n];
                for(i=0;i<n;i++)
                {
                    count[i]=son.read32();
                    values[i]=son.read32();
                }
                uint32_t sum=0;
                for(i=0;i<n;i++)
                {
                    sum+=count[i];
                }
                info.Ctts=new uint32_t[sum+1]; // keep a safe margin

                for(i=0;i<n;i++)
                {
                    if(i<20)
                    {
                        aprintf("Ctts: nb: %u (%x) val:%u (%x)\n",count[i],count[i],values[i],values[i]);
                    }
                    for(k=0;k<count[i];k++)
                    {
                        info.Ctts[info.nbCtts++]=values[i];
                    }
                }
                delete [] values;
                delete [] count;
                if(!info.nbCtts)
                {
                    delete [] info.Ctts;
                    info.Ctts=NULL;
                    ADM_info("Destroying Ctts, seems invalid\n");
                }
                ADM_assert(info.nbCtts<sum+1);
                ADM_info("Found %u elements\n",info.nbCtts);
                break;
            }
            case ADM_MP4_STCO:
            {
                son.skipBytes(4);

                info.nbCo = son.read32();
                ADM_info("\t\tnbCo: %u\n", info.nbCo);

                info.Co = new uint64_t[info.nbCo];

                for(int j = 0; j < info.nbCo; j++)
                {
                    info.Co[j] = son.read32();
                    aprintf( "Chunk offset: %u / %u : %" PRIu64"\n", j, info.nbCo - 1, info.Co[j]);
                }
                break;
            }
            case ADM_MP4_STCO64:
            {
                son.skipBytes(4);

                info.nbCo = son.read32();
                ADM_info("\t\tnbCo: %u\n", info.nbCo);

                info.Co = new uint64_t[info.nbCo];

                for(int j = 0; j< info.nbCo; j++)
                {
                    info.Co[j] = son.read64();
                    aprintf( "Chunk offset: %u / %u : %" PRIu64"\n", j, info.nbCo - 1, info.Co[j]);
                }
                break;
            }
            case ADM_MP4_STSD:
            {
                son.read32(); // flags & version
                int nbEntries=son.read32();
                int left;
                ADM_info("[STSD] Number of entries: %d\n",nbEntries);
                for(int i=0;i<nbEntries;i++)
                {
                    int entrySize=son.read32();
                    int entryName=son.read32();
                    left=entrySize-8;
                    if(i || (trackType==TRACK_VIDEO && _videoFound) || (trackType==TRACK_OTHER))
                    {
                        son.skipBytes(left);
                        ADM_info("[STSD] ignoring %s, size %u\n",fourCC::tostringBE(entryName),entrySize);
                        if(trackType==TRACK_OTHER) ADM_info("[STSD] because track=other\n");
                            continue;
                    }
                    switch(trackType)
                    {
                        case TRACK_VIDEO:
                        {
                            uint32_t lw=0,lh=0;
                            ADM_info("[STSD] VIDEO %s, size %u\n",fourCC::tostringBE(entryName),entrySize);
                            son.skipBytes(8); // reserved etc..
                            left-=8;
                            son.read32(); // version/revision
                            left-=4;
                            printf("[STSD] vendor %s\n",fourCC::tostringBE(son.read32()));
                            left-=4;

                            son.skipBytes(8); // spatial qual etc..
                            left-=8;

                            printf("[STSD] width :%u\n",lw=son.read16());
                            printf("[STSD] height :%u\n",lh=son.read16());
                            left-=4;

                            son.skipBytes(8); // Resolution
                            left-=8;

                            printf("[STSD] datasize :%u\n",son.read32());
                            left-=4;

                            printf("[STSD] FrameCount :%u\n",son.read16());
                            left-=2;

                            // Codec name
                            uint32_t u32=son.read();
                            if(u32>31) u32=31;
                            printf("Codec string :%d <",u32);
                            for(int i=0;i<u32;i++) printf("%c",son.read());
                            printf(">\n");
                            son.skipBytes(32-1-u32);
                            left-=32;

                            if(left>=4)
                            {
                                son.read32();
                                left-=4; // Depth & color Id
                            }

                            printf("LEFT:%d\n",left);

                            //if(left>8)
                            //    decodeVideoAtom(&son);

                            _video_bih.biWidth=_mainaviheader.dwWidth=lw ;
                            _video_bih.biHeight=_mainaviheader.dwHeight=lh;
                            _video_bih.biCompression=_videostream.fccHandler;

#define commonPart(x) _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)#x);
                            switch(entryName)
                            {
                                case MKFCCR('h','d','v','5'): // hdv5
                                {
                                    commonPart(MPEG);
                                    adm_atom hdv5(&son);
                                    printf("Reading hdv5, got %s\n",fourCC::tostringBE(hdv5.getFCC()));
                                    left=0;

                                    break;
                                }
                                case MKFCCR('m','j','p','b'):  //mjpegb
                                {
                                    commonPart(MJPB);
                                    left=0;

                                    break;
                                }
                                case MKFCCR('S','V','Q','1'):  //mjpegb
                                {
                                    commonPart(SVQ1);
                                    left=0;

                                    break;
                                }
                                case MKFCCR('m','j','p','a'):  //mjpegb
                                {
                                    commonPart(MJPG);
                                    left=0;

                                    break;
                                }
                                case MKFCCR('s','2','6','3'):  //s263 d263
                                {
                                    commonPart(H263);
                                    adm_atom d263(&son);
                                    printf("Reading s253, got %s\n",fourCC::tostringBE(d263.getFCC()));
                                    left=0;

                                    break;
                                }
                                case MKFCCR('m','p','4','v'):  //mp4v
                                {
                                    commonPart(DIVX);
                                    adm_atom esds(&son);
                                    printf("Reading esds, got %s\n",fourCC::tostringBE(esds.getFCC()));
                                    if(esds.getFCC()==MKFCCR('e','s','d','s'))
                                        decodeEsds(&esds,TRACK_VIDEO);
                                    left=0;

                                    break;
                                }
                                case MKFCCR('S','V','Q','3'):
                                {   //'SVQ3':
                                    // For SVQ3, the codec needs it to begin by SVQ3
                                    // We go back by 4 bytes to get the 4CC
                                    printf("SVQ3 atom found\n");
                                    VDEO.extraDataSize=left+4;
                                    VDEO.extraData=new uint8_t[ VDEO.extraDataSize ];
                                    if(!son.readPayload(VDEO.extraData+4,VDEO.extraDataSize-4 ))
                                    {
                                        GUI_Error_HIG(QT_TRANSLATE_NOOP("mp4demuxer","Problem reading SVQ3 headers"), NULL);
                                    }
                                    VDEO.extraData[0]='S';
                                    VDEO.extraData[1]='V';
                                    VDEO.extraData[2]='Q';
                                    VDEO.extraData[3]='3';
                                    printf("SVQ3 Header size : %" PRIu32"",_videoExtraLen);
                                    commonPart(SVQ3);
                                    left=0;

                                    break;
                                }
                                case MKFCCR('d','v','c',' '): // 'dvc '
                                case MKFCCR('d','v','c','p'): // 'dvcp'
                                    commonPart(DVSD);
                                    break;

                                case MKFCCR('c','v','i','d'): // 'cvid'
                                    commonPart(cvid);
                                    break;

                                case MKFCCR('h','2','6','3'): // 'dv':
                                    commonPart(H263);
                                    break;

                                case MKFCCR('M','J','P','G'): // 'jpeg'
                                case MKFCCR('j','p','e','g'): // 'jpeg'
                                case MKFCCR('A','V','D','J'): // 'jpeg'
                                    commonPart(MJPG);
                                    break;

                                case MKFCCR('A','V','d','n'): // DNxHD
                                    commonPart(AVdn);
                                    break;

                                case MKFCCR('a','p','c','h'): // ProRes 422 High Quality
                                    commonPart(apch);
                                    break;

                                case MKFCCR('a','p','c','n'): // ProRes 422 Standard Definition
                                    commonPart(apcn);
                                    break;

                                case MKFCCR('a','p','c','s'): // ProRes 422 LT
                                    commonPart(apcs);
                                    break;

                                case MKFCCR('a','p','c','o'): // ProRes 422 Proxy
                                    commonPart(apcs);
                                    break;

                                case MKFCCR('a','p','4','h'): // ProRes 4444
                                    commonPart(ap4h);
                                    break;
//
                                case MKFCCR('h','e','v','1'): // hev1 / hevc
                                case MKFCCR('h','v','c','1'): // hev1 / hevc
                                {
                                    commonPart(H265);
                                    while(!son.isDone())
                                    {
                                        adm_atom avcc(&son);
                                        const char *hevc = (entryName == MKFCCR('h','e','v','1'))? "hev1" : "hvc1";
                                        printf("Reading %s, got %s\n",hevc,fourCC::tostringBE(avcc.getFCC()));
                                        if( avcc.getFCC()== MKFCCR('h','v','c','C'))
                                        {
                                            VDEO.extraDataSize=avcc.getRemainingSize();
                                            ADM_info("Found %d bytes of extradata \n",VDEO.extraDataSize);
                                            VDEO.extraData=new uint8_t [VDEO.extraDataSize];
                                            avcc.readPayload(VDEO.extraData,VDEO.extraDataSize);
                                            mixDump(VDEO.extraData,VDEO.extraDataSize);
                                            avcc.skipAtom();
                                        }
                                    } // while
                                    son.skipAtom();
                                    left=0;

                                    break;
                                }
//
                                case MKFCCR('a','v','c','1'): // avc1
                                case MKFCCR('a','v','c','3'): // avc3
                                {
                                    commonPart(H264);
                                    // There is a avcC atom just after
                                    // configuration data for h264
                                    while(!son.isDone())
                                    {
                                        adm_atom avcc(&son);
                                        printf("Reading avcC, got %s\n",fourCC::tostringBE(avcc.getFCC()));
                                        if(avcc.getFCC()!=MKFCCR('a','v','c','C'))
                                        {
                                            avcc.skipAtom();
                                            continue;
                                        }

                                        int len,offset;
                                        VDEO.extraDataSize=avcc.getRemainingSize();
                                        VDEO.extraData=new uint8_t [VDEO.extraDataSize];
                                        avcc.readPayload(VDEO.extraData,VDEO.extraDataSize);
                                        printf("avcC size:%d\n",VDEO.extraDataSize);
                                        // Dump some info
#define MKD8(x) VDEO.extraData[x]
#define MKD16(x) ((MKD8(x)<<8)+MKD8(x+1))
#define MKD32(x) ((MKD16(x)<<16)+MKD16(x+2))

                                        printf("avcC Revision             :%x\n", MKD8(0));
                                        printf("avcC AVCProfileIndication :%x\n", MKD8(1));
                                        printf("avcC profile_compatibility:%x\n", MKD8(2));
                                        printf("avcC AVCLevelIndication   :%x\n", MKD8(3));

                                        printf("avcC lengthSizeMinusOne   :%x\n", MKD8(4));
                                        printf("avcC NumSeq               :%x\n", MKD8(5));
                                        len=MKD16(6);
                                        printf("avcC sequenceParSetLen    :%x ",len );
                                        offset=8;
                                        mixDump(VDEO.extraData+offset,len);

                                        offset=8+len;
                                        printf("avcC numOfPictureParSets  :%x\n", MKD8(offset++));
                                        len=MKD16(offset);
                                        offset++;
                                        printf("avcC Pic len              :%x\n",len);
                                        mixDump(VDEO.extraData+offset,len);
                                        // Verify width and height, the values from the container may be wrong.
                                        ADM_SPSInfo sps;
                                        if(false==extractSPSInfo(VDEO.extraData,VDEO.extraDataSize,&sps))
                                        {
                                            ADM_warning("Could not decode H.264 extradata.\n");
                                            break;
                                        }
                                        if(sps.width && sps.height)
                                        {
                                            if(lw && lw!=sps.width)
                                                ADM_warning("Width mismatch, container says %u, codec %u, trusting codec\n",lw,sps.width);
                                            _video_bih.biWidth=_mainaviheader.dwWidth=sps.width;
                                            if(lh && lh!=sps.height)
                                                ADM_warning("Height mismatch, container says %u, codec %u, trusting codec\n",lh,sps.height);
                                            _video_bih.biHeight=_mainaviheader.dwHeight=sps.height;
                                        }else
                                            ADM_warning("Got invalid dimensions from SPS, cannot verify video width and height.\n");
                                        // Prefill time base from fps1000 value we got from SPS, we handle only standard cases here.
                                        switch(sps.fps1000)
                                        {
                                            case 23976:
                                                _videostream.dwScale=1001;
                                                _videostream.dwRate=24000;
                                                break;
                                            case 29970:
                                                _videostream.dwScale=1001;
                                                _videostream.dwRate=30000;
                                                break;
                                            case 24000: case 25000: case 30000: case 50000: case 60000:
                                                _videostream.dwScale=1000;
                                                _videostream.dwRate=sps.fps1000;
                                                break;
                                            default:break;
                                        }
                                        break;
                                    } // while
                                    son.skipAtom();
                                    left=0;

                                    break;
                                } // avc1

                                case MKFCCR('a','v','0','1'): // AV1
                                    commonPart(av01);
                                    while(!son.isDone())
                                    {
                                        adm_atom av1c(&son);
                                        if(av1c.getFCC()== MKFCCR('a','v','1','C'))
                                        {
                                            uint32_t len=av1c.getRemainingSize();
#define AV1_EXTRADATA_OFFSET 4
                                            if(len>AV1_EXTRADATA_OFFSET)
                                            {
                                                VDEO.extraDataSize=len-AV1_EXTRADATA_OFFSET;
                                                ADM_info("Found %d bytes of AV1 extradata\n",VDEO.extraDataSize);
                                                uint8_t *tmp=new uint8_t[len];
                                                av1c.readPayload(tmp,len);
                                                VDEO.extraData=new uint8_t[VDEO.extraDataSize];
                                                memcpy(VDEO.extraData, tmp+AV1_EXTRADATA_OFFSET, VDEO.extraDataSize);
                                                delete [] tmp;
                                                tmp=NULL;
                                                mixDump(VDEO.extraData,VDEO.extraDataSize);
                                            }
                                            av1c.skipAtom();
                                        }
                                    }
                                    son.skipAtom();
                                    left=0;
                                    break;

                                case MKFCCR('v','p','0','9'): // VP9
                                    _videostream.fccHandler = _video_bih.biCompression = fourCC::get((uint8_t *)"VP9 ");
                                    while(!son.isDone())
                                    {
                                        adm_atom vpc(&son);
                                        if(vpc.getFCC() != MKFCCR('v','p','c','C'))
                                        {
                                            vpc.skipAtom();
                                            continue;
                                        }
                                        int64_t len = vpc.getRemainingSize();
#define VPCC_SIZE 12
                                        if(len != VPCC_SIZE)
                                            ADM_warning("Unexpected vpcC size %" PRId64" instead of %u\n",len,VPCC_SIZE);
                                        else
                                        {
                                            uint8_t *tmp = new uint8_t[VPCC_SIZE];
                                            if(!vpc.readPayload(tmp,VPCC_SIZE))
                                            {
                                                delete [] tmp;
                                                tmp = NULL;
                                                ADM_warning("Cannot read vpcC.\n");
                                                vpc.skipAtom();
                                                continue;
                                            }

                                            ADM_info("Raw vpcC:\n");
                                            mixDump(tmp,VPCC_SIZE);

                                            uint8_t *p = tmp;
                                            uint8_t version = *p++;
                                            if(version != 1)
                                                ADM_warning("Unsupported vpcC version, expected 1, got %u\n",version);
                                            else
                                            {
                                                p += 3+1+1; // flags, profile and level
                                                _videoColRange = (*p++ & 1) ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;
                                                _videoColPrimaries = *p++;
                                                _videoColTransferCharacteristic = *p++;
                                                _videoColMatrixCoefficients = *p;
                                                _videoColFlags = ADM_COL_FLAG_RANGE_SET | ADM_COL_FLAG_PRIMARIES_SET | ADM_COL_FLAG_TRANSFER_SET | ADM_COL_FLAG_MATRIX_COEFF_SET;
                                                ADM_info("Color properties from vpcC:\n\trange:\t\t%u\n\tprimaries:\t%u\n\ttransfer:\t%u\n\tmcoeff:\t\t%u\n",
                                                    _videoColRange, _videoColPrimaries, _videoColTransferCharacteristic, _videoColMatrixCoefficients);
                                            }
                                            delete [] tmp;
                                            tmp = NULL;
                                            vpc.skipAtom();
                                        }
                                    }
                                    son.skipAtom();
                                    left = 0;
                                    break;

                                default:
                                    if(left>10)
                                    {
                                        adm_atom avcc(&son);
                                        printf("Reading , got %s\n",fourCC::tostringBE(avcc.getFCC()));
                                        left=0;
                                    }
                                    break;
                            } // entryName
                            break;
                        } // TRACK_VIDEO

                        case TRACK_AUDIO:
                        {
                            uint32_t channels,bpp,fq,packSize;
                            uint32_t lpcmFlags = 0;

                            // Put some defaults
                            ADIO.encoding=WAV_UNKNOWN;
                            ADIO.frequency=44100;
                            ADIO.byterate=AUDIO_BYTERATE_UNSET;
                            ADIO.channels=2;
                            ADIO.bitspersample=16;

                            printf("[STSD] AUDIO <%s>, 0x%08x, size %u\n",fourCC::tostringBE(entryName),entryName,entrySize);
                            son.skipBytes(8); // reserved etc..
                            left-=8;

                            int atomVersion=son.read16(); // version
                            left-=2;
                            if(atomVersion > 2)
                            {
                                ADM_warning("[stsd] Unsupported audio sample description version %d, skipping track.\n",atomVersion);
                                break;
                            }
                            printf("[STSD]Revision       :%d\n",atomVersion);
                            son.skipBytes(2); // Revision level
                            left-=2;

                            printf("[STSD]Vendor         : %s\n",fourCC::tostringBE(son.read32()));
                            left-=4;

                            channels = son.read16(); // Number of channels when version < 2
                            left-=2;
                            if(atomVersion < 2)
                            {
                                printf("[STSD]Channels       :%d\n",channels);
                                ADIO.channels = channels;
                            }else if(channels != 3)
                                ADM_warning("[stsd] Audio sample description version 2, but always3 = %u\n",channels);

                            bpp = son.read16(); // Sample Size
                            left-=2;
                            printf("[STSD]Bit per sample :%d\n",bpp);
                            if(atomVersion < 2)
                                ADIO.bitspersample = bpp;

                            son.read16(); // Compression ID
                            left-=2;

                            son.read16(); // Packet Size
                            left-=2;

                            fq = son.read16();
                            if(atomVersion < 2)
                            {
                                printf("[STSD]Fq:%u\n",fq);
                                if(fq != trackScale)
                                    ADM_warning("[stsd] Sampling rate in audio sample description doesn't match track scale!\n");
                                if(fq < MIN_SAMPLING_RATE)
                                {
                                    ADM_warning("[stsd] Sampling rate below minimum supported, making something up.\n");
                                    fq = 48000;
                                }
                                ADIO.frequency = fq;
                                printf("[STSD]Fq       :%d\n",ADIO.frequency); // Sample rate
                            }
                            son.skipBytes(2); // Fixed point
                            left-=4;
                            printf("Bytes left : %d\n",left);
                            info.samplePerPacket=1;
                            info.bytePerPacket=1;
                            info.bytePerFrame=1;

                            switch(atomVersion)
                            {
                                case 0:break;
                                case 1 :
                                {
                                    info.samplePerPacket=son.read32();
                                    info.bytePerPacket=son.read32();
                                    info.bytePerFrame=son.read32();
#define ADM_NOT_NULL(x) if(!info.x) info.x=1;
                                    printf("[STSD] Sample per packet %u\n",info.samplePerPacket);
                                    printf("[STSD] Bytes per packet  %u\n",info.bytePerPacket);
                                    printf("[STSD] Bytes per frame   %u\n",info.bytePerFrame);
                                    printf("[STSD] Bytes per sample   %u\n",son.read32());
                                    ADM_NOT_NULL(samplePerPacket);
                                    ADM_NOT_NULL(bytePerPacket);
                                    ADM_NOT_NULL(bytePerFrame);

                                    left-=16;

                                    break;
                                }
                                case 2:
                                {
                                    printf("v2.0 sizeOfStructOnly = %u\n",son.read32());
                                    union { uint64_t i; double d; } freq64;
                                    freq64.i = son.read64();
                                    ADIO.frequency = fq = (((uint64_t)freq64.d) & 0xFFFFFFFF);
#define MARK(x) {uint32_t v=son.read32(); printf(#x " => %d\n",v);x=v;}
                                    MARK(channels);
                                    printf("0x7f000 = 0x%x\n",son.read32());
                                    MARK(bpp);
                                    lpcmFlags = son.read32();
                                    printf("LPCM flags= %d\n",lpcmFlags);
                                    printf("byte per frame = %d\n",son.read32());
                                    printf("sample per frame = %d\n",son.read32());
                                    ADIO.bitspersample=bpp;
                                    ADIO.channels=channels;
                                    info.bytePerPacket=bpp/8;
                                    info.bytePerFrame=info.bytePerPacket*ADIO.channels;
                                    left-=4*9;

                                    break;
                                }
                            } // atomVersion
                            printf("[STSD] chan:%u bpp:%u fq:%u (left %u)\n",channels,bpp,fq,left);
#define audioCodec(x) ADIO.encoding=WAV_##x;
                            switch(entryName)
                            {
                                case MKFCCR('a','c','-','3'):
                                case MKFCCR('s','a','c','3'):
                                    audioCodec(AC3);

                                    break;
                                case MKFCCR('e','c','-','3'):
                                    audioCodec(EAC3);

                                    break;
                                case MKFCCR('l','p','c','m'):
                                    ADIO.byterate=ADIO.frequency*ADIO.bitspersample*ADIO.channels/8;
                                    ADIO.encoding = (lpcmFlags & 2)? WAV_LPCM : WAV_PCM;

                                    break;
                                case MKFCCR('t','w','o','s'):
                                    audioCodec(LPCM);
                                    ADIO.byterate=ADIO.frequency*ADIO.bitspersample*ADIO.channels/8;
                                    if(info.bytePerPacket<2)
                                    {
                                        info.bytePerPacket=2;
                                        info.bytePerFrame=2*ADIO.channels;
                                        ADM_info("[MP4] Overriding bytePer packet with %d\n",info.bytePerPacket);
                                    }

                                    break;
                                case MKFCCR('u','l','a','w'):
                                    audioCodec(ULAW);
                                    ADIO.byterate=ADIO.frequency;
                                    info.bytePerFrame=ADIO.channels;

                                    break;
                                case MKFCCR('s','o','w','t'):
                                    audioCodec(PCM);
                                    ADIO.byterate=ADIO.frequency*ADIO.bitspersample*ADIO.channels/8;
                                    if(info.bytePerPacket<2)
                                    {
                                        info.bytePerPacket=2;
                                        info.bytePerFrame=2*ADIO.channels;
                                        ADM_info("[MP4] Overriding bytePer packet with %d\n",info.bytePerPacket);
                                    }

                                    break;
                                case MKFCCR('.','m','p','3'): //.mp3
                                    audioCodec(MP3);

                                    break;
                                case MKFCCR('r','a','w',' '):
                                    audioCodec(8BITS_UNSIGNED);
                                    ADIO.byterate=ADIO.frequency*ADIO.channels;

                                    break;
                                case MKFCCR('s','a','m','r'):
                                {
                                    audioCodec(AMRNB);
                                    ADIO.frequency=8000;
                                    ADIO.channels=1;
                                    ADIO.bitspersample=16;
                                    ADIO.byterate=12000/8;
                                    if(left>10)
                                    {
                                        adm_atom amr(&son);
                                        printf("Reading wave, got %s\n",fourCC::tostringBE(amr.getFCC()));
                                        left=0;
                                    }

                                    break;
                                }
                                case MKFCCR('Q','D','M','2'):
                                case MKFCCR('O','p','u','s'):
                                {
                                    int64_t sz;
                                    if(MKFCCR('Q','D','M','2') == entryName)
                                    {
                                        audioCodec(QDM2)
                                    }else
                                    {
                                        audioCodec(OPUS)
                                    }
                                    sz=son.getRemainingSize();
                                    _tracks[1+nbAudioTrack].extraDataSize=sz;
                                    _tracks[1+nbAudioTrack].extraData=new uint8_t[sz];
                                    son.readPayload(_tracks[1+nbAudioTrack].extraData,sz);
                                    left=0;

                                    break;
                                }
                                case MKFCCR('m','s',0,0x55): // why 55 ???
                                case MKFCCR('m','s',0,0x11): // why 11 ???
                                case MKFCCR('m','p','4','a'):
                                case MKFCCR('i','n','2','4'):
                                {
                                    if(entryName == MKFCCR('i','n','2','4'))
                                    {
                                        ADIO.bitspersample=24;
                                        ADIO.byterate=ADIO.frequency*ADIO.bitspersample*ADIO.channels/8;
                                        info.bytePerPacket=3;
                                        info.bytePerFrame=3*ADIO.channels;
                                        audioCodec(LPCM)
                                    }else if(entryName == MKFCCR('m','s',0,0x11))
                                        audioCodec(MSADPCM)
                                    else
                                        audioCodec(AAC);

                                    if(left>10)
                                    {
                                        while(!son.isDone())
                                        {
                                            adm_atom wave(&son);
                                            printf("> got %s atom\n",fourCC::tostringBE(wave.getFCC()));
                                            switch(wave.getFCC())
                                            {
                                                case MKFCCR('c','h','a','n'):
                                                    printf("Found channel layout atom, skipping\n");

                                                    break;
                                                case MKFCCR('w','a','v','e'): // wave
                                                {
                                                    while(!wave.isDone())
                                                    {
                                                        adm_atom item(&wave);
                                                        printf("parsing wave, got %s,0x%x\n", fourCC::tostringBE(item.getFCC()), item.getFCC());

                                                        switch(item.getFCC())
                                                        {
                                                            case MKFCCR('f','r','m','a'): // frma
                                                            {
                                                                uint32_t codecid=item.read32();
                                                                printf("frma Codec Id :%s\n",fourCC::tostringBE(codecid));

                                                                break;
                                                            }
                                                            case MKFCCR('e','n','d','a'): // enda
                                                            {
                                                                bool little = item.read16() == 1;
                                                                if(little && ADIO.encoding == WAV_LPCM)
                                                                {
                                                                    printf("Little-endian\n");
                                                                    audioCodec(PCM)
                                                                }
                                                                break;
                                                            }
                                                            case MKFCCR('m','s',0,0x55):
                                                            case MKFCCR('m','s',0,0x11):
                                                            { // We have a waveformat here
                                                                printf("[STSD]Found MS audio header:\n");
                                                                ADIO.encoding=ADM_swap16(item.read16());
                                                                ADIO.channels=ADM_swap16(item.read16());
                                                                ADIO.frequency=ADM_swap32(item.read32());
                                                                ADIO.byterate=ADM_swap32(item.read32());
                                                                ADIO.blockalign=ADM_swap16(item.read16());
                                                                ADIO.bitspersample=ADM_swap16(item.read16());
                                                                printWavHeader(&(ADIO));

                                                                break;
                                                            }
                                                            case MKFCCR('m','p','4','a'): // mp4a
                                                                break;
                                                            case MKFCCR('e','s','d','s'): // esds
                                                            {
                                                                decodeEsds(&item,TRACK_AUDIO);
                                                                break;
                                                            }
                                                            default:break;
                                                        }

                                                        item.skipAtom();

                                                    } // while(!wave.isDone())
                                                    left=0;
                                                    break;
                                                } // case wave
                                                case MKFCCR('e','s','d','s'):
                                                {
                                                    decodeEsds(&wave,TRACK_AUDIO);
                                                    break;
                                                }
                                                default:
                                                    printf("UNHANDLED ATOM : %s\n",fourCC::tostringBE(wave.getFCC()));
                                                    break;
                                            } // switch(wave.getFCC)
                                            wave.skipAtom();
                                        } // while(!son.isDone())
                                    } // if left > 10
                                    left=0;
                                    break;
                                } // mp4a
                            } // entryName

                            // all audio part read for current track, if it is AAC and we have extrdata, check the channels...
                            refineAudio(&(ADIO),_tracks[1+nbAudioTrack].extraDataSize,_tracks[1+nbAudioTrack].extraData);
                            break;
                        } // TRACK_AUDIO
                        default:
                        ADM_assert(0);
                    } // trackType
                    son.skipBytes(left);
                } // for
                break;
            } // ADM_MP4_STSD
            default:
                printf("[STBL]Skipping atom %s\n",fourCC::tostringBE(son.getFCC()));
        } // id
        son.skipAtom();
    } // while
    uint8_t r=0;
    uint32_t nbo=0;
    switch(trackType)
    {
        case TRACK_VIDEO:
        {
            if(_tracks[0].index)
            {
                printf("Already got a video track\n");
                return 1;
            }
            r=indexify(&(_tracks[0]),trackScale,&info,0,&nbo);

            if(!r) return false;

            _videostream.dwLength= _mainaviheader.dwTotalFrames=_tracks[0].nbIndex;
            // update fps
            double f=_videostream.dwLength;
            ADM_info("Movie duration = %d\n",(int)_movieDuration);
            ADM_info("# images = %d\n",(int)_mainaviheader.dwTotalFrames);

            if(_movieDuration) f=1000000.*f/_movieDuration;
            else f=25000;

            ADM_info("Avg fps %f\n",(float)f);

            _videostream.dwRate=trackScale;

            // if we have a sync atom ???
            if(info.nbSync)
            {
                // Mark keyframes
                for(int i=0;i<info.nbSync;i++)
                {
                    uint32_t sync=info.Sync[i];
                    if(sync) sync--;
                    _tracks[0].index[sync].intra=AVI_KEY_FRAME;
                }
            }else
            { // All frames are kf
                for(int i=0;i<_tracks[0].nbIndex;i++)
                {
                    _tracks[0].index[i].intra=AVI_KEY_FRAME;
                }
            }
            // Now do the CTTS thing
            if(info.Ctts)
            {
                updateCtts(&info);
            }else
            { // No ctts, dts=pts
                for(int i=0;i<_videostream.dwLength;i++)
                {
                    _tracks[0].index[i].pts= _tracks[0].index[i].dts;
                }
            }
            VDEO.index[0].intra=AVI_KEY_FRAME;

            break;
        } // TRACK_VIDEO
        case TRACK_AUDIO:
            printf("Cur audio track :%u\n",nbAudioTrack);
#if 0
          if(info.SzIndentical ==1 && (ADIO.encoding==WAV_LPCM || ADIO.encoding==WAV_PCM ))
            {
              printf("Overriding size %" PRIu32" -> %" PRIu32"\n", info.SzIndentical,info.SzIndentical*2*ADIO.channels);
              info.SzIndentical=info.SzIndentical*2*ADIO.channels;
            }


            if(info.SzIndentical ==1 && (ADIO.encoding==WAV_ULAW ))
            {
              printf("Overriding size %" PRIu32" -> %" PRIu32"\n", info.SzIndentical,info.SzIndentical*ADIO.channels);
              info.SzIndentical=info.SzIndentical*ADIO.channels;
            }
#endif
            if(nbAudioTrack>=_3GP_MAX_TRACKS-1)
            {
                ADM_warning("Maximum number of tracks reached, cannot add audio track.\n");
                r=1;
                break;
            }
            r=indexify(&(_tracks[1+nbAudioTrack]),trackScale,&info,1,&nbo);
            ADM_info("Indexed audio, nb blocks:%u\n",nbo);
            nbAudioTrack++;
            _tracks[nbAudioTrack].scale=trackScale;
            if(r)
            {
                nbo=_tracks[nbAudioTrack].nbIndex;
                if(!nbo)
                    _tracks[nbAudioTrack].nbIndex=info.nbSz;
                ADM_info("Indexed audio, nb blocks:%u (final)\n",_tracks[nbAudioTrack].nbIndex);
            }else
            {
                if(_tracks[nbAudioTrack].index)
                {
                    delete [] _tracks[nbAudioTrack].index;
                    _tracks[nbAudioTrack].index=NULL;
                }
                nbAudioTrack--;
            }
            r=1; // don't fail on audio
            break;
        case TRACK_OTHER:
            r=1;
            break;
    }
    return r;
}
/**
      \fn decodeEsds
      \brief Decode esds atom
*/
uint8_t MP4Header::decodeEsds(void *ztom,uint32_t trackType)
{
    adm_atom *tom=(adm_atom *)ztom;
    int tag,l;
    // in case of mpeg4 we only take
    // the mpeg4 vol header
    printf("[MP4]Esds atom found\n");

    tom->skipBytes(4);
    tag=0xff;
    while(tag!=Tag_DecSpecificInfo && !tom->isDone())
    {
        tag=tom->read();
        l=readPackedLen(tom);
        printf("\t Tag : %u Len : %u\n",tag,l);
        switch(tag)
        {
            case Tag_ES_Desc:
                printf("\t ES_Desc\n");
                tom->skipBytes(3);
                break;
            case Tag_DecConfigDesc:
            {
                uint8_t objectTypeIndication=tom->read();
                printf("\tDecConfigDesc : Tag %u\n",objectTypeIndication);
                if(trackType==TRACK_VIDEO)
                {
                    switch(objectTypeIndication)
                    {
                        case 0x60: // Visual ISO/IEC 13818-2 Simple Profile (MPEG-2)
                        case 0x61: // Visual ISO/IEC 13818-2 Main Profile (MPEG-2)
                            ADM_info("Changing FourCC from %s to MPEG (object type indication: 0x%x)\n",
                                    fourCC::tostring(_videostream.fccHandler),
                                    objectTypeIndication);
                            commonPart(MPEG);
                            break;
                        case 0x6A: // Visual ISO/IEC 11172-2 (MPEG Video)
                            ADM_info("Changing FourCC from %s to mpg1 (object type indication: 0x%x)\n",
                                    fourCC::tostring(_videostream.fccHandler),
                                    objectTypeIndication);
                            commonPart(mpg1);
                            break;
                        default:
                            ADM_warning("Object type indication 0x%x not handled\n",objectTypeIndication);
                            break;
                    }
                }
                if(trackType==TRACK_AUDIO && ADIO.encoding==WAV_AAC)
                {
                    switch(objectTypeIndication)
                    {
                        case 0x69:
                        case 0x6b:
                        /* case 0x6d: // PNG according to libavformat/isom.c */
                            ADIO.encoding=WAV_MP3;
                            break;
                        case 0xdd:
                            ADIO.encoding=WAV_OGG_VORBIS;
                            break;
                        /* case 226: // ? doesn't match any valid object type */
                        case 0xa5:
                            ADIO.encoding=WAV_AC3;
                            break;
                        case 0xa9:
                            ADIO.encoding=WAV_DTS;
                            break;
                        default:break;
                    }
                }
                tom->skipBytes(1+3+4+4);
                break;
            }
            case Tag_DecSpecificInfo:
                printf("\t DecSpecicInfo\n");
                switch(trackType)
                {
                    case TRACK_VIDEO: // Video
                        if(!VDEO.extraDataSize)
                        {
                            VDEO.extraDataSize=l;
                            VDEO.extraData=new uint8_t[l];
                            if(fread(VDEO.extraData,VDEO.extraDataSize,1,_fd)<1)
                            {
                                ADM_warning("Error reading video extradata from file.\n");
                                delete [] VDEO.extraData;
                                VDEO.extraData=NULL;
                                VDEO.extraDataSize=0;
                            }else
                            {
                                ADM_info("%d bytes of video extradata successfully read from file.\n",l);
                            }
                        }
                        break;
                    case TRACK_AUDIO:
                        printf("Esds for audio\n");
                        _tracks[1+nbAudioTrack].extraDataSize=l;
                        _tracks[1+nbAudioTrack].extraData=new uint8_t[l];
                        if(fread(_tracks[1+nbAudioTrack].extraData,
                            _tracks[1+nbAudioTrack].extraDataSize,1,_fd)<1)
                        {
                            ADM_warning("Error reading audio extradata from file.\n");
                            delete [] _tracks[1+nbAudioTrack].extraData;
                            _tracks[1+nbAudioTrack].extraData=NULL;
                            _tracks[1+nbAudioTrack].extraDataSize=0;
                        }else
                        {
                            ADM_info("%d bytes of audio extradata successfully read from file.\n",l);
                        }
                        break;
                    default: printf("Unknown track type for esds %d\n",trackType);
                }
        } // tag
    } // while

    tom->skipAtom();
    return 1;
}
/**
    \fn updateCtts
    \brief compute deltaPtsDts from ctts, needed to create valid mp4 when copying
*/
uint8_t MP4Header::updateCtts(MPsampleinfo *info )
{
    uint32_t scope=info->nbCtts;
    double f;
    if(scope>_videostream.dwLength) scope=_videostream.dwLength;
    ADM_info("[MP4]**************** Updating CTTS **********************\n");
    for(int i=0;i<scope;i++)
    {
        f=(int32_t)info->Ctts[i];
        f/=_videoScale;
        f*=1000000; // us
        f+=_tracks[0].index[i].dts;
        _tracks[0].index[i].pts=(uint64_t)f;
        aprintf(" Frame :%d DTS=%s",i,ADM_us2plain(_tracks[0].index[i].dts));
        aprintf(" PTS=%s\n",ADM_us2plain(_tracks[0].index[i].pts));
    }

  return 1;
}
//***********************************
MPsampleinfo::MPsampleinfo(void)
{
  memset(this,0,sizeof( MPsampleinfo));
}
MPsampleinfo::~MPsampleinfo()
{
#define MPCLEAR(x) {if(x) delete [] x;x=NULL;}
      MPCLEAR (Co);
      MPCLEAR (Sz);
      MPCLEAR (Sc);
      MPCLEAR (Sn);
      MPCLEAR (SttsN);
      MPCLEAR (SttsC);
      MPCLEAR (Sync);
      MPCLEAR (Ctts);
}

// EOF
