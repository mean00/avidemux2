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
{ { QT_TRANSLATE_NOOP("adm","Unknown"), "", "", "und", NULL },
  { "Afar", "", "aa", "aar", NULL },
  { "Abkhazian", "", "ab", "abk", NULL },
  { "Afrikaans", "", "af", "afr", NULL },
  { "Akan", "", "ak", "aka", NULL },
  { "Albanian", "", "sq", "sqi", "alb" },
  { "Amharic", "", "am", "amh", NULL },
  { "Arabic", "", "ar", "ara", NULL },
  { "Aragonese", "", "an", "arg", NULL },
  { "Armenian", "", "hy", "hye", "arm" },
  { "Assamese", "", "as", "asm", NULL },
  { "Avaric", "", "av", "ava", NULL },
  { "Avestan", "", "ae", "ave", NULL },
  { "Aymara", "", "ay", "aym", NULL },
  { "Azerbaijani", "", "az", "aze", NULL },
  { "Bashkir", "", "ba", "bak", NULL },
  { "Bambara", "", "bm", "bam", NULL },
  { "Basque", "", "eu", "eus", "baq" },
  { "Belarusian", "", "be", "bel", NULL },
  { "Bengali", "", "bn", "ben", NULL },
  { "Bihari", "", "bh", "bih", NULL },
  { "Bislama", "", "bi", "bis", NULL },
  { "Bosnian", "", "bs", "bos", NULL },
  { "Breton", "", "br", "bre", NULL },
  { "Bulgarian", "", "bg", "bul", NULL },
  { "Burmese", "", "my", "mya", "bur" },
  { "Catalan", "", "ca", "cat", NULL },
  { "Chamorro", "", "ch", "cha", NULL },
  { "Chechen", "", "ce", "che", NULL },
  { "Chinese", "", "zh", "zho", "chi" },
  { "Church Slavic", "", "cu", "chu", NULL },
  { "Chuvash", "", "cv", "chv", NULL },
  { "Cornish", "", "kw", "cor", NULL },
  { "Corsican", "", "co", "cos", NULL },
  { "Cree", "", "cr", "cre", NULL },
  { "Czech", "", "cs", "ces", "cze" },
  { "Danish", "Dansk", "da", "dan", NULL },
  { "Divehi", "", "dv", "div", NULL },
  { "Dutch", "Nederlands", "nl", "nld", "dut" },
  { "Dzongkha", "", "dz", "dzo", NULL },
  { "English", "English", "en", "eng", NULL },
  { "Esperanto", "", "eo", "epo", NULL },
  { "Estonian", "", "et", "est", NULL },
  { "Ewe", "", "ee", "ewe", NULL },
  { "Faroese", "", "fo", "fao", NULL },
  { "Fijian", "", "fj", "fij", NULL },
  { "Finnish", "Suomi", "fi", "fin", NULL },
  { "French", "Francais", "fr", "fra", "fre" },
  { "Western Frisian", "", "fy", "fry", NULL },
  { "Fulah", "", "ff", "ful", NULL },
  { "Georgian", "", "ka", "kat", "geo" },
  { "German", "Deutsch", "de", "deu", "ger" },
  { "Gaelic (Scots)", "", "gd", "gla", NULL },
  { "Irish", "", "ga", "gle", NULL },
  { "Galician", "", "gl", "glg", NULL },
  { "Manx", "", "gv", "glv", NULL },
  { "Greek, Modern", "", "el", "ell", "gre" },
  { "Guarani", "", "gn", "grn", NULL },
  { "Gujarati", "", "gu", "guj", NULL },
  { "Haitian", "", "ht", "hat", NULL },
  { "Hausa", "", "ha", "hau", NULL },
  { "Hebrew", "", "he", "heb", NULL },
  { "Herero", "", "hz", "her", NULL },
  { "Hindi", "", "hi", "hin", NULL },
  { "Hiri Motu", "", "ho", "hmo", NULL },
  { "Hungarian", "Magyar", "hu", "hun" },
  { "Igbo", "", "ig", "ibo", NULL },
  { "Icelandic", "Islenska", "is", "isl", "ice" },
  { "Ido", "", "io", "ido", NULL },
  { "Sichuan Yi", "", "ii", "iii", NULL },
  { "Inuktitut", "", "iu", "iku", NULL },
  { "Interlingue", "", "ie", "ile", NULL },
  { "Interlingua", "", "ia", "ina", NULL },
  { "Indonesian", "", "id", "ind", NULL },
  { "Inupiaq", "", "ik", "ipk", NULL },
  { "Italian", "Italiano", "it", "ita", NULL },
  { "Javanese", "", "jv", "jav", NULL },
  { "Japanese", "", "ja", "jpn", NULL },
  { "Kalaallisut (Greenlandic)", "", "kl", "kal", NULL },
  { "Kannada", "", "kn", "kan", NULL },
  { "Kashmiri", "", "ks", "kas", NULL },
  { "Kanuri", "", "kr", "kau", NULL },
  { "Kazakh", "", "kk", "kaz", NULL },
  { "Central Khmer", "", "km", "khm", NULL },
  { "Kikuyu", "", "ki", "kik", NULL },
  { "Kinyarwanda", "", "rw", "kin", NULL },
  { "Kirghiz", "", "ky", "kir", NULL },
  { "Komi", "", "kv", "kom", NULL },
  { "Kongo", "", "kg", "kon", NULL },
  { "Korean", "", "ko", "kor", NULL },
  { "Kuanyama", "", "kj", "kua", NULL },
  { "Kurdish", "", "ku", "kur", NULL },
  { "Lao", "", "lo", "lao", NULL },
  { "Latin", "", "la", "lat", NULL },
  { "Latvian", "", "lv", "lav", NULL },
  { "Limburgan", "", "li", "lim", NULL },
  { "Lingala", "", "ln", "lin", NULL },
  { "Lithuanian", "", "lt", "lit", NULL },
  { "Luxembourgish", "", "lb", "ltz", NULL },
  { "Luba-Katanga", "", "lu", "lub", NULL },
  { "Ganda", "", "lg", "lug", NULL },
  { "Macedonian", "", "mk", "mkd", "mac" },
  { "Marshallese", "", "mh", "mah", NULL },
  { "Malayalam", "", "ml", "mal", NULL },
  { "Maori", "", "mi", "mri", "mao" },
  { "Marathi", "", "mr", "mar", NULL },
  { "Malay", "", "ms", "msa", "may" },
  { "Malagasy", "", "mg", "mlg", NULL },
  { "Maltese", "", "mt", "mlt", NULL },
  { "Moldavian", "", "mo", "mol", NULL },
  { "Mongolian", "", "mn", "mon", NULL },
  { "Nauru", "", "na", "nau", NULL },
  { "Navajo", "", "nv", "nav", NULL },
  { "Ndebele, South", "", "nr", "nbl", NULL },
  { "Ndebele, North", "", "nd", "nde", NULL },
  { "Ndonga", "", "ng", "ndo", NULL },
  { "Nepali", "", "ne", "nep", NULL },
  { "Norwegian Nynorsk", "", "nn", "nno", NULL },
  { "Norwegian Bokmål", "", "nb", "nob", NULL },
  { "Norwegian", "Norsk", "no", "nor", NULL },
  { "Chichewa; Nyanja", "", "ny", "nya", NULL },
  { "Occitan (post 1500); Provençal", "", "oc", "oci", NULL },
  { "Ojibwa", "", "oj", "oji", NULL },
  { "Oriya", "", "or", "ori", NULL },
  { "Oromo", "", "om", "orm", NULL },
  { "Ossetian; Ossetic", "", "os", "oss", NULL },
  { "Panjabi", "", "pa", "pan", NULL },
  { "Persian", "", "fa", "fas", "per" },
  { "Pali", "", "pi", "pli", NULL },
  { "Polish", "", "pl", "pol", NULL },
  { "Portuguese", "Portugues", "pt", "por", NULL },
  { "Pushto", "", "ps", "pus", NULL },
  { "Quechua", "", "qu", "que", NULL },
  { "Romansh", "", "rm", "roh", NULL },
  { "Romanian", "", "ro", "ron", "rum" },
  { "Rundi", "", "rn", "run", NULL },
  { "Russian", "", "ru", "rus", NULL },
  { "Sango", "", "sg", "sag", NULL },
  { "Sanskrit", "", "sa", "san", NULL },
  { "Serbian", "", "sr", "srp", "scc" },
  { "Croatian", "Hrvatski", "hr", "hrv", "scr" },
  { "Sinhala", "", "si", "sin", NULL },
  { "Slovak", "", "sk", "slk", "slo" },
  { "Slovenian", "", "sl", "slv", NULL },
  { "Northern Sami", "", "se", "sme", NULL },
  { "Samoan", "", "sm", "smo", NULL },
  { "Shona", "", "sn", "sna", NULL },
  { "Sindhi", "", "sd", "snd", NULL },
  { "Somali", "", "so", "som", NULL },
  { "Sotho, Southern", "", "st", "sot", NULL },
  { "Spanish", "Espanol", "es", "spa", NULL },
  { "Sardinian", "", "sc", "srd", NULL },
  { "Swati", "", "ss", "ssw", NULL },
  { "Sundanese", "", "su", "sun", NULL },
  { "Swahili", "", "sw", "swa", NULL },
  { "Swedish", "Svenska", "sv", "swe", NULL },
  { "Tahitian", "", "ty", "tah", NULL },
  { "Tamil", "", "ta", "tam", NULL },
  { "Tatar", "", "tt", "tat", NULL },
  { "Telugu", "", "te", "tel", NULL },
  { "Tajik", "", "tg", "tgk", NULL },
  { "Tagalog", "", "tl", "tgl", NULL },
  { "Thai", "", "th", "tha", NULL },
  { "Tibetan", "", "bo", "bod", "tib" },
  { "Tigrinya", "", "ti", "tir", NULL },
  { "Tonga (Tonga Islands)", "", "to", "ton", NULL },
  { "Tswana", "", "tn", "tsn", NULL },
  { "Tsonga", "", "ts", "tso", NULL },
  { "Turkmen", "", "tk", "tuk", NULL },
  { "Turkish", "", "tr", "tur", NULL },
  { "Twi", "", "tw", "twi", NULL },
  { "Uighur", "", "ug", "uig", NULL },
  { "Ukrainian", "", "uk", "ukr", NULL },
  { "Urdu", "", "ur", "urd", NULL },
  { "Uzbek", "", "uz", "uzb", NULL },
  { "Venda", "", "ve", "ven", NULL },
  { "Vietnamese", "", "vi", "vie", NULL },
  { "Volapük", "", "vo", "vol", NULL },
  { "Welsh", "", "cy", "cym", "wel" },
  { "Walloon", "", "wa", "wln", NULL },
  { "Wolof", "", "wo", "wol", NULL },
  { "Xhosa", "", "xh", "xho", NULL },
  { "Yiddish", "", "yi", "yid", NULL },
  { "Yoruba", "", "yo", "yor", NULL },
  { "Zhuang", "", "za", "zha", NULL },
  { "Zulu", "", "zu", "zul", NULL },
  { NULL, NULL, NULL, NULL, NULL } };
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
        if(!strcmp(lang->iso639_2,iso))
            return lang->eng_name;
        if(lang->iso639_2b && !strcmp(lang->iso639_2b,iso))
            return lang->eng_name;
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
        if(languages[i].iso639_2b && !strcmp(languages[i].iso639_2b,iso)) return i;
    }
    ADM_error("Language %s not found in list\n",iso);
    return -1;
    
}
