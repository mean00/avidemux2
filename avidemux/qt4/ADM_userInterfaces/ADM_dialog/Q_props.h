#ifndef Q_props_h
#define Q_props_h

#include "ui_props.h"

class propWindow : public QDialog
{
    Q_OBJECT

public:
    propWindow(QWidget *parent);
private:
    bool gotExtraData;
    bool gotAudio;
    bool firstTime;
    Ui_propsDialog ui;

    void showEvent(QShowEvent *event);

private slots:
    void propsCopyToClipboard(void);
};
#endif	// Q_props_h
