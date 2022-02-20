#include "ui_audioTracks.h"
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#define NB_MENU 32
/**
    \class audioTrackWindow
*/

class audioTrackWindow : public QDialog
{
	Q_OBJECT
public:
            int              trackCount;
            QLabel           *labels[NB_MENU];
            QCheckBox        *enabled[NB_MENU];
            QComboBox        *codec[NB_MENU];
            QComboBox        *inputs[NB_MENU];
            QPushButton      *codecConf[NB_MENU];
            QPushButton      *filters[NB_MENU];
            QComboBox        *languages[NB_MENU];
            QPushButton      *dupConfig;

public:
    audioTrackWindow(int numOfTrack)
    {
        trackCount = numOfTrack;
        ui.setupUi(this);
        const char * clabel = QT_TRANSLATE_NOOP("qaudiotracks","Track %d");
        char * plabel = new char[strlen(clabel)+16];
        for (int i=0; i<NB_MENU; i++)
        {
            sprintf(plabel,clabel,i+1);
            labels[i] = new QLabel(QString(plabel));
            ui.gridLayout->addWidget(labels[i],i,0);
            enabled[i] = new QCheckBox(QT_TRANSLATE_NOOP("qaudiotracks","Enabled"));
            ui.gridLayout->addWidget(enabled[i],i,1);
            inputs[i] = new QComboBox();
            ui.gridLayout->addWidget(inputs[i],i,2);
            languages[i] = new QComboBox();
            ui.gridLayout->addWidget(languages[i],i,3);
            codec[i] = new QComboBox();
            ui.gridLayout->addWidget(codec[i],i,4);
            codecConf[i] = new QPushButton(QT_TRANSLATE_NOOP("qaudiotracks","Configure"));
            ui.gridLayout->addWidget(codecConf[i],i,5);
            filters[i] = new QPushButton(QT_TRANSLATE_NOOP("qaudiotracks","Filters"));
            ui.gridLayout->addWidget(filters[i],i,6);
        }
        dupConfig = new QPushButton(QT_TRANSLATE_NOOP("qaudiotracks","Duplicate down"));
        ui.gridLayout->addWidget(dupConfig,0,7);
        delete [] plabel;
        showTracks(numOfTrack);
    }
    
    void showTracks(int numOfTrack)
    {
        if (numOfTrack < trackCount)    // always show source tracks
            numOfTrack = trackCount;
        numOfTrack += 1;    // show one inactive track if possible
        if (numOfTrack < 4)
            numOfTrack = 4; // show at least 4 track
        for (int i=0; i<NB_MENU; i++)
        {
            labels[i]->setVisible(i<numOfTrack);
            enabled[i]->setVisible(i<numOfTrack);
            inputs[i]->setVisible(i<numOfTrack);
            languages[i]->setVisible(i<numOfTrack);
            codec[i]->setVisible(i<numOfTrack);
            codecConf[i]->setVisible(i<numOfTrack);
            filters[i]->setVisible(i<numOfTrack);
        }            
        this->resize(100,100);  // hacky way to set size to minimum-fit
    }
    
    ~audioTrackWindow()
    {

    }
    
    Ui_DialogAudioTracks ui;

public slots:
   
};