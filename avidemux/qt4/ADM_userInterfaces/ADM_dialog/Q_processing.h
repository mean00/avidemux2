/**
 * 
 */
#pragma once
#include "ui_processing.h"
/**
 * \class processingWindow
 */
class processingWindow : public QDialog
{
	Q_OBJECT
    
public:
    
	processingWindow(QWidget *parent);
        
        bool    active;
                Ui_DialogProcessing *ui;
public slots:
        void    stop(bool a);
};

// EOF
