#include "ui_audioTracks.h"

class audioTrackWindow : public QDialog
{
	Q_OBJECT

protected:


public:
	audioTrackWindow()
    {
        ui.setupUi(this);
    }
	~audioTrackWindow()
    {

    }
	Ui_DialogAudioTracks ui;

public slots:
   
};