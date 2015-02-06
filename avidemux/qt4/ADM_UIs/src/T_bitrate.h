#ifndef T_bitrate_h
#define T_bitrate_h

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

#include "ADM_inttype.h"
#include "ADM_encoderConf.h"

namespace ADM_Qt4Factory
{
	class  ADM_Qbitrate : public QWidget
	{
		Q_OBJECT

	public slots:
		void comboChanged(int i);

	public:
		QSpinBox        *box;
		QComboBox       *combo;
		QLabel          *text1;
		QLabel          *text2;
		COMPRES_PARAMS  *compress;
		uint32_t        maxQ, _minQ;

		ADM_Qbitrate(COMPRES_PARAMS *p, uint32_t minQ, uint32_t mq, QGridLayout *layout, int line);
		virtual ~ADM_Qbitrate();
		void readBack(void);
	};
}
#endif	// T_bitrate_h
