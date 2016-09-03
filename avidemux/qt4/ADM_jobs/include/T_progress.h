#ifndef Q_jobProgress_h
#define Q_jobProgress_h
#include "string"
#include <QWidget>
#include "ui_uiProgress.h"
#include "ADM_default.h"
using std::string;

/**
    \class jobProgress
*/
class jobProgress   : public QDialog 
{
	Q_OBJECT
protected:
            uint32_t numberOfJobs;
            uint32_t currentJob;
            string   currentOutputFile;
            uint32_t percent;
        
            void     updateUi(void);

public:
                jobProgress(uint32_t nbJobs);
	virtual     ~jobProgress();
            void  setCurrentJob(uint32_t job);
            void  setCurrentOutputName(const string &name);
            void  setPercent(uint32_t percent);
            void  updatePercent(void);
protected:
    Ui_DialogProgress ui;
public slots:
};
#endif	// Q_gui2_h

