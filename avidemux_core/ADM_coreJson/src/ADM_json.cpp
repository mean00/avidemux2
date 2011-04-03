/**

        \file  ADM_coreJson.h
        \brief 
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"
#include "ADM_default.h"

#include "ADM_coreJson.h"
extern "C"
{
#include "libjson.h"
}
#define COOKIE ((JSONNODE *)cookie)
/**

*/
admJson::admJson()
{
    JSONNODE *n = json_new(JSON_NODE);    
    cookie=(void *)n;
    cookies.push_back(cookie);
}
/**

*/
admJson::~admJson()
{
    int l=cookies.size();
    for(int i=0;i<l;i++)
    {
        json_delete((JSONNODE*)cookies[i]);
    }
    cookie=NULL;
    cookies.clear();
}
/**

*/
bool admJson::addNode(const char *nodeName)
{
    JSONNODE *n = json_new(JSON_NODE);    
    cookies.push_back((void *)n);
    json_set_name(n,nodeName);
    cookie=(void *)n;
    return true;
}
/**

*/
bool admJson::endNode(void)
{
    int l=cookies.size();
    ADM_assert(l>1);
    JSONNODE *prev=(JSONNODE *)cookies[l-2];
    json_push_back(prev,(JSONNODE *)cookie);
    cookies.pop_back();
    cookie=prev;
    return true;
}

/**
*/
bool admJson::addString(const char *key,const char *value)
{
    json_push_back(COOKIE, json_new_a(key,value));
    return true;
}
/**

*/
bool admJson::addUint32(const char *key,const uint32_t value)
{
    json_push_back(COOKIE, json_new_i(key,value));
    return true;
}
/**
*/
bool admJson::addInt32(const char *key,const int32_t value)
{
    json_push_back(COOKIE, json_new_i(key,value));
    return true;
}
/**

*/
bool admJson::addFloat(const char *key,const float value)
{
    json_push_back(COOKIE, json_new_f(key,value));
    return true;
}
/**

*/
bool admJson::addBool(const char *key,const bool value)
{
    json_push_back(COOKIE, json_new_b(key,value));
    return true;
}
/**

*/
bool admJson::addCompressParam(const char *key, const COMPRES_PARAMS param)
{
    return true;
}
/**

*/
bool admJson::dumpToFile(const char *file)
{
    FILE *f=fopen(file,"w");
    if(!f)
    {
        ADM_error("Cannot open file %s\n",file);
        return false;
    }
    json_char *jc = json_write_formatted(COOKIE);
    fprintf(f,"%s",jc);
    json_free(jc);

    fclose(f);
    return true;
}
//****************************************************************
/**

*/
bool admJsonToCouple::scan( void *xnode,string name)
{
    JSONNODE *node=(JSONNODE *)xnode;
   if (!node){
        ADM_error("Invalid JSON Node\n");
        return false;
    }
 
    JSONNODE_ITERATOR i = json_begin(node);
    while (i != json_end(node)){
        if (*i == NULL){
            ADM_error("Invalid JSON Node\n");
            return false;
        }
        json_char *node_name = json_name(*i);
        //printf("Node :%s\n",node_name);
        // recursively call ourselves to dig deeper into the tree
        if (json_type(*i) == JSON_ARRAY || json_type(*i) == JSON_NODE)
        {
            if(name=="") 
                scan(*i,string(node_name));
            else
                scan(*i,name+string(".")+string(node_name));
        }
        else
        {
            keyVal k;
            json_char *node_value = json_as_string(*i);
            if(name=="") 
                k.key=string(node_name);
            else 
                k.key=string(name)+string(".")+string(node_name);
            k.value=string(node_value);
            readItems.push_back(k);
            json_free(node_value);
        }
        json_free(node_name);
        ++i;
    }
    return true;
}
/**
    \fn readFromFile
    \brief construct a list of key/value from a json file
*/

CONFcouple *admJsonToCouple::readFromFile(const char *file)
{
        FILE *f=fopen(file,"rt");
        if(!f)
        {
            ADM_error("Cannot open %s\n",file);
            return NULL;
        }
        fseek(f,0,SEEK_END);
        uint32_t fileSize=ftell(f);
        fseek(f,0,SEEK_SET);
        char buffer[fileSize+1];
        char *head=buffer;
        while(fgets(head,fileSize,f))
        {
            head=buffer+strlen(buffer);
        }
        fclose(f);
        // Now parse and build a tree out of it...
        JSONNODE *n = json_parse(buffer);
        if(true==scan(n,""))
        {

        }
        json_delete(n);
        // Dump
        int l=readItems.size();
        for(int i=0;i<l;i++)
        {
            printf(" %s => %s\n",readItems[i].key.c_str(),readItems[i].value.c_str());
        }
        //
        CONFcouple *c=new CONFcouple(l);
        for(int i=0;i<l;i++)
              c->setInternalName(readItems[i].key.c_str(),readItems[i].value.c_str());
        return c;
}

// EOF
