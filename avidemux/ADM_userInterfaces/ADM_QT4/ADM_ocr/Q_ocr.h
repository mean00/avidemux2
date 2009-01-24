#ifndef Q_ocr_h
#define Q_ocr_h

#include "ui_ocr.h"
#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_ocr/ADM_ocr.h"
#include "ADM_ocr/ADM_ocrInternal.h"
#include "ADM_UIs/ADM_QT4/include/DIA_flyDialogQt4.h"

class Ui_ocrWindow : public QDialog
{
	Q_OBJECT

public:
	Ui_ocrWindow(void);
	~Ui_ocrWindow();
	Ui_DialogOcr ui;

	uint32_t _w,_h;
	uint8_t *data;
	ADM_QCanvas *canvas;
	admGlyph *_glyph,*_head;
	char *_decodedString;

	ADM_QCanvas *smallCanvas;
	ReplyType _reply;

	void setGlyph(admGlyph *glyph,admGlyph *head,char *decodedString);
	void dialogReturn(ReplyType r);
	void resizeSmall(uint32_t w,uint32_t h,uint8_t *smallData);

public slots:
	void pushButtonCalibrate(bool i);
	void pushButtonSkipAll(bool i);
	void pushButtonSkip(bool i);
	void pushButtonIgnore(bool i);
	void pushButtonOk(bool i);
	void pushButtonClose(bool i);
};
#endif	// Q_ocr_h
