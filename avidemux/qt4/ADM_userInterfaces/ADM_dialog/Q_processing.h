/**
 * 
 */
#pragma once
#include "ui_processing.h"
#include "ADM_default.h"
#include "DIA_processing.h"
namespace ADM_Qt4CoreUIToolkit
{
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
public:
                                DIA_processingQt4( const char *title,uint32_t fps100, uint64_t duration );
        virtual		        ~DIA_processingQt4();
            
        virtual bool            update(uint32_t frames);

public slots:
        void                    stop(bool a);
       
};
}

// EOF
