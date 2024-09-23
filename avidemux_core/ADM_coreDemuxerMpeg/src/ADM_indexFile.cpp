/**
    \file ADM_indexFile
    \brief Handle index file reading
    copyright            : (C) 2009 by mean
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
#include "ADM_default.h"
#include "ADM_indexFile.h"
#include "ctype.h"
#include "prefs.h"
/**

*/
dmxToken::dmxToken(const char *name,const char *value)
{
    this->name=ADM_strdup(name);
    this->value=ADM_strdup(value);
}
/**

*/
dmxToken::~dmxToken()
{
    if(name) ADM_dealloc(name);
    if(value) ADM_dealloc(value);
}

char *dmxToken::getName(void) {return name;}
char *dmxToken::getValue(void){return value;}
/**

*/
bool  dmxToken::isNumeric(void)
{
bool num=true;
int l=strlen(value);
    for(int i=0;i<l;i++)
    {
        char v=value[i];
        if(!isdigit(v)&& v!=0x0a && v!=0x0d) num=false;
    }
    return num;
}   
/**

*/
uint32_t dmxToken::getAsNumber(void)
{
uint32_t v;
    v=atoi(value);
    return v;
}
/**

*/
uint64_t dmxToken::getAsNumber64(void)
{
uint64_t v;
    sscanf(value,"%" PRId64,&v);
    return v;
}

//****************************************************************************************

/**

*/

indexFile::indexFile()
{
    file=NULL;
    mFile=NULL;
    buffer.setSize(ADM_INDEX_BUFFER);
}
/**

*/

indexFile::~indexFile()
{
   close();
}
/**

*/
void  indexFile::purgeTokens(void)
{
    int nb=ListOfTokens.size();
    for(int i=0;i<nb;i++)
        delete ListOfTokens[i];
    ListOfTokens.clear();
}
/**

*/
dmxToken        *indexFile::searchToken(const char *name)
{
    for(int i=0;i<ListOfTokens.size();i++)
    {
        dmxToken *tk=ListOfTokens[i];
        if(!strcasecmp(name,tk->getName())) return tk;
    }
    printf("[indexFile] Token %s not found\n",name);
    for(int i=0;i<ListOfTokens.size();i++)
        printf("  [%d]%s\n",i,ListOfTokens[i]->getName());
    return NULL;
}
/**

*/

bool indexFile::open(const char *name)
{
    uint32_t indexingPref = 2;
    if (!prefs->get(INDEXING_TS_PS_INDEXING, &indexingPref)) indexingPref = 2;
    
    if (indexingPref==0)
    {
        mFile = mfopen(name,"rt");
        if(!mFile)
        {
            return false;
        }
        return true;
    }
    
    file=ADM_fopen(name,"rt");
    if(!file)
    {
        mFile = mfopen(name,"rt");
        if(!mFile)
        {
            return false;
        }
    }
    return true;
}

/**

*/

bool indexFile::close(void)
{
    if(file)
    {
        fclose(file);
        file=NULL;
    }
    if(mFile)
    {
        mfclose(mFile);
        mFile=NULL;
    }
    purgeTokens();
    return true;
}

/**

*/

bool indexFile::goToSection(const char *section)
{
char match[100];
    sprintf(match,"[%s]\n",section);
    if (file)
    {
        fseek(file,0,SEEK_SET);
        while(1)
        {
            if(!fgets((char*)buffer.at(0),ADM_INDEX_BUFFER,file) )
            {
                printf("[indexFile] Cannot find section %s,%s*\n",section,match);
                return false;
            }
            if(!strcasecmp((char*)buffer.at(0),match)) return true;
        }
    }
    else
    {
        mfseek(mFile,0,SEEK_SET);
        while(1)
        {
            if(!mfgets((char*)buffer.at(0),ADM_INDEX_BUFFER,mFile) )
            {
                printf("[indexFile] Cannot find section %s,%s*\n",section,match);
                return false;
            }
            if(!strcasecmp((char*)buffer.at(0),match)) return true;
        }
    }

    return false;
}

/**
    \fn readSection
*/

bool indexFile::readSection(const char *section)
{
    if(false==goToSection(section)) return false;
    // Until we reach the next section, store all couples name/value into the
    //
    while(1)
    {
        if(!readString(ADM_INDEX_BUFFER,buffer.at(0))) break;
        if(buffer[0]=='[') break; // end of section
        // Now search the = and replace it by a zero
        char *head,*tail;
        head=(char *)buffer.at(0);
        tail=(char *)buffer.at(0);
        tail=strstr((char *)buffer.at(0),"=");
        if(!tail) 
        {
            if(buffer[0]=='#') // Comment
                continue;
            if(buffer[0]=='\0')
                break;
            printf("[psIndexer] Weird line :%s\n",buffer.at(0));
        }else
        {
            *tail=0;
            tail++;
            dmxToken *tk=new dmxToken(head,tail);
            ListOfTokens.append(tk);
        }
    }
    return true;
}

/**

*/

uint32_t indexFile::getAsUint32(const char *name)
{
    dmxToken *token=searchToken(name);
    if(!token) return 0;
    if(token->isNumeric()) return token->getAsNumber();
    printf("[psIndex] token %s is not a digit : %s\n",name,token->getValue());
    return 0;
}
/**

*/
uint64_t indexFile::getAsUint64(const char *name)
{
    dmxToken *token=searchToken(name);
    if(!token) return 0;
    if(token->isNumeric()) return token->getAsNumber64();
    printf("[psIndex] token %s is not a digit : %s\n",name,token->getValue());
    return 0;
}
/** 
    \fn getAsHex
    \read entry as hex

*/

uint32_t indexFile::getAsHex(const char *name)
{
    uint32_t v;
    dmxToken *token=searchToken(name);
    char *s;
    if(!token) return 0;
    s=token->getValue();
    sscanf(s,"%x",&v);
    return v;
}

/**

*/

char *indexFile::getAsString(const char *name)
{
    dmxToken *token=searchToken(name);
    if(!token) return NULL;
    return token->getValue();

}

/**

*/

bool  indexFile::readString(uint32_t maxLen,uint8_t *buffer)
{
    if (file)
    {
        if(!fgets((char *)buffer,maxLen,file)) return false;
    }
    else
    {
        if(!mfgets((char *)buffer,maxLen,mFile)) return false;
    }
    buffer[maxLen-1]=0;
    if(buffer[0])
        while(1)
        {
            int l=strlen((char *)buffer);
            if(!l) break;
            char c=buffer[l-1];
            if(c==0xa || c==0xd)
            {
                   buffer[l-1]=0;
                   continue;
            }
            break;
        }
    return true;
}


//EOF

