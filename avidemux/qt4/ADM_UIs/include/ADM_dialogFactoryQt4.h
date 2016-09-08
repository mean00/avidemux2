#pragma once
#include <QString>

#define FAC_QT_GRIDLAYOUT 1
#define FAC_QT_VBOXLAYOUT 2
/**
 */
class QtFactoryUtils
{
public:
            QtFactoryUtils(const char *in);
protected:    
    QString myQtTitle;
    bool    titleFromShortKey(const char *in);
};