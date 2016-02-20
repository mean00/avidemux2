/*
      EBML Reader
      (c) Mean 2007
      fixounet@free.fr
      
      GPL v2
      
*/

#ifndef  ADM_EBML
#define ADM_EBML
#include "mkv_tags.h"
class ADM_ebml
{
  protected:
        uint64_t  _fileSize;
        
  public:
        ADM_ebml  *_root;
        uint32_t  _refCount;                
                    ADM_ebml(void);
        virtual     ~ADM_ebml();
        int64_t     readSignedInt(uint32_t nb);
        uint64_t    readUnsignedInt(uint32_t nb);
        float       readFloat(uint32_t n);
        uint8_t     readString(char *string, uint32_t maxLen);
        uint8_t     readUTF8(char *string, uint32_t maxLen);
        uint8_t     readElemId(uint64_t *code,uint64_t *len);
        uint64_t    readEBMCode(void);
        uint64_t    readEBMCode_Full(void);
        int64_t     readEBMCode_Signed(void);
        
        
        /***********************************/
             uint8_t readu8(void);  
             int8_t  reads8(void);
             uint16_t readu16(void);  
             int16_t  reads16(void);
             uint32_t readu32(void);  
             int32_t  reads32(void);
        /***********************************/
        virtual     bool     readBin(uint8_t *whereto,uint32_t len)=0;
        virtual     bool     skip(uint32_t nbBytes)=0;
        virtual     uint64_t tell(void)=0;
};
/**
 */
class ADM_ebml_file : public ADM_ebml
{
  protected:
                FILE *fp;
                uint64_t  _begin;
                uint64_t  _size;
                uint32_t  _close;
  public: 
                              ADM_ebml_file();
                              ADM_ebml_file(ADM_ebml_file *father,uint64_t size);
                              ~ADM_ebml_file();
                    bool      open(const char *fn);
       
        virtual     bool      readBin(uint8_t *whereto,uint32_t len);
        virtual     bool      skip(uint32_t nbBytes);
                    uint64_t  tell(void);
                    bool      seek(uint64_t pos);
                    bool      finished(void);
                    uint64_t  getFileSize(void) {return _size;};
                    bool      find(ADM_MKV_SEARCHTYPE search,
                                        MKV_ELEM_ID  prim,MKV_ELEM_ID second,uint64_t *len,bool rewind=1);
                    bool      simplefind(MKV_ELEM_ID  prim,uint64_t *len,bool rewind=true);
                    uint64_t  remaining(void);
};

void bigHexPrint(uint64_t v);

#endif
