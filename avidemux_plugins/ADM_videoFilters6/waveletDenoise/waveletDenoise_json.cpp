// automatically generated by admSerialization.py, do not edit!
#include "ADM_default.h"
#include "ADM_paramList.h"
#include "ADM_coreJson.h"
#include "waveletDenoise.h"
bool  waveletDenoise_jserialize(const char *file, const waveletDenoise *key){
admJson json;
json.addFloat("threshold",key->threshold);
json.addFloat("softness",key->softness);
json.addBool("highq",key->highq);
json.addBool("chroma",key->chroma);
return json.dumpToFile(file);
};
bool  waveletDenoise_jdeserialize(const char *file, const ADM_paramList *tmpl,waveletDenoise *key){
admJsonToCouple json;
CONFcouple *c=json.readFromFile(file);
if(!c) {ADM_error("Cannot read json file");return false;}
bool r= ADM_paramLoadPartial(c,tmpl,key);
delete c;
return r;
};