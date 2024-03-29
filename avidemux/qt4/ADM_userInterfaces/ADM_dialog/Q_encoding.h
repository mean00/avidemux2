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
#include "ADM_tray.h"

/**     
 * \class DIA_encodingQt4
 */
class DIA_encodingQt4 : public QDialog,public DIA_encodingBase
{
    Q_OBJECT
public:
    DIA_encodingQt4(uint64_t duration);
    ~DIA_encodingQt4( );
    
protected:
    void setTotalSize(uint64_t size);
    void setAudioSize(uint64_t size);
    void setVideoSize(uint64_t size);
    void setPercent(uint32_t percent);
    void setFps(uint32_t fps);
    void setFrameCount(uint32_t nb);
    void setElapsedTimeMs(uint32_t nb);
    void setRemainingTimeMS(uint32_t nb);
    void setAverageQz(uint32_t nb);
    void setAverageBitrateKbits(uint32_t kb);

    bool                stopRequest;
    bool                stayOpen;
    bool                deleteStats;
    bool                multiPass;
    bool                firstPass;
    Ui_encodingDialog   *ui;    
    ADM_tray            *tray;
    std::string         outputFileName;
    std::string         logFileName;
public:    
    void *WINDOW;
    
    void setPhase(ADM_ENC_PHASE_TYPE phase, const char *n = NULL);
    void setFileName(const char *n);
    void setLogFileName(const char *n);
    void setAudioCodec(const char *n);
    void setVideoCodec(const char *n);
    void setBitrate(uint32_t br,uint32_t globalbr);
    void setContainer(const char *container);
    void setQuantIn(int size);
    bool isAlive( void );
    void keepOpen(void);
 public slots:
    void useTrayButtonPressed(void);
    void pauseButtonPressed(void);
    void priorityChanged(int priorityLevel);
    void shutdownChanged(int state);
    void deleteStatsChanged(int state);
    void closeEvent(QCloseEvent *event);
    void reject(void);

};
#endif	// Q_encoding_h
