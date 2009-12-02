/**
    \file Q_shell.h
*/
#ifndef Q_SHELL_H
#define Q_SHELL_H
#include "ADM_inttype.h"
#include <QtGui/QItemDelegate>
#include "ui_shell.h"
#include "ADM_jsShell.h"
/**
    \class ADM_jsQt4Shell
*/

class qShell: public QDialog
{
	Q_OBJECT
protected:
    jsShellEvaluate      *evaluator;
    Ui_SpiderMonkeyShell ui;
public:
                    qShell(jsShellEvaluate *s) ;
    virtual         ~qShell() ;
    bool            run(void);
public slots:
    bool            evaluate(bool x);
};

#endif
