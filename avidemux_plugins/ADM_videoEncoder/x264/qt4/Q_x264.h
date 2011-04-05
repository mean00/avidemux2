/***************************************************************************
                          \fn ADM_x264
                          \brief Front end for x264 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2011 gruntster/mean
 ***************************************************************************/

#ifndef Q_X264_H
#define Q_X264_H
#include "ui_x264ConfigDialog.h"
/**
    \class x264Dialog
*/
class x264Dialog : public QDialog
{
	Q_OBJECT

private:
    bool upload(void);
    int lastBitrate, lastVideoSize;

protected:
	void  *cookie;

public:
	x264Dialog(QWidget *parent, void *param);
	Ui_x264ConfigDialog ui;
    bool download(void);
public slots:

private slots:
        void encodingModeComboBox_currentIndexChanged(int index);
        void quantiserSlider_valueChanged(int value);
        void quantiserSpinBox_valueChanged(int value);
        void targetRateControlSpinBox_valueChanged(int value);
	
        void configurationComboBox_currentIndexChanged(int index);
        void saveAsButton_pressed(void);
        void deleteButton_pressed(void);

        bool updatePresetList(void);


#if 0
        void maxCrfSlider_valueChanged(int value);
        void maxCrfSpinBox_valueChanged(int value);
        void mbTreeCheckBox_toggled(bool checked);
#endif

};
#endif
