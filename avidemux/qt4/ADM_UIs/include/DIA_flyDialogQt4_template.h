
/***************************************************************************
                          Template for QT4 flyDialog
    copyright            : (C) 2007 by mean
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


#ifndef WINDOW_NAME
#error WINDOW_NAME undefined
#endif

#ifndef PARAM_NAME
#error PARAM_NAME undefined
#endif

#ifndef DIALOG_NAME
#error DIALOG_NAME undefined
#endif

#ifndef FUNC_NAME
#error FUNC_NAME undefined
#endif

#ifndef FLY_NAME
#error FLY_NAME undefined
#endif


static int lock=0;

class WINDOW_NAME : public QDialog
 {
     Q_OBJECT
 protected : 
    int lock;
 public:
     FLY_NAME *myCrop;
     ADM_QCanvas *canvas;
     WINDOW_NAME(QWidget *parent, PARAM_NAME *param,AVDMGenericVideoStream *in);
     ~WINDOW_NAME();
     DIALOG_NAME ui;
 public slots:
      void gather(PARAM_NAME *param);
      //void update(int i);
 private slots:
   void sliderUpdate(int foo);
   void valueChanged(int foo);

 private:
     
 };
 
  void WINDOW_NAME::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void WINDOW_NAME::gather(PARAM_NAME *param)
  {
    
        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(PARAM_NAME));
  }

void WINDOW_NAME::valueChanged( int f )
{
  if(lock) return;
  lock++;
  myCrop->update();
  lock--;
}


/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t FUNC_NAME(PARAM_NAME *param, AVDMGenericVideoStream *in)
{
        uint8_t ret=0;
        
        WINDOW_NAME dialog(qtLastRegisteredDialog(), param,in);
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


