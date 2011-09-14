#ifndef Q_vobsub_h
#define Q_vobsub_h
#if 0
#include "ui_vobsub.h"
#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter/ADM_vobsubinfo.h"
#include "ADM_videoFilter/ADM_vidVobSub.h"

class Ui_vobsubWindow : public QDialog
{
	Q_OBJECT

protected:
	vobSubParam *param;
	void fillLanguage(const char *file);

public:
	Ui_vobsubWindow(QWidget *parent, vobSubParam *param);
	~Ui_vobsubWindow();
	Ui_vobSubDialog ui;

	void gather(void);

private slots:
	void idxSel(bool i);
};
#endif	// Q_vobsub_h
#endif