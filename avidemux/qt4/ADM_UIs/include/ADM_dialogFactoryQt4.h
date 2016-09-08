#include <QString>

#define FAC_QT_GRIDLAYOUT 1
#define FAC_QT_VBOXLAYOUT 2
/**
 */
class QtFactoryUtils
{
public:
    QString myQtTitle;
    bool    titleFromShortKey(const char *in);
};