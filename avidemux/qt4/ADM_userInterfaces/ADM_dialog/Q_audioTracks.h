#include "ui_audioTracks.h"
#define NB_MENU 4
/**
    \class audioTrackWindow
*/

#define ROLL(a,b) {a[0]=ui.b##1;a[1]=ui.b##2;a[2]=ui.b##3;a[3]=ui.b##4;}

class audioTrackWindow : public QDialog
{
	Q_OBJECT

public:
            
            QCheckBox        *enabled[NB_MENU];
            QComboBox        *codec[NB_MENU];
            QComboBox        *inputs[NB_MENU];
            QPushButton      *codecConf[NB_MENU];
            QPushButton      *filters[NB_MENU];
            QPushButton      *languages[NB_MENU];

public:
	audioTrackWindow()
    {
        ui.setupUi(this);
        ROLL(enabled,checkBoxEnabled);
        ROLL(codec,comboBoxCodec);
        ROLL(codecConf,pushButtonCodecConf);
        ROLL(filters,pushButtonFilter);
        ROLL(inputs,comboBoxInput);
        ROLL(languages,pushButtonLanguage);
    }
	~audioTrackWindow()
    {

    }
	Ui_DialogAudioTracks ui;

public slots:
   
};