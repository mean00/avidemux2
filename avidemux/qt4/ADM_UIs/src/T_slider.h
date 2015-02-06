#ifndef T_slider_h
#define T_slider_h

#include <QSlider>
#include <QSpinBox>
#include <QWidget>

namespace ADM_qt4Factory
{
	class SpinSlider : public QWidget
	{
		Q_OBJECT

	public:
		SpinSlider (QWidget *parent = 0);
		int value();

	signals:
		void valueChanged (int value);

	public slots:
		void setValue (int value);
		void setMinimum (int value);
		void setMaximum (int value);

	private:
		QSlider * slider;
		QSpinBox * spinner;
	};
}
#endif	// T_slider_h
