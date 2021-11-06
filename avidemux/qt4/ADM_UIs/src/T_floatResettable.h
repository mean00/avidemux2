#ifndef T_FLOAT_RES_h
#define T_FLOAT_RES_h

#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>

#include "ADM_inttype.h"
#include "DIA_factory.h"

namespace ADM_Qt4Factory
{
    class ADM_QDoubleSpinboxResettable : public QWidget
    {
        Q_OBJECT

    private slots:
        void    reset(bool checked);

    private:
        double  _rst;
        QLabel  *text;
        QDoubleSpinBox *box;
        QPushButton *button;
        void    *cookie;

    public:
        ADM_QDoubleSpinboxResettable(QWidget *parent, QGridLayout *layout, void *elem, const char *title, const char *tip,
                    int line, int decimals, double min, double max, double rst, double current);
        virtual ~ADM_QDoubleSpinboxResettable();
        void    enable(bool onoff);
        double  readout(void);
    };
}
#endif
