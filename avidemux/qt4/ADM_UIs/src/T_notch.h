#ifndef T_NOTCH_H
#define T_NOTCH_H

#include <QtGui/QGridLayout>
#include <QtGui/QCheckBox>

#include "DIA_factory.h"

namespace ADM_qt4Factory
{
	class QCheckBoxReadOnly : public QObject
	{
		Q_OBJECT

	private:
		QCheckBox *box;
		bool state;

	public:
		QCheckBoxReadOnly(QCheckBox *box, bool state);

		public slots:
			void stateChanged(int state);
	};
}
#endif
