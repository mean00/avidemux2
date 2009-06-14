#ifndef T_threadCount_h
#define T_threadCount_h

#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

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
