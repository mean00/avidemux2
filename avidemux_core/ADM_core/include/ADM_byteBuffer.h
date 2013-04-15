/***************************************************************************
                          ADM_audioStream.h  -  description
                             -------------------
    copyright            : (C) 2008 by mean
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
#pragma once
/**
 * \class ADM_buffer
 * @param size
 */
class ADM_byteBuffer
{
        public:
                ADM_byteBuffer()
                {
                    data=NULL;
                    len=0;
                }                
                ADM_byteBuffer(int size)
                {
                    data=NULL;
                    len=0;
                    setSize(size);
                }
                virtual ~ADM_byteBuffer()
                {
                    clean();
                }
                void setSize(int size)
                {
                    ADM_assert(!data);
                    data=(uint8_t *)ADM_alloc(size);
                    len=size;
                }
                uint8_t  *at(int size)
                {
                    return data+size;
                }
                uint8_t & operator[]( int ad ) 
                {
                    ADM_assert(data);
                    return data[ad];
                }
                
                bool clean()
                {
                    if(data)    
                        ADM_dezalloc(data);
                    data=NULL;
                    len=0;
                    return true;
                }
        protected:
                uint8_t *data;
                int len;
};
/**
 * \class ADM_floatBuffer
  */
class ADM_floatBuffer
{
        public:
                ADM_floatBuffer()
                {
                    data=NULL;
                    len=0;
                }                
                ADM_floatBuffer(int size)
                {
                    setSize(size);
                }
                virtual ~ADM_floatBuffer()
                {
                    clean();
                }
                void setSize(int size)
                {
                    ADM_assert(!data);
                    data=(float *)ADM_alloc(size*sizeof(float));
                    len=size;
                }
                float  *at(int size)
                {
                    return data+size;
                }
                float & operator[]( int ad ) 
                {
                    ADM_assert(data);
                    return data[ad];
                }
                
                bool clean()
                {
                    if(data)    
                        ADM_dezalloc(data);
                    data=NULL;
                    len=0;
                    return true;
                }
        protected:
                float *data;
                int len;
};
