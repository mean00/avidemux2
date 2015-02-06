#ifndef T_threadCount_h
#define T_threadCount_h
#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>
#include "ADM_default.h"

namespace ADM_qt4Factory
{
	class ADM_QthreadCount : public QWidget
	{
		Q_OBJECT

	public slots:
		void radioGroupChanged(QAbstractButton *s);

	public:
		QLabel *text;
		QRadioButton *radiobutton1;
		QRadioButton *radiobutton2;
		QRadioButton *radiobutton3;
		QButtonGroup *buttonGroup;
		QSpinBox *spinBox;

		ADM_QthreadCount(QWidget *widget, const char *title, uint32_t value, QGridLayout *layout, int line);
		~ADM_QthreadCount();
	};
}
#endif	// T_threadCount_h
