#ifndef Q_props_h
#define Q_props_h

#include <vector>
#include "ui_props.h"

class propWindow : public QDialog
{
    Q_OBJECT

public:
    propWindow(QWidget *parent);
    Ui_propsDialog ui;
private:
    std::vector <QString> listOfValues;
public slots:
    void propsCopyToClipboard(void);
};
#endif	// Q_props_h
