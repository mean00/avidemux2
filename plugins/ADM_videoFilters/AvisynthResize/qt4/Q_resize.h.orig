#ifndef Q_resizing_h
#define Q_resizing_h

#include "ui_resizing.h"

typedef struct resParam
{
	uint32_t width,height;
	uint32_t originalWidth, originalHeight;
	uint32_t fps1000;
	uint32_t algo;
	uint32_t pal;
} resParam;

class resizeWindow : public QDialog
{
	Q_OBJECT

protected: 
	resParam *_param;

public:
	resizeWindow(resParam *param);
	Ui_resizeDialog ui;

public slots:
	void gather(void);
	void update(int i);
};
#endif	// Q_resizing_h
