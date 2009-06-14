#ifndef Q_encoding_h
#define Q_encoding_h

#include "ui_encoding.h"

class encodingWindow : public QDialog
{
     Q_OBJECT

 public:
     encodingWindow();
     Ui_encodingDialog ui;

 public slots:
	void buttonPressed(void);
	void priorityChanged(int priorityLevel);
	void shutdownChanged(int state);
};
#endif	// Q_encoding_h
