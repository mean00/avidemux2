/***************************************************************************
                          \fn nvEnc_utils.h
                          \brief Easier to use api to nvenc
                             -------------------

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
typedef void* CUcontext;

/**
 * \class nvEncSession
 * \brief Wrapper around nvenc
 */
class nvEncSession
{
public:
                nvEncSession();
                ~nvEncSession();
        bool    init(void);
public:
    // create context then openSession
    // closeSession, then delete context
        bool    createContext(void);
        bool    deleteContext(void);
        
        bool    openSession(void);
        bool    closeSession(void);
protected:
        bool                            inited;
        void                            *encoder;
        CUcontext                       context;
};
// Load if needed, returns true if nvenc is operationnal
// can be called multiple time without problem
bool loadNvEnc(void);

// EOf