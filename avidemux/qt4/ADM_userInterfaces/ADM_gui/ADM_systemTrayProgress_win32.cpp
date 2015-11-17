#pragma once

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_systemTrayProgress.h"
#include <QtWinExtras>
/**
 */
class winTaskBarProgress : public admUITaskBarProgress
{
    public:
        winTaskBarProgress(QMainWindow *w)
        {
              button = new QWinTaskbarButton(w);
        }
        virtual ~winTaskBarProgress()
        {
            delete button;
            button=NULL;
        }
    virtual bool enable() 
    {
        QWinTaskbarProgress *progress = button->progress();
        progress->setVisible(true);
        return true;
    }
    virtual bool disable() 
    {
        QWinTaskbarProgress *progress = button->progress();
        progress->setVisible(false);
        return true;
    }
    virtual bool setProgress(int percent) 
    {
        QWinTaskbarProgress *progress = button->progress();
        progress->setValue(percent);
    } 
    QWinTaskbarButton *button;
};

/**
 */
admUITaskBarProgress *createADMTaskBarProgress(void *parent)
{
        QMainWindow *win=( QMainWindow *)parent;
        return new winTaskBarProgress(win);
}
// EOF