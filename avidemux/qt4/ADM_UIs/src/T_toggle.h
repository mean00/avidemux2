#ifndef T_toggle_h
#define T_toggle_h

#include <QCheckBox>
#include <QWidget>

namespace ADM_qt4Factory
{
	typedef enum
	{
		TT_TOGGLE,
		TT_TOGGLE_UINT,
		TT_TOGGLE_INT
	} TOG_TYPE;

	class ADM_QCheckBox : public QCheckBox
	{
		Q_OBJECT

	protected:
		void     *_toggle;
		TOG_TYPE _type;

	public slots:
		void changed(int i);

	public:
		ADM_QCheckBox(const QString & str, QWidget *root, void *toggle, TOG_TYPE type);
		void connectMe(void);
	};
}
#endif	// T_toggle_h
