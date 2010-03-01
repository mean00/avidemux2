/** *************************************************************************
             
    \fn Q_encoding.h
    
                      
    copyright            : (C) 2008 by mean/gruntster/?
    
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef Q_encoding_h
#define Q_encoding_h

#include "ui_encoding.h"
#include "ADM_inttype.h"
#include "DIA_encoding.h"
/**
    \class encodingWindow
*/
class encodingWindow : public QDialog
{
     Q_OBJECT

 public:
     encodingWindow();
     Ui_encodingDialog ui;

 public slots:
	void buttonPressed(void);
	void priorityChanged(int priorityLevel);
	void shutdownChanged(int state);
};
/**
    \class DIA_encodingQt4
*/

class DIA_encodingQt4 : public DIA_encodingBase
{
public:
    DIA_encodingQt4( uint64_t duration);
    ~DIA_encodingQt4( );
    
protected:
    encodingWindow *window;
    void setFps(uint32_t fps);
    void setPhasis(const char *n);
    void setAudioCodec(const char *n);
    void setVideoCodec(const char *n);
    void setBitrate(uint32_t br,uint32_t globalbr);
    void setContainer(const char *container);
    void setQuantIn(int size);
    void setTotalSize(uint64_t size);
    void setAudioSize(uint64_t size);
    void setPercent(uint32_t percent);
    bool isAlive( void );
};
#endif	// Q_encoding_h
