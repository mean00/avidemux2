#ifndef T_button_h
#define T_button_h

#include <QGridLayout>
#include <QPushButton>
#include <QWidget>

#include "ADM_inttype.h"
#include "DIA_factory.h"

namespace ADM_Qt4Factory
{
	class ADM_Qbutton : public QWidget
	{
		Q_OBJECT

	public slots:
		void clicked(bool i);

	protected :
		ADM_FAC_CALLBACK *_cb;
		void *_cookie;

	public:
		QPushButton *button;

		ADM_Qbutton(QWidget *z, QGridLayout *layout, const char *blah, int line, ADM_FAC_CALLBACK *cb, void *cookie);
		virtual ~ADM_Qbutton();
	};
}
#endif	// T_button_h
