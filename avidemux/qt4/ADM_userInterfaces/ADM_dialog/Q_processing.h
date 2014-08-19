/**
 * 
 */
#pragma once
#include "ui_processing.h"
#include "ADM_default.h"
#include "DIA_processing.h"
namespace ADM_Qt4CoreUIToolkit
{
#define PROC_NB_SLOTS 10
/**
 *      \class DIA_processingQt4 QObject,public DIA_audioTrackBase
 */    
class DIA_processingQt4 : public QDialog, public DIA_processingBase
{
        Q_OBJECT
protected:
        virtual void            postCtor( void ) ;
        Ui_DialogProcessing     *ui;        
        bool                    _stopRequest;      
        Clock                   _totalTime;
        int                     _slots[PROC_NB_SLOTS];
        int                     _slotIndex;
        bool                    _first;
        
public:
                                DIA_processingQt4( const char *title, uint64_t totalToProcess );
        virtual		        ~DIA_processingQt4();
            
        virtual bool            update(uint32_t frames,uint64_t processed);

public slots:
        void                    stop(bool a);
       
};
}

// EOF
