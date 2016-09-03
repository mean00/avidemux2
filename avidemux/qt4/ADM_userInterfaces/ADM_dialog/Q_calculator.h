#ifndef Q_calculator_h
#define Q_calculator_h

#include "ui_calculator.h"

class calculatorDialog : public QDialog
{
	Q_OBJECT

private:
	Ui_CalculatorDialog ui;
	unsigned int _videoFrameCount;
	unsigned int _videoDuration;

	void update(void);
	unsigned int getPictureSize(void);

public:
	calculatorDialog(QWidget* parent);
	~calculatorDialog();

	unsigned int videoSize(void);
	unsigned int videoBitrate(void);

private slots:
	void comboBox_currentIndexChanged(int index);
	void mediumComboBox_currentIndexChanged(int index);
	void spinBox_valueChanged(int value);
};
#endif	// Q_calculator_h
