/***************************************************************************
                          \fn ADM_x265
                          \brief Front end for x265 HEVC encoder
                             -------------------
    
    copyright            : (C) 2014 gruntster/mean
 ***************************************************************************/

#ifndef Q_X265_H
#define Q_X265_H
#include "ui_x265ConfigDialog.h"
/**
    \class x265Dialog
*/
class x265Dialog : public QDialog
{
	Q_OBJECT

private:
    bool upload(void);
    int lastBitrate, lastVideoSize;

protected:
	void  *cookie;

public:
	x265Dialog(QWidget *parent, void *param);
	Ui_x265ConfigDialog ui;
    bool download(void);
public slots:

private slots:
        void useAdvancedConfigurationCheckBox_toggled(bool checked);
        void meSpinBox_valueChanged(int value);
        void meSlider_valueChanged(int value);
        void encodingModeComboBox_currentIndexChanged(int index);
        void quantiserSlider_valueChanged(int value);
        void quantiserSpinBox_valueChanged(int value);
        void targetRateControlSpinBox_valueChanged(int value);
        void loopFilterCheckBox_toggled(bool checked);
        void mbTreeCheckBox_toggled(bool checked);
        void aqVarianceCheckBox_toggled(bool checked);
	
        void configurationComboBox_currentIndexChanged(int index);
        void saveAsButton_pressed(void);
        void deleteButton_pressed(void);

        bool updatePresetList(void);

        bool toogleAdvancedConfiguration(bool advancedEnabled);
        
#if 0
        void maxCrfSlider_valueChanged(int value);
        void maxCrfSpinBox_valueChanged(int value);
#endif

};
#endif
