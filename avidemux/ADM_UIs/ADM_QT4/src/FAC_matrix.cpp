/***************************************************************************
  FAC_toggle.cpp
  Handle dialog factory element : Toggle
  (C) 2006 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtGui/QSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "ADM_default.h"
#include "DIA_factory.h"

extern const char *shortkey(const char *);

namespace ADM_qt4Factory
{
class diaElemMatrix : public diaElem
{
  protected:
  public:
    uint8_t *_matrix;
    uint32_t _matrixSize;
    		diaElemMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip=NULL);
  virtual   ~diaElemMatrix() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
};


diaElemMatrix::diaElemMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip)
  : diaElem(ELEM_MATRIX)
{
	
	  param=(void *)trix;
	  paramTitle=shortkey(toggleTitle);
	  _matrix=new uint8_t[trixSize*trixSize];
	   _matrixSize=trixSize;
	   memcpy(_matrix,trix,trixSize*trixSize);
	  this->tip=tip;
	
}

diaElemMatrix::~diaElemMatrix()
{
	 if(_matrix) delete [] _matrix;
	  _matrix=NULL;
	  if(myWidget)
	  {
		  QSpinBox **arrayWidget=( QSpinBox **)myWidget;
		  delete [] arrayWidget;
		  myWidget=NULL;
	  }
	  if(paramTitle)
	      delete paramTitle;
	  paramTitle=NULL;
}
void diaElemMatrix::setMe(void *dialog, void *opaque,uint32_t line)
{
	 QSpinBox **box=new QSpinBox*[_matrixSize*_matrixSize];
	  QGridLayout *layout=(QGridLayout*) opaque;
	 myWidget=(void *)box; 


	  
	  
	 
	 QLabel *text=new QLabel( QString::fromUtf8(this->paramTitle),(QWidget *)dialog);
	 layout->addWidget(text,line,0);
	 
	 QGridLayout *layout2=new QGridLayout((QWidget *)dialog);
	  layout->addLayout(layout2,line,1);
	  /*layout->setMargin(0);
	  layout->setSpacing(0);*/
	 

	 for(int y=0;y<_matrixSize*_matrixSize;y++)
	 			  {
	 				  box[y]=new QSpinBox((QWidget *)dialog);
	 				  box[y]->setMinimum(0);
	 				  box[y]->setMaximum(255);
	 				  box[y]->setValue(_matrix[y]);
	 				 layout2->addWidget(box[y],y/_matrixSize,y%_matrixSize);
	 			  }
	 myWidget =(void *)box;
}
void diaElemMatrix::getMe(void)
{
	 QSpinBox **box=( QSpinBox **)myWidget;
	 ADM_assert(box);
	for(int y=0;y<_matrixSize*_matrixSize;y++)
		 			  {
		 				  _matrix[y]=box[y]->value();
		 			  }
	memcpy(param,_matrix,_matrixSize*_matrixSize);
}
void diaElemMatrix::enable(uint32_t onoff)
{
	QSpinBox **arrayWidget=( QSpinBox **)myWidget;
			 
			 ADM_assert(arrayWidget);
			 int index=0;
			  for(int y=0;y<_matrixSize*_matrixSize;y++)
			  {
				  if(onoff) arrayWidget[y]->setEnabled(1);
				  else arrayWidget[y]->setEnabled(0);
				  
			  }
}
}
//****************************Hoook*****************

diaElem  *qt4CreateMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip)
{
	return new  ADM_qt4Factory::diaElemMatrix(trix,toggleTitle,trixSize,tip);
}
void qt4DestroyMatrix(diaElem *e)
{
	ADM_qt4Factory::diaElemMatrix *a=(ADM_qt4Factory::diaElemMatrix *)e;
	delete a;
}
