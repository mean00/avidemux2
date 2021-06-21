/***************************************************************************                         
    copyright            : (C) 2002/2017 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QPushButton>
#include "Q_blackenBorders.h"
#include "ADM_toolkitQt.h"

//
//	Video is in RGB Colorspace
//
//
/**
 * \fn ctor
 */
Ui_blackenWindow::Ui_blackenWindow(QWidget* parent, blackenBorder *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
    ui.setupUi(this);    
    lock=0;
    // Allocate space for green-ised video
    width=in->getInfo()->width;
    height=in->getInfo()->height;

    canvas=new ADM_QCanvas(ui.graphicsView,width,height);
    
    myBlacken=new flyBlacken( this,width, height,in,canvas,ui.horizontalSlider);
    myBlacken->left=param->left&0xffffe;
    myBlacken->right=param->right&0xffffe;
    myBlacken->top=param->top&0xffffe;
    myBlacken->bottom=param->bottom&0xffffe;
    myBlacken->_cookie=&ui;
    myBlacken->addControl(ui.toolboxLayout);
    myBlacken->setTabOrder();
    myBlacken->upload();
    myBlacken->sliderChanged();

    bool rubberIsHidden = false;
    QSettings *qset = qtSettingsCreate();
    if(qset)
    {
        qset->beginGroup("blackenBorder");
        rubberIsHidden = qset->value("rubberIsHidden", false).toBool();
        qset->endGroup();
        delete qset;
        qset = NULL;
    }

    myBlacken->rubber->nestedIgnore=1;
    myBlacken->rubber_is_hidden = rubberIsHidden;
    ui.checkBoxRubber->setChecked(myBlacken->rubber_is_hidden);

    connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
    connect( ui.checkBoxRubber,SIGNAL(stateChanged(int)),this,SLOT(toggleRubber(int)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
    SPINNER(Left);
    SPINNER(Right);
    SPINNER(Top);
    SPINNER(Bottom);
#undef SPINNER
#define SPINNER(x) ui.spinBox##x->setSingleStep(2); ui.spinBox##x->setKeyboardTracking(false);
    SPINNER(Left)
    SPINNER(Right)
    SPINNER(Top)
    SPINNER(Bottom)
#undef SPINNER

    QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
    connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

    setModal(true);
}
/**
 * \fn dtor
 */
Ui_blackenWindow::~Ui_blackenWindow()
{
    if(myBlacken)
    {
        QSettings *qset = qtSettingsCreate();
        if(qset)
        {
            qset->beginGroup("blackenBorder");
            qset->setValue("rubberIsHidden", myBlacken->rubber_is_hidden);
            qset->endGroup();
            delete qset;
            qset = NULL;
        }
        delete myBlacken;
        myBlacken=NULL;
    }
    if(canvas)
        delete canvas;
    canvas=NULL;
}
/**
 * \fn sliderUpdate
 */
void Ui_blackenWindow::sliderUpdate(int foo)
{
    myBlacken->sliderChanged();
}
/**
 * \fn gather
 */
void Ui_blackenWindow::gather(blackenBorder *param)
{
    myBlacken->download();
    param->left=myBlacken->left;
    param->right=myBlacken->right;
    param->top=myBlacken->top;
    param->bottom=myBlacken->bottom;
}
/**
 * \fn toggleRubber
 */
void Ui_blackenWindow::toggleRubber(int checkState)
{
    bool visible=true;
    if(checkState)
        visible=false;
    myBlacken->rubber->rubberband->setVisible(visible);
    myBlacken->rubber_is_hidden=!visible;
}
/**
 * \fn valueChanged
 */
void Ui_blackenWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myBlacken->rubber->nestedIgnore++;
    myBlacken->download();
    myBlacken->sameImage();
    myBlacken->rubber->nestedIgnore--;
    lock--;
}
/**
 * \fn reset
 */
void Ui_blackenWindow::reset( bool f )
{
    myBlacken->left=0;
    myBlacken->right=0;
    myBlacken->bottom=0;
    myBlacken->top=0;
    lock++;
    myBlacken->upload();
    myBlacken->sameImage();
    lock--;
}
/**
 * \fn resizeEvent
 */
void Ui_blackenWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myBlacken->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myBlacken->adjustCanvasPosition();
    
     int x=(int)((double)myBlacken->left*myBlacken->_zoom);
    int y=(int)((double)myBlacken->top*myBlacken->_zoom);
    int w=(int)((double)(myBlacken->_w-(myBlacken->left+myBlacken->right))*myBlacken->_zoom);
    int h=(int)((double)(myBlacken->_h-(myBlacken->top+myBlacken->bottom))*myBlacken->_zoom);

    myBlacken->blockChanges(true);
    myBlacken->rubber->nestedIgnore++;
    myBlacken->rubber->move(x,y);
    myBlacken->rubber->resize(w,h);
    myBlacken->rubber->nestedIgnore--;
    myBlacken->blockChanges(false);
}
/**
 * \fn showEvent
 */
void Ui_blackenWindow::showEvent(QShowEvent *event)
{
    myBlacken->rubber->rubberband->show(); // must be called first
    QDialog::showEvent(event);
    myBlacken->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
    myBlacken->rubber->nestedIgnore=0;
    if(myBlacken->rubber_is_hidden)
        myBlacken->rubber->rubberband->hide();
}

//************************
uint8_t flyBlacken::upload(bool redraw, bool toRubber)
{
Ui_blackenDialog *w=(Ui_blackenDialog *)_cookie;

    if(!redraw)
    {
        blockChanges(true);
    }


    w->spinBoxLeft->setValue(left);
    w->spinBoxRight->setValue(right);
    w->spinBoxTop->setValue(top);
    w->spinBoxBottom->setValue(bottom);
    if(toRubber)
    {
        rubber->nestedIgnore++;
        rubber->move(_zoom*(float)left,_zoom*(float)top);
        rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
        rubber->nestedIgnore--;
    }
    if(!redraw)
    {
        blockChanges(false);
    }
    
    return 1;
}
uint8_t flyBlacken::download(void)
{
    int reject=0;
    Ui_blackenDialog *w=(Ui_blackenDialog *)_cookie;
#define SPIN_GET(x,y) x=w->spinBox##y->value(); if(x&1) { x&=0xffffe; blockChanges(true); w->spinBox##y->setValue(x); blockChanges(false); }
        SPIN_GET(left,Left);
        SPIN_GET(right,Right);
        SPIN_GET(top,Top);
        SPIN_GET(bottom,Bottom);

        //printf("%d %d %d %d\n",left,right,top,bottom);

        if((top+bottom)>_h)
                {
                        top=bottom=0;
                        reject=1;
                }
        if((left+right)>_w)
                {
                        left=right=0;
                        reject=1;
                }
        if(reject)
                upload();
        else
           {
               blockChanges(true);
               rubber->nestedIgnore++;
               rubber->move(_zoom*(float)left,_zoom*(float)top);
               rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
               rubber->nestedIgnore--;
               blockChanges(false);
           }        
               return true;
}
void flyBlacken::setTabOrder(void)
{
    Ui_blackenDialog *w=(Ui_blackenDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(w->spinBox##x);
#define PUSH_TOG(x) controls.push_back(w->checkBox##x);
    PUSH_SPIN(Left)
    PUSH_SPIN(Right)
    PUSH_SPIN(Top)
    PUSH_SPIN(Bottom)
    PUSH_TOG(Rubber)

    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);

    QWidget *first, *second;

    for(std::vector<QWidget *>::iterator tor = controls.begin(); tor != controls.end(); ++tor)
    {
        if(tor+1 == controls.end()) break;
        first = *tor;
        second = *(tor+1);
        _parent->setTabOrder(first,second);
        //ADM_info("Tab order: %p (%s) --> %p (%s)\n",first,first->objectName().toUtf8().constData(),second,second->objectName().toUtf8().constData());
    }
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
bool DIA_getBlackenParams(	blackenBorder *param,ADM_coreVideoFilter *in)
{
    bool ret=0;

    Ui_blackenWindow dialog(qtLastRegisteredDialog(), param,in);
    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }
    qtUnregisterDialog(&dialog);
    return ret;
}
//____________________________________
// EOF
