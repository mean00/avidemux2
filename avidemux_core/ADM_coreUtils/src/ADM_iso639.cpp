/**
 * \file ADM_iso639.h
 * \brief iso639-2 to plaintext
 * Table borrowed from handbrake
 
 */

#include "ADM_default.h"
#include "ADM_iso639.h"
#include <string.h>
#include <ctype.h>

static int languageCount=0;


static const iso639_lang_t languages[] =
{ { "Unknown", "", "", "und" },
  { "Afar", "", "aa", "aar" },
  { "Abkhazian", "", "ab", "abk" },
  { "Afrikaans", "", "af", "afr" },
  { "Akan", "", "ak", "aka" },
  { "Albanian", "", "sq", "sqi", "alb" },
  { "Amharic", "", "am", "amh" },
  { "Arabic", "", "ar", "ara" },
  { "Aragonese", "", "an", "arg" },
  { "Armenian", "", "hy", "hye", "arm" },
  { "Assamese", "", "as", "asm" },
  { "Avaric", "", "av", "ava" },
  { "Avestan", "", "ae", "ave" },
  { "Aymara", "", "ay", "aym" },
  { "Azerbaijani", "", "az", "aze" },
  { "Bashkir", "", "ba", "bak" },
  { "Bambara", "", "bm", "bam" },
  { "Basque", "", "eu", "eus", "baq" },
  { "Belarusian", "", "be", "bel" },
  { "Bengali", "", "bn", "ben" },
  { "Bihari", "", "bh", "bih" },
  { "Bislama", "", "bi", "bis" },
  { "Bosnian", "", "bs", "bos" },
  { "Breton", "", "br", "bre" },
  { "Bulgarian", "", "bg", "bul" },
  { "Burmese", "", "my", "mya", "bur" },
  { "Catalan", "", "ca", "cat" },
  { "Chamorro", "", "ch", "cha" },
  { "Chechen", "", "ce", "che" },
  { "Chinese", "", "zh", "zho", "chi" },
  { "Church Slavic", "", "cu", "chu" },
  { "Chuvash", "", "cv", "chv" },
  { "Cornish", "", "kw", "cor" },
  { "Corsican", "", "co", "cos" },
  { "Cree", "", "cr", "cre" },
  { "Czech", "", "cs", "ces", "cze" },
  { "Danish", "Dansk", "da", "dan" },
  { "Divehi", "", "dv", "div" },
  { "Dutch", "Nederlands", "nl", "nld", "dut" },
  { "Dzongkha", "", "dz", "dzo" },
  { "English", "English", "en", "eng" },
  { "Esperanto", "", "eo", "epo" },
  { "Estonian", "", "et", "est" },
  { "Ewe", "", "ee", "ewe" },
  { "Faroese", "", "fo", "fao" },
  { "Fijian", "", "fj", "fij" },
  { "Finnish", "Suomi", "fi", "fin" },
  { "French", "Francais", "fr", "fra", "fre" },
  { "Western Frisian", "", "fy", "fry" },
  { "Fulah", "", "ff", "ful" },
  { "Georgian", "", "ka", "kat", "geo" },
  { "German", "Deutsch", "de", "deu", "ger" },
  { "Gaelic (Scots)", "", "gd", "gla" },
  { "Irish", "", "ga", "gle" },
  { "Galician", "", "gl", "glg" },
  { "Manx", "", "gv", "glv" },
  { "Greek, Modern", "", "el", "ell", "gre" },
  { "Guarani", "", "gn", "grn" },
  { "Gujarati", "", "gu", "guj" },
  { "Haitian", "", "ht", "hat" },
  { "Hausa", "", "ha", "hau" },
  { "Hebrew", "", "he", "heb" },
  { "Herero", "", "hz", "her" },
  { "Hindi", "", "hi", "hin" },
  { "Hiri Motu", "", "ho", "hmo" },
  { "Hungarian", "Magyar", "hu", "hun" },
  { "Igbo", "", "ig", "ibo" },
  { "Icelandic", "Islenska", "is", "isl", "ice" },
  { "Ido", "", "io", "ido" },
  { "Sichuan Yi", "", "ii", "iii" },
  { "Inuktitut", "", "iu", "iku" },
  { "Interlingue", "", "ie", "ile" },
  { "Interlingua", "", "ia", "ina" },
  { "Indonesian", "", "id", "ind" },
  { "Inupiaq", "", "ik", "ipk" },
  { "Italian", "Italiano", "it", "ita" },
  { "Javanese", "", "jv", "jav" },
  { "Japanese", "", "ja", "jpn" },
  { "Kalaallisut (Greenlandic)", "", "kl", "kal" },
  { "Kannada", "", "kn", "kan" },
  { "Kashmiri", "", "ks", "kas" },
  { "Kanuri", "", "kr", "kau" },
  { "Kazakh", "", "kk", "kaz" },
  { "Central Khmer", "", "km", "khm" },
  { "Kikuyu", "", "ki", "kik" },
  { "Kinyarwanda", "", "rw", "kin" },
  { "Kirghiz", "", "ky", "kir" },
  { "Komi", "", "kv", "kom" },
  { "Kongo", "", "kg", "kon" },
  { "Korean", "", "ko", "kor" },
  { "Kuanyama", "", "kj", "kua" },
  { "Kurdish", "", "ku", "kur" },
  { "Lao", "", "lo", "lao" },
  { "Latin", "", "la", "lat" },
  { "Latvian", "", "lv", "lav" },
  { "Limburgan", "", "li", "lim" },
  { "Lingala", "", "ln", "lin" },
  { "Lithuanian", "", "lt", "lit" },
  { "Luxembourgish", "", "lb", "ltz" },
  { "Luba-Katanga", "", "lu", "lub" },
  { "Ganda", "", "lg", "lug" },
  { "Macedonian", "", "mk", "mkd", "mac" },
  { "Marshallese", "", "mh", "mah" },
  { "Malayalam", "", "ml", "mal" },
  { "Maori", "", "mi", "mri", "mao" },
  { "Marathi", "", "mr", "mar" },
  { "Malay", "", "ms", "msa", "msa" },
  { "Malagasy", "", "mg", "mlg" },
  { "Maltese", "", "mt", "mlt" },
  { "Moldavian", "", "mo", "mol" },
  { "Mongolian", "", "mn", "mon" },
  { "Nauru", "", "na", "nau" },
  { "Navajo", "", "nv", "nav" },
  { "Ndebele, South", "", "nr", "nbl" },
  { "Ndebele, North", "", "nd", "nde" },
  { "Ndonga", "", "ng", "ndo" },
  { "Nepali", "", "ne", "nep" },
  { "Norwegian Nynorsk", "", "nn", "nno" },
  { "Norwegian Bokmål", "", "nb", "nob" },
  { "Norwegian", "Norsk", "no", "nor" },
  { "Chichewa; Nyanja", "", "ny", "nya" },
  { "Occitan (post 1500); Provençal", "", "oc", "oci" },
  { "Ojibwa", "", "oj", "oji" },
  { "Oriya", "", "or", "ori" },
  { "Oromo", "", "om", "orm" },
  { "Ossetian; Ossetic", "", "os", "oss" },
  { "Panjabi", "", "pa", "pan" },
  { "Persian", "", "fa", "fas", "per" },
  { "Pali", "", "pi", "pli" },
  { "Polish", "", "pl", "pol" },
  { "Portuguese", "Portugues", "pt", "por" },
  { "Pushto", "", "ps", "pus" },
  { "Quechua", "", "qu", "que" },
  { "Romansh", "", "rm", "roh" },
  { "Romanian", "", "ro", "ron", "rum" },
  { "Rundi", "", "rn", "run" },
  { "Russian", "", "ru", "rus" },
  { "Sango", "", "sg", "sag" },
  { "Sanskrit", "", "sa", "san" },
  { "Serbian", "", "sr", "srp", "scc" },
  { "Croatian", "Hrvatski", "hr", "hrv", "scr" },
  { "Sinhala", "", "si", "sin" },
  { "Slovak", "", "sk", "slk", "slo" },
  { "Slovenian", "", "sl", "slv" },
  { "Northern Sami", "", "se", "sme" },
  { "Samoan", "", "sm", "smo" },
  { "Shona", "", "sn", "sna" },
  { "Sindhi", "", "sd", "snd" },
  { "Somali", "", "so", "som" },
  { "Sotho, Southern", "", "st", "sot" },
  { "Spanish", "Espanol", "es", "spa" },
  { "Sardinian", "", "sc", "srd" },
  { "Swati", "", "ss", "ssw" },
  { "Sundanese", "", "su", "sun" },
  { "Swahili", "", "sw", "swa" },
  { "Swedish", "Svenska", "sv", "swe" },
  { "Tahitian", "", "ty", "tah" },
  { "Tamil", "", "ta", "tam" },
  { "Tatar", "", "tt", "tat" },
  { "Telugu", "", "te", "tel" },
  { "Tajik", "", "tg", "tgk" },
  { "Tagalog", "", "tl", "tgl" },
  { "Thai", "", "th", "tha" },
  { "Tibetan", "", "bo", "bod", "tib" },
  { "Tigrinya", "", "ti", "tir" },
  { "Tonga (Tonga Islands)", "", "to", "ton" },
  { "Tswana", "", "tn", "tsn" },
  { "Tsonga", "", "ts", "tso" },
  { "Turkmen", "", "tk", "tuk" },
  { "Turkish", "", "tr", "tur" },
  { "Twi", "", "tw", "twi" },
  { "Uighur", "", "ug", "uig" },
  { "Ukrainian", "", "uk", "ukr" },
  { "Urdu", "", "ur", "urd" },
  { "Uzbek", "", "uz", "uzb" },
  { "Venda", "", "ve", "ven" },
  { "Vietnamese", "", "vi", "vie" },
  { "Volapük", "", "vo", "vol" },
  { "Welsh", "", "cy", "cym", "wel" },
  { "Walloon", "", "wa", "wln" },
  { "Wolof", "", "wo", "wol" },
  { "Xhosa", "", "xh", "xho" },
  { "Yiddish", "", "yi", "yid" },
  { "Yoruba", "", "yo", "yor" },
  { "Zhuang", "", "za", "zha" },
  { "Zulu", "", "zu", "zul" },
  { NULL, NULL, NULL } };
/**
 * \fn ADM_getLanguageList
 * @return 
 */
const ADM_iso639_t *ADM_getLanguageList()
{
    return languages;
}
/**
 * \fn ADM_getLanguageListSize
 * @return 
 */
int                 ADM_getLanguageListSize()
{
    if(!languageCount)
    {
        const ADM_iso639_t *t=languages;
        int n=0;
        while(t->eng_name)
        {
            t++;
            n++;
        };
        languageCount=n;
    }
    return languageCount;
}
/**
 * \fn ADM_iso639b_toPlaintext
 * @param iso
 * @return 
 */
const char *ADM_iso639b_toPlaintext(const char *iso)
{
    iso639_lang_t *lang;
      for( lang = (iso639_lang_t*) languages; lang->eng_name; lang++ )
      {
          if(!strcmp(lang->iso639_2,iso)) return lang->eng_name;
      }
      return iso;
}
/**
 * \fn ADM_getIndexForIso639
 * @param iso
 * @return 
 */
int                 ADM_getIndexForIso639(const char *iso)
{
    int n=ADM_getLanguageListSize();
    for(int i=0;i<n;i++)
    {
        if(!strcmp(languages[i].iso639_2,iso)) return i;
    }
    ADM_error("Language %s not found in list\n",iso);
    return -1;
    
}
